/*  $Id: indexer.c,v 1.23 2002-06-17 12:32:37 terpstra Exp $
 *  
 *  indexer.c - Handles indexing a message for keyword searching
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: GPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; version 2.1.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "keyword.h"

#include "mbox.h"
#include "wbuffer.h"
#include "indexer.h"
#include "btree.h"
#include "breader.h"

#include <stdlib.h>
#include <stdio.h>

/*------------------------------------------------ Constant parameters */

/* A message is not allowed to have more than this many keywords. We simply
 * start ignoring keywords beyond this limit.
 */
#define LU_INDEXER_MAX_KEYS	2048

/* A limit on the amount of dynamic storage we allow for decomposing and
 * creating extra keywords. Here we assume that the average word is 8 chars
 * long.
 */
#define LU_INDEXER_MAX_DYNAMIC	16384

/* Max size of my_indexer_ptr */
#define MY_INDEXER_MAX		0xFFFFUL

/*------------------------------------------------ Private data types */

typedef lu_word my_indexer_ptr;

typedef struct My_Indexer_Tree_T
{
	const char*	key;
	
	lu_byte		skew;
	my_indexer_ptr	left;
	my_indexer_ptr	right;
} My_Indexer_Tree;

/*------------------------------------------------ Private global vars */

static My_Indexer_Tree*	my_indexer_buf = 0;

/* These tell us how much of the dynamic vars above have been used by the
 * current indexer pass.
 */
static my_indexer_ptr	my_indexer_avl_root= 0;
static char*		my_indexer_dyn_off = 0;
static my_indexer_ptr	my_indexer_avl_off = 0;

static const char* my_indexer_mons[12] = { 
	"jan", "feb", "mar", "apr", "may", "jun", 
	"jul", "aug", "sep", "oct", "nov", "dec" };

static const char* my_indexer_dows[7] = {
	"sun", "mon", "tue", "wed", "thu", "fri", "sat" };

/*------------------------------------------------ Private helper methods */

LU_BTREE_DEFINE(
	indexer, 
	my_indexer_ptr, 
	MY_INDEXER_MAX,
	My_Indexer_Tree,
	my_indexer_buf,
	strcmp)

/* This function finds space in our tree to put the keyword in.
 * It does not enter it in the tree if we have already seen this key.
 */ 
static int my_indexer_push_keyword(
	const char*	keyword,
	void*		arg)
{
	my_indexer_ptr tmp;
	
	int len = strlen(keyword) + 1;
	
	/* Do we have a record we could fit this keyword and the avl tree
	 * record in our buffer?
	 */
	if (((char*)&my_indexer_buf[my_indexer_avl_off+1]) + len > my_indexer_dyn_off)
	{
		return -1;
	}
	
	my_indexer_buf[my_indexer_avl_off].key = keyword;
	
	/* Use the avl insert method to insert the key balanced. */
	tmp = my_btree_indexer_insert(my_indexer_avl_root, my_indexer_avl_off);
	
	if (tmp == MY_INDEXER_MAX)
	{	/* It was already indexed */
		return 0;
	}
	
	/* This is the new root */
	my_indexer_avl_root = tmp;
	
	/* The record was not there and has been added. Finalize it. */
	my_indexer_dyn_off -= len;
	my_indexer_buf[my_indexer_avl_off++].key = my_indexer_dyn_off;
	memcpy(my_indexer_dyn_off, keyword, len);
	
	return 0;
}

/*
 * Traverse a message, looking for plaintext parts.  When
 * such a part is located, we'll index all of the words
 * it contains.
 */
static void my_indexer_traverse(
	struct Lu_Mbox_Message* in, 
	struct mail_bodystruct* body)
{
	struct mail_body_part *p;
	size_t length;
	char *buffer;
	int nfree;
	
	if (!body)
	{	/* You never know... */
		return;
	}

