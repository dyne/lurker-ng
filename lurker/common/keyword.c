/*  $Id: keyword.c,v 1.5 2002-07-11 20:39:23 terpstra Exp $
 *  
 *  prefix.c - Digest a hunk of string into keywords.
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
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "keyword.h"

/*------------------------------------------------ Private global vars */

/* These are characters which should be interpretted as both part of the word
 * and as a word seperator. eg: 'maul.sith.vpn' should be indexed as 'maul',
 * 'sith', 'vpn', and 'maul.sith.vpn' because '.' is listed here.
 */
static const char my_keyword_word_splits[] = "$@./:\\-_~&=%?#+";
static char my_keyword_is_split[256];

/* These are characters which should be interpretted as word breaks.
 * No known language should use these as letters in a word.
 * All chars 000-037 fall in this category too.
 */
static const char my_keyword_word_divs[] = " !\"'()*,;<>[]^`{|}";
static char my_keyword_is_div[256];

/* These tables are the conversion for characters being written to keywords.
 */
static const char my_keyword_orig[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char my_keyword_dest[] = "abcdefghijklmnopqrstuvwxyz";
static char my_keyword_conv[256];

/* We need to be initd */
static int my_keyword_initd = 1;

/*------------------------------------------------ Private helper methods */

/* Combine the prefix with the substring */
static int my_keyword_index_hunk(
	const char* buf,
	const char* eos,
	const char* prefix,
	int (*writefn)(const char* keyword, void* arg),
	void* arg)
{
	char out[LU_KEYWORD_LEN+1];
	char* w;
	char* e;
	
	if (buf == eos)
	{	/* Don't index nothing */
		return 0;
	}
	
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
		*w++ = my_keyword_conv[((int)*buf++)];
	}
	
	*w = 0;
	if (!out[0])
	{
		/* Ignore this keyword */
		return 0;
	}
	
	return writefn(&out[0], arg);
}

/* Look at a section of non-whitespace chars and decide what to do with it. */
static int my_keyword_digest_hunk(
	const char* buf, 
	const char* eos, 
	const char* prefix,
	int       (*writefn)(const char* keyword, void* arg),
	void*       arg,
	int         do_div)
{
	const char* start;
	const char* scan;
	
	/*!!! Make me work with non-romanian languages (eg. japanese) */
	/* Japanese has no spaces to delineate words */
	
	/* Firstly, index the entire chunk, but without leading or trailing
	 * chars from is_split.
	 */
	
	while (buf != eos && my_keyword_is_split[((int)*buf)])    buf++;
	
	/* Don't index the ether */
	if (buf == eos) return 0;
	
	/* We know there's at least one non-split char so we can keep
	 * knocking off the eos without testing for emptiness.
	 */
	while (my_keyword_is_split[((int)*(eos-1))]) eos--;
	
	/* Index the entire hunk. */
	if (my_keyword_index_hunk(buf, eos, prefix, writefn, arg) != 0)
		return -1;
	
	if (!do_div) return 0;
	
	/* Now, divide the chunk into bits which we will keyword index */
	start = buf;
	for (scan = buf; scan != eos; scan++)
	{
		if (my_keyword_is_split[((int)*scan)])
		{
			if (start != scan)
			{
				if (my_keyword_index_hunk(start, scan, 
					prefix, writefn, arg) != 0)
					return -1;
			}
			
			start = scan+1;
		}
	}
	
	if (start != eos)
	{
		if (my_keyword_index_hunk(start, eos, prefix, writefn, arg) != 0)
			return -1;
	}
	
	return 0;
}

static void my_keyword_init(void)
{
	int i;
	
	/* Clear the lookup tables */
	memset(&my_keyword_is_split[0], 0, sizeof(my_keyword_is_split));
	memset(&my_keyword_is_div  [0], 0, sizeof(my_keyword_is_div));
	
	/* Bootstrap the lookup tables */
	for (i = 0; i < sizeof(my_keyword_word_splits)-1; i++)
		my_keyword_is_split[((int)my_keyword_word_splits[i])] = 1;
	for (i = 0; i < sizeof(my_keyword_word_divs)-1; i++)
		my_keyword_is_div[((int)my_keyword_word_divs[i])] = 1;
	
	/* All control characters divide words */
	for (i = 0; i < 040; i++)
		my_keyword_is_div[i] = 1;
	
	/* Initialize conversion table */
	for (i = 0; i < 256; i++)
		my_keyword_conv[i] = i;
	
	/* Fill the conversion entries */
	for (i = 0; i < sizeof(my_keyword_orig)-1; i++)
		my_keyword_conv[((int)my_keyword_orig[i])] = 
			my_keyword_dest[i];
	
	my_keyword_initd = 0;
}

/*------------------------------------------------- Public component methods */

/* Run through a buffer looking for segments of non-divide characters.
 */
int my_keyword_digest_string(
	const char* buf, 
	int         len, 
	const char* prefix,
	int       (*writefn)(const char* keyword, void* arg),
	void*       arg,
	int         do_div)
{
	const char* start;
	const char* scan;
	const char* eos = buf + len;
	
	if (my_keyword_initd)
		my_keyword_init();
	
	start = 0;
	for (scan = buf; scan != eos; scan++)
	{
		if (my_keyword_is_div[((int)*scan)])
		{
			if (start)
			{
				my_keyword_digest_hunk(start, scan, 
					prefix, writefn, arg, do_div);
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
		my_keyword_digest_hunk(start, eos, prefix, writefn, arg, do_div);
	}
	
	return 0;
}

