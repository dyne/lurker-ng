/*  $Id: search.c,v 1.10 2002-06-23 11:47:46 terpstra Exp $
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
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "keyword.h"

#include "breader.h"
#include "search.h"

#include <st.h>
#include <ctype.h>
#include <stdio.h>

/* Our search algorithm does a simple set intersection on several ordered 
 * lists. This is done by walking the lists in order. However, the underlying
 * mechanism will not touch all the intervening records, but instead jumps to 
 * the correct location. This allows for us to work well even on intersecting
 * a common word with a rare one.
 *
 * To predict the number of hits we use the normal equations solution to the
 * least squares problem. Although QR might be better, the least squares 
 * solution is easy to implement and debug. Plus, it's faster and since we
 * don't expect a great fitting, the QR method would often be a waste.
 *
 * That said, here is how we model the system:
 * x (hit)	y (offset)	
 * 1		1000
 * 2		963
 * 3		500
 * 4		440
 * ...
 *
 * So, we want to find the best fit line 'ax + b = y' and then predict at 
 * which value of x the line crosses zero (no more hits).
 * 
 * Let A = [ x 1 ], c = [ a b ]^T, M = A^T*A, z = A^T*y
 * Then, we want to solve Mc = z to get our coefficients.
 *
 * Note M = [ sum x^2 sum x ], z = [ sum xy ]
 *          [ sum x   sum 1 ]      [ sum y  ]
 *
 * Then M^-1 = [  M11 -M01 ] / (M00*M11 - M10*M01)
 *             [ -M10  M00 ]
 * 
 * Then [ a b ]^T = [ M11*z0-M01*z1  -M10*z0+M00*z1 ] / (M00*M11-M10*M01)
 * Solve ax + b = 0 -> x = -b / a = (M01*z0-M00*z1)/(M11*z0-M01*z1)
 *                    Symmetric -------^
 *
 * Strictly speaking, we should sum from smallest to largest... But...
 * Well, that would take effort! We only do at most 1000 terms... double
 * is big. If you don't like it---you implement the 3*messge_id math methods.
 */

/*------------------------------------------------ Constant parameters */

/* This is the maximum terms allowed in a search.
 */
#define LU_MAX_TERMS	16

/*------------------------------------------------ Private global vars */

static Lu_Breader_Handle	my_search_handle[LU_MAX_TERMS];
static message_id		my_search_id[LU_MAX_TERMS];
static int			my_search_handles = 0;
static st_mutex_t		my_search_mutex;

static double		my_search_m00;	/* +xx */
static double		my_search_m01;	/* +x */
static message_id	my_search_m11;	/* +1 */
static double		my_search_z0;	/* +xy */
static double		my_search_z1;	/* +y */

/*------------------------------------------------- Public component methods */

int lu_search_init()
{
	my_search_mutex = st_mutex_new();
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
	st_mutex_destroy(my_search_mutex);
	return 0;
}

/*------------------------------------------------- Public component methods */

int lu_search_start(
	const char* keywords,
	const char** error)
{
	message_id	predict;
	char		buf[LU_KEYWORD_LEN+1];
	char*		w;
	char*		e;
	
	st_mutex_lock(my_search_mutex);
	
	e = &buf[sizeof(buf)-1];
	
	if (my_search_handles != 0)
	{
		*error = _("Already executing a search");
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
			*error = _("Keyword truncated");
		
			/* Skip over the overflow */
			while (*keywords && !isspace(*keywords)) keywords++;
		}
		
		*w = 0;
		if (buf[0] != 0)
		{
			if (my_search_handles == LU_MAX_TERMS)
			{
				*error = _("Too many search terms");
				continue;
			}
			
			my_search_handle[my_search_handles] = 
				lu_breader_new(&buf[0]);
			
			if (my_search_handle[my_search_handles] == 0)
			{
				*error = _("Unable to obtain a breader");
				lu_search_end(&predict);
				return -1;
			}
			
			my_search_id[my_search_handles] = 
				lu_breader_last(my_search_handle[my_search_handles]);
			
			my_search_handles++;
		}
	}
	

	my_search_z0 = my_search_z1 = 0;
	my_search_m00 = my_search_m01 = my_search_m11 = 0;
	
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
	
	done = 0;
	
	if (my_search_handles == 0)
	{	/* No results for empty search */
		*result = lu_common_minvalid;
		done = 1;
	}
	
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
				break;
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
		
		if (lu_breader_offset_id(
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
			break;
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
	
	if (*result != lu_common_minvalid)
	{
		my_search_m11 += 1;
		my_search_m01 += my_search_m11;
		my_search_m00 += my_search_m11*my_search_m11;
		my_search_z0  += my_search_m11 * *result;
		my_search_z1  += *result;
	}
	else
	{	/* We have seen all the hits -- give exact answer:
		 * x 0         y 1
		 * x <answer>  y 0
		 */
		my_search_m00 =  my_search_m11;
		my_search_m00 *= my_search_m11;
		my_search_m01 = my_search_m11;
		my_search_m11 = 2;
		my_search_z0  = 0;
		my_search_z1  = 1;
	}
	
	return 0;
}

int lu_search_end(message_id* predict)
{
	double div, x;
	int i;
	
	for (i = 0; i < my_search_handles; i++)
	{
		lu_breader_free(my_search_handle[i]);
	}
	my_search_handles = 0;
	
#ifdef DEBUG
	printf("M = [ %e %e ], z = [ %e ]\n", my_search_m00, my_search_m01, my_search_z0);
	printf("    [ %e %e ]      [ %e ]\n", my_search_m01, (double)my_search_m11, my_search_z1);
	
	printf("a = %e\n",
		(my_search_m11*my_search_z0 -my_search_m01*my_search_z1) /
		(my_search_m00*my_search_m11-my_search_m01*my_search_m01));
	printf("b = %e\n",
		(my_search_m00*my_search_z1 -my_search_m01*my_search_z0) /
		(my_search_m00*my_search_m11-my_search_m01*my_search_m01));
#endif
	
	div = my_search_m11*my_search_z0-my_search_m01*my_search_z1;
	if (div >= 1 || div <= -1)
	{
		x = (my_search_m01*my_search_z0-my_search_m00*my_search_z1) / div;
		*predict = x;
#ifdef DEBUG
		printf("I predict: %e\n", x);
#endif
	}
	else
	{
		*predict = lu_common_minvalid;
	}
	
	st_mutex_unlock(my_search_mutex);
	
	return 0;
}
