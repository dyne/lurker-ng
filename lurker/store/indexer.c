/*  $Id: indexer.c,v 1.1 2002-02-03 03:10:53 terpstra Exp $
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

#define _GNU_SOURCE
// #define DEBUG 1

#include "common.h"
#include "io.h"

#include "wbuffer.h"
#include "indexer.h"

#if 0
/*
 * Determine where to split words, and index the
 * results in the database with lu_push_keyword.
 */
static void index_keywords(char *buffer, int length)
{
	const char *word, *adv;

	while (length > 0 && isspace(*buffer))
	{
		++buffer;
		--length;
	}

	printf("%.*s", length, buffer);

	/*
	 * Words are split based on punctuation (ispunct()) and
	 * spacing (isspace()).  A word with punctuation in it
	 * will be split into segments--before and after the
	 * punctuation occurs--as well as indexed as a whole.
	 * E.g., `foo_bar' causes `foo,' `bar,' and `foo_bar' to
	 * be indexed.
	 *
	 * !!! Finish this RIGHT NOW.
	 */
	for (word = adv = buffer; length > 0; ++adv, --length);
}

/*
 * Traverse a message, looking for plaintext parts.  When
 * such a part is located, we'll index all of the words
 * it contains.
 */
void index_traverse(struct msg* in, struct mail_bodystruct* body)
{
	struct mail_body_part *p;		/* Filthy struct. */
	size_t length;
	char *buffer;
	int nfree;

	switch ((int)body->type)
	{
		case TYPEMESSAGE:
			/*
			 * This part contains an encapsulated message.
			 */
			if (strcasecmp(body->subtype, "rfc822"))
				break;

			index_traverse(in, body->nested.msg->body);
			break;

		case TYPETEXT:
			/*
			 * This is what we want to index -- stop.
			 */
			if (strcasecmp(body->subtype, "plain"))
				break;

			buffer = mail_select(in, body, &length, &nfree);
			index_keywords(buffer, length);
			if (nfree)
				fs_give((void **)&buffer);

			break;

		case TYPEMULTIPART:
			/*
			 * Multipart message.  Look at the nested parts
			 * and hopefully find some plaintext.
			 */
			for (p = body->nested.part; p != NULL; p = p->next)
				index_traverse(in, &p->body);
			break;
		default:
			break;
	}
}

#endif

/*------------------------------------------------- Public component methods */

int lu_indexer_init()
{
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
	return 0;
}