	switch ((int)body->type)
	{
		case TYPEMESSAGE:
			/* This part contains an encapsulated message.
			 */
			 
			my_indexer_traverse(in, body->nested.msg->body);
			break;

		case TYPETEXT:
			/* This is what we want to index -- stop.
			 */

			buffer = lu_mbox_select_body(in, body, &length, &nfree);
			my_keyword_digest_string(
				buffer, length,
				LU_KEYWORD_WORD,
				&my_indexer_push_keyword, 0, 1);
			if (nfree)
				fs_give((void **)&buffer);

			break;

		case TYPEMULTIPART:
			/*
			 * Multipart message.  Look at the nested parts
			 * and hopefully find some plaintext.
			 */
			for (p = body->nested.part; p != NULL; p = p->next)
				my_indexer_traverse(in, &p->body);
			break;
		default:
			break;
	}
}

static int my_indexer_push_address(
	ADDRESS* address)
{
	static char buf[200];
	
	if (!address) return 0;
	
	if (address->personal)
	{
		my_keyword_digest_string(
			address->personal, strlen(address->personal),
			LU_KEYWORD_AUTHOR,
			&my_indexer_push_keyword, 0, 1);
	}
	
	if (address->mailbox && address->host)
	{
		snprintf(&buf[0], sizeof(buf),
			"%s@%s",
			address->mailbox,
			address->host);
	
		my_keyword_digest_string(
			&buf[0], strlen(&buf[0]),
			LU_KEYWORD_AUTHOR,
			&my_indexer_push_keyword, 0, 1);
	}
	else if (address->mailbox)
	{
		my_keyword_digest_string(
			address->mailbox, strlen(address->mailbox),
			LU_KEYWORD_AUTHOR,
			&my_indexer_push_keyword, 0, 1);
	}
	else if (address->host)
	{
		my_keyword_digest_string(
			address->host, strlen(address->host),
			LU_KEYWORD_AUTHOR,
			&my_indexer_push_keyword, 0, 1);
	}
	
	return 0;
}

static int my_indexer_dump_words(
	my_indexer_ptr where,
	message_id id)
{
	if (where == MY_INDEXER_MAX)
	{
		return 0;
	}
	
	if (my_indexer_dump_words(my_indexer_buf[where].left, id) != 0)
	{
		return -1;
	}

#ifdef DEBUG
	printf("%s ", my_indexer_buf[where].key);
#else
	lu_wbuffer_append(my_indexer_buf[where].key, id);
#endif
	
	if (my_indexer_dump_words(my_indexer_buf[where].right, id) != 0)
	{
		return -1;
	}
	
	return 0;
}

/*------------------------------------------------- Public component methods */

int lu_indexer_init()
{
	my_indexer_buf = malloc(LU_INDEXER_MAX_KEYS * sizeof(My_Indexer_Tree) +
				LU_INDEXER_MAX_DYNAMIC);
	if (!my_indexer_buf)
	{
		fputs(_("Failed to allocate storage for keyword import tree\n"),
			stderr);
		return -1;
	}
	
	return 0;
}

int lu_indexer_open()
{
	return 0;
}

int lu_indexer_sync()
{
	return 0;
}

int lu_indexer_close()
{
	return 0;
}

int lu_indexer_quit()
{
	if (my_indexer_buf)
	{
		free(my_indexer_buf);
		my_indexer_buf = 0;
	}
	
	return 0;
}

/*------------------------------------------------- Indexing algorithm */

void lu_indexer_prep()
{
	/* We have imported no keywords in this pass yet. */
	my_indexer_dyn_off = ((char*)my_indexer_buf) + 
		LU_INDEXER_MAX_KEYS * sizeof(My_Indexer_Tree) +
		LU_INDEXER_MAX_DYNAMIC;
	
	my_indexer_avl_off  = 0;
	my_indexer_avl_root = MY_INDEXER_MAX;
}

