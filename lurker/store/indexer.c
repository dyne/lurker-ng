/*  $Id: indexer.c,v 1.17 2002-05-11 19:24:59 terpstra Exp $
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
#include "prefix.h"

#include "mbox.h"
#include "wbuffer.h"
#include "indexer.h"
#include "btree.h"

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

/* These are characters which should be interpretted as both part of the word
 * and as a word seperator. eg: 'maul.sith.vpn' should be indexed as 'maul',
 * 'sith', 'vpn', and 'maul.sith.vpn' because '.' is listed here.
 */
static const char my_indexer_word_splits[] =
"$@./:\\-_";
static char my_indexer_is_split[256];

/* These are characters which should be interpretted as word breaks.
 * No known language should use these as letters in a word.
 * All chars 000-037 fall in this category too.
 */
static const char my_indexer_word_divs[] = WORD_BREAKS;
static char my_indexer_is_div[256];

/* These tables are the conversion for characters being written to keywords.
 */
static const char my_indexer_orig[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char my_indexer_dest[] = "abcdefghijklmnopqrstuvwxyz";
static char my_indexer_conv[256];

/*------------------------------------------------ Private helper methods */

LU_BTREE_DEFINE(
	indexer, 
	my_indexer_ptr, 
	MY_INDEXER_MAX,
	My_Indexer_Tree,
	my_indexer_buf,
	strcmp)

/* Test the strings for equality - ignoring case */
static int my_indexer_strcasecmp(
	const char* a, 
	const char* b)
{
	while (*a && *b)
	{
		int dif = my_indexer_conv[((int)*a++)] - 
		          my_indexer_conv[((int)*b++)];
		if (dif != 0) return dif;
	}
	
	if (*b) return 1;
	if (*a) return -1;
	
	return 0;
}

/* This function finds space in our tree to put the keyword in.
 * It does not enter it in the tree if we have already seen this key.
 */ 
static int my_indexer_push_keyword(
	const char* keyword)
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

/* Combine the prefix with the substring */
static int my_indexer_index_hunk(
	const char* buf,
	const char* eos,
	const char* prefix)
{
	char out[LU_KEYWORD_LEN+1];
	char* w;
	char* e;
	
	/* A quick check to avoid function calls */
	if (prefix[0])
	{
		strcpy(&out[0], prefix);
		w = &out[strlen(prefix)];
	}
	else
	{
		w = &out[0];
	}
	
	e = &out[sizeof(out) - 1];
	
	/* Copy the range into the buffer while converting it */
	while (w != e && buf != eos)
	{
		*w++ = my_indexer_conv[((int)*buf++)];
	}
	
	*w = 0;
	return my_indexer_push_keyword(&out[0]);
}

/* Look at a section of non-whitespace chars and decide what to do with it. */
static int my_indexer_digest_hunk(
	const char* buf, 
	const char* eos, 
	const char* prefix)
{
	const char* start;
	const char* scan;
	
	/*!!! Make me work with non-romanian languages (eg. japanese) */
	/* Japanese has no spaces to delineate words */
	
	/* Firstly, index the entire chunk, but without leading or trailing
	 * chars from is_split.
	 */
	
	while (buf != eos && my_indexer_is_split[((int)*buf)])    buf++;
	
	/* Don't index the ether */
	if (buf == eos) return 0;
	
	/* We know there's at least one non-split char so we can keep
	 * knocking off the eos without testing for emptiness.
	 */
	while (my_indexer_is_split[((int)*(eos-1))]) eos--;
	
	/* Index the entire hunk. */
	if (my_indexer_index_hunk(buf, eos, prefix) != 0)
		return -1;
	
	/* Now, divide the chunk into bits which we will keyword index */
	start = buf;
	for (scan = buf; scan != eos; scan++)
	{
		if (my_indexer_is_split[((int)*scan)])
		{
			if (start != scan)
			{
				if (my_indexer_index_hunk(start, scan, prefix) != 0)
					return -1;
			}
			
			start = scan+1;
		}
	}
	
	if (start != eos)
	{
		if (my_indexer_index_hunk(start, eos, prefix) != 0)
			return -1;
	}
	
	return 0;
}

/* Run through a buffer looking for segments of non-divide characters.
 */
static int my_indexer_keyword_scan(
	const char* buf, 
	int len, 
	const char* prefix)
{
	const char* start;
	const char* scan;
	const char* eos = buf + len;
	
	start = 0;
	for (scan = buf; scan != eos; scan++)
	{
		if (my_indexer_is_div[((int)*scan)])
		{
			if (start)
			{
				my_indexer_digest_hunk(start, scan, prefix);
				start = 0;
			}
		}
		else
		{
			if (!start)
			{
				start = scan;
			}
		}
	}
	
	if (start)
	{
		my_indexer_digest_hunk(start, eos, prefix);
	}
	
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
			/*
			 * This part contains an encapsulated message.
			 */
			if (my_indexer_strcasecmp(body->subtype, "rfc822"))
				break;

			my_indexer_traverse(in, body->nested.msg->body);
			break;

		case TYPETEXT:
			/*
			 * This is what we want to index -- stop.
			 */
			if (my_indexer_strcasecmp(body->subtype, "plain"))
				break;

			buffer = lu_mbox_select_body(in, body, &length, &nfree);
			my_indexer_keyword_scan(buffer, length,
				LU_KEYWORD_WORD);
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
	
	if (address->personal)
	{
		my_indexer_keyword_scan(
			address->personal, 
			strlen(address->personal),
			LU_KEYWORD_AUTHOR);
	}
	
	if (address->mailbox && address->host)
	{
		snprintf(&buf[0], sizeof(buf),
			"%s@%s",
			address->mailbox,
			address->host);
	
		my_indexer_keyword_scan(
			&buf[0],
			strlen(&buf[0]),
			LU_KEYWORD_AUTHOR);
	}
	else if (address->mailbox)
	{
		my_indexer_keyword_scan(
			address->mailbox,
			strlen(address->mailbox),
			LU_KEYWORD_AUTHOR);
	}
	else if (address->host)
	{
		my_indexer_keyword_scan(
			address->host,
			strlen(address->host),
			LU_KEYWORD_AUTHOR);
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
	int i;
	
	/* Clear the lookup tables */
	memset(&my_indexer_is_split[0], 0, sizeof(my_indexer_is_split));
	memset(&my_indexer_is_div  [0], 0, sizeof(my_indexer_is_div));
	
	/* Bootstrap the lookup tables */
	for (i = 0; i < sizeof(my_indexer_word_splits)-1; i++)
		my_indexer_is_split[((int)my_indexer_word_splits[i])] = 1;
	for (i = 0; i < sizeof(my_indexer_word_divs)-1; i++)
		my_indexer_is_div[((int)my_indexer_word_divs[i])] = 1;
	
	/* All control characters divide words */
	for (i = 0; i < 040; i++)
		my_indexer_is_div[i] = 1;
	
	/* Initialize conversion table */
	for (i = 0; i < 256; i++)
		my_indexer_conv[i] = i;
	
	/* Fill the conversion entries */
	for (i = 0; i < sizeof(my_indexer_orig)-1; i++)
		my_indexer_conv[((int)my_indexer_orig[i])] = 
			my_indexer_dest[i];
	
	my_indexer_buf = malloc(LU_INDEXER_MAX_KEYS * sizeof(My_Indexer_Tree) +
				LU_INDEXER_MAX_DYNAMIC);
	if (!my_indexer_buf)
	{
		fprintf(stderr, "Failed to allocate storage for keyword import tree\n");
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
	my_indexer_avl_root = 0;
}

void lu_indexer_location(
	lu_word		list,
	lu_word		mbox,
	message_id	thread,
	int		is_head)
{
	char buf[LU_KEYWORD_LEN+1];
	
	/* Push the mailing list keyword. */
	snprintf(&buf[0], sizeof(buf), "%s%d", 
		LU_KEYWORD_LIST, list);
	my_indexer_push_keyword(&buf[0]);
	
	/* Push the mail box keyword. */
	snprintf(&buf[0], sizeof(buf), "%s%d:%d", 
		LU_KEYWORD_MBOX, list, mbox);
	my_indexer_push_keyword(&buf[0]);
	
	/* Push the thread keyword. */
	snprintf(&buf[0], sizeof(buf), "%s%d",
		LU_KEYWORD_THREAD, thread);
	my_indexer_push_keyword(&buf[0]);
	
	if (is_head)
	{
		/* Push the head keyword */
		snprintf(&buf[0], sizeof(buf), "%s%d",
			LU_KEYWORD_LIST_THREADS, list);
		my_indexer_push_keyword(&buf[0]);
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
	my_indexer_push_keyword(&buf[0]);
	
	snprintf(&buf[0], sizeof(buf), "%s%s", 
		LU_KEYWORD_MONTH, my_indexer_mons[when->tm_mon]);
	my_indexer_push_keyword(&buf[0]);
	
	snprintf(&buf[0], sizeof(buf), "%s%d",
		LU_KEYWORD_DAY_OF_MONTH, when->tm_mday);
	my_indexer_push_keyword(&buf[0]);
		
	snprintf(&buf[0], sizeof(buf), "%s%d", 
		LU_KEYWORD_HOUR, when->tm_hour);
	my_indexer_push_keyword(&buf[0]);
		
	snprintf(&buf[0], sizeof(buf), "%s%s", 
		LU_KEYWORD_WEEKDAY, my_indexer_dows[when->tm_wday]);
	my_indexer_push_keyword(&buf[0]);
	
	/* Now, push keywords for the reply-to */
	if (*reply_id)
	{
		snprintf(&buf[0], sizeof(buf), "%s%s",
			LU_KEYWORD_REPLY_TO, 
			reply_id);
		my_indexer_push_keyword(&buf[0]);
	}
	
	/* Now, push keywords for all source addresses */
	if (body->env->from)
	{
		my_indexer_push_address(body->env->from);
	}
	
	if (body->env->sender)
	{
		my_indexer_push_address(body->env->sender);
	}
	
	if (body->env->reply_to)
	{
		my_indexer_push_address(body->env->sender);
	}
	
	/* If we have a subject index it as both body and subject */
	if (body->env->subject)
	{
		my_indexer_keyword_scan(
			body->env->subject, 
			strlen(body->env->subject),
			LU_KEYWORD_SUBJECT);
			
		my_indexer_keyword_scan(
			body->env->subject, 
			strlen(body->env->subject),
			LU_KEYWORD_WORD);
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
