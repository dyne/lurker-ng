/*  $Id: search.c,v 1.4 2002-05-29 08:09:54 terpstra Exp $
 *  
 *  search.h - Uses the breader to execute a given search
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
#include "prefix.h"

#include "breader.h"
#include "search.h"

#include <ctype.h>

/*------------------------------------------------ Constant parameters */

/* This is the maximum terms allowed in a search.
 */
#define LU_MAX_TERMS	16

/*------------------------------------------------ Private global vars */

static Lu_Breader_Handle	my_search_handle[LU_MAX_TERMS];
static message_id		my_search_id[LU_MAX_TERMS];
static int			my_search_handles = 0;

/*------------------------------------------------- Public component methods */

int lu_search_init()
{
	return 0;
}

int lu_search_open()
{
	return 0;
}

int lu_search_sync()
{
	return 0;
}

int lu_search_close()
{
	return 0;
}

int lu_search_quit()
{
	return 0;
}

/*------------------------------------------------- Public component methods */

int lu_search_start(
	const char* keywords,
	const char** error)
{
	char buf[LU_KEYWORD_LEN+1];
	char* w;
	char* e;
	
	e = &buf[sizeof(buf)-1];
	
	if (my_search_handles != 0)
	{
		*error = "Already executing a search";
		return -1;
	}
	
	while (*keywords)
	{
		/* Skip whitespace */
		while (*keywords && isspace(*keywords)) keywords++;
		
		for (	w = &buf[0]; 
			*keywords && !isspace(*keywords) && w != e; 
			keywords++, w++)
		{
			/* Special case work around file system */
			*w = *keywords;
		}
		
		if (*keywords && !isspace(*keywords))
		{
			*error = "Keyword truncated";
		
			/* Skip over the overflow */
			while (*keywords && !isspace(*keywords)) keywords++;
		}
		
		*w = 0;
		if (buf[0] != 0)
		{
			if (my_search_handles == LU_MAX_TERMS)
			{
				*error = "Too many search terms";
				continue;
			}
			
			my_search_handle[my_search_handles] = 
				lu_breader_new(&buf[0]);
			
			if (my_search_handle[my_search_handles] == 0)
			{
				lu_search_end();
				return -1;
			}
			
			my_search_id[my_search_handles] = 
				lu_breader_last(my_search_handle[my_search_handles]);
			
			my_search_handles++;
		}
	}
	
	return 0;
}

int lu_search_result(
	message_id* result)
{
	int		i;
	int		which;
	int		done;
	
	message_id	largest;
	message_id	smallest;
	message_id	index;
	
	if (my_search_handles == 0)
	{	/* No results for empty search */
		*result = lu_common_minvalid;
		return 0;
	}
	
	done = 0;
	while (!done)
	{
		which    = 0;
		largest  = lu_common_minvalid;
		smallest = lu_common_minvalid;
		
		for (i = 0; i < my_search_handles; i++)
		{
			if (my_search_id[i] == lu_common_minvalid)
			{	/* no more hits */
				*result = lu_common_minvalid;
				return 0;
			}
			
			if (largest == lu_common_minvalid ||
			    my_search_id[i] > largest)
			{
				largest = my_search_id[i];
				which = i;
			}
			
			if (my_search_id[i] < smallest)
			{
				smallest = my_search_id[i];
			}
		}
		
		if (smallest == largest)
		{	/* We have a hit */
			*result = smallest;
			done = 1;
			
			if (smallest == 0)
			{	/* There can't be any more hits */
				my_search_id[0] = lu_common_minvalid;
				break;
			}
			else
			{
				/* Find the next earliest record in 0 */
				which = 0;
				smallest--;
			}
		}
		
		/* Ok, we know the smallest, largest, and which the largest is */
		
		if (lu_breader_offset(
			my_search_handle[which], 
			smallest, 
			&index) != 0)
		{	/* The breader died! */
			return -1;
		}
		
		if (index == lu_common_minvalid)
		{	/* No more results */
			my_search_id[which] = lu_common_minvalid;
			if (!done)
			{
				*result = lu_common_minvalid;
			}
			
			return 0;
		}
		
		/* Ok, what is the value at this location? */
		if (lu_breader_read(
			my_search_handle[which], 
			index,
			1,
			&my_search_id[which]) != 0)
		{	/* The breader died! */
			return -1;
		}
	}
	
	return 0;
}

int lu_search_end()
{
	int i;
	
	for (i = 0; i < my_search_handles; i++)
	{
		lu_breader_free(my_search_handle[i]);
	}
	my_search_handles = 0;
	
	return 0;
}