void lu_indexer_location(
	lu_word		list,
	lu_word		mbox,
	message_id*	offset)
{
	char			buf[LU_KEYWORD_LEN+1];
	
	/* Push the mailing list keyword. */
	snprintf(&buf[0], sizeof(buf), "%s%d", 
		LU_KEYWORD_LIST, list);
	*offset = lu_breader_quick_records(&buf[0]);
	my_indexer_push_keyword(&buf[0], 0);
	
	/* Push the mail box keyword. */
	snprintf(&buf[0], sizeof(buf), "%s%d:%d", 
		LU_KEYWORD_MBOX, list, mbox);
	my_indexer_push_keyword(&buf[0], 0);
}

void lu_indexer_threading(
	lu_word		list,
	message_id	thread,
	int		is_head)
{
	char buf[LU_KEYWORD_LEN+1];
	
	/* Push the thread keyword. */
	snprintf(&buf[0], sizeof(buf), "%s%d",
		LU_KEYWORD_THREAD, thread);
	my_indexer_push_keyword(&buf[0], 0);
	
	if (is_head)
	{
		/* Push the head keyword */
		snprintf(&buf[0], sizeof(buf), "%s%d",
			LU_KEYWORD_LIST_THREADS, list);
		my_indexer_push_keyword(&buf[0], 0);
	}
}
	
void lu_indexer_message(
	struct Lu_Mbox_Message*	body, 
	time_t		stamp,
	const char*	reply_id)
{
	char buf[LU_KEYWORD_LEN+1];
	struct tm* when;
	
	/* Start working on the time keywords */
	when = localtime(&stamp);
	
	snprintf(&buf[0], sizeof(buf), "%s%d",
		LU_KEYWORD_YEAR, when->tm_year + 1900);
	my_indexer_push_keyword(&buf[0], 0);
	
	snprintf(&buf[0], sizeof(buf), "%s%s", 
		LU_KEYWORD_MONTH, my_indexer_mons[when->tm_mon]);
	my_indexer_push_keyword(&buf[0], 0);
	
	snprintf(&buf[0], sizeof(buf), "%s%d",
		LU_KEYWORD_DAY_OF_MONTH, when->tm_mday);
	my_indexer_push_keyword(&buf[0], 0);
		
	snprintf(&buf[0], sizeof(buf), "%s%d", 
		LU_KEYWORD_HOUR, when->tm_hour);
	my_indexer_push_keyword(&buf[0], 0);
		
	snprintf(&buf[0], sizeof(buf), "%s%s", 
		LU_KEYWORD_WEEKDAY, my_indexer_dows[when->tm_wday]);
	my_indexer_push_keyword(&buf[0], 0);
	
	/* Now, push keywords for the reply-to */
	if (*reply_id)
	{
		snprintf(&buf[0], sizeof(buf), "%s%s",
			LU_KEYWORD_REPLY_TO, 
			reply_id);
		my_indexer_push_keyword(&buf[0], 0);
	}
	
	/* Now, push keywords for all source addresses */
	my_indexer_push_address(body->env->from);
	my_indexer_push_address(body->env->sender);
	my_indexer_push_address(body->env->reply_to);
	
	/* If we have a subject index it as both body and subject */
	if (body->env->subject)
	{
		my_keyword_digest_string(
			body->env->subject, strlen(body->env->subject),
			LU_KEYWORD_SUBJECT,
			&my_indexer_push_keyword, 0, 1);
			
		my_keyword_digest_string(
			body->env->subject, strlen(body->env->subject),
			LU_KEYWORD_WORD,
			&my_indexer_push_keyword, 0, 1);
	}
	
	/* Now, scan all the keywords in the body */
	my_indexer_traverse(body, body->body);
}

int lu_indexer_dump(
	message_id id)	
{
	/* Ok, we have all the keyword - dump them. */
#ifdef DEBUG
	printf("%d: [ ", id);
	my_indexer_dump_words(my_indexer_avl_root, id);
	printf("]\n");
	return 0;
#else
	return my_indexer_dump_words(my_indexer_avl_root, id);
#endif
}
