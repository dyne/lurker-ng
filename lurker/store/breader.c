/*  $Id: breader.c,v 1.2 2002-02-10 03:24:29 terpstra Exp $
 *  
 *  breader.c - Knows how to use the abstracted read interface for buffered access
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

#include "flatfile.h"
#include "breader.h"
#include "btree.h"

/* Now, what's the plan... We want to cache reads.
 *
 * So, clearly we want buffers for each open keyword. However, if a person
 * executed a search for 'foo', it is likely the next search will also be
 * for 'foo'. Therefore, even after a handle has been 'closed'. We keep it
 * around until we need to reclaim the resources.
 * 
 * Also, we know that pulling a contiguous range of records at once is faster
 * than fragmented retrievals. Therefore, we will have underlying buffers
 * that hold LU_PULL_AT_ONCE records.
 *
 * Finally, the 'offset' call is relatively difficult. To implement this, we
 * will be doing a linear prediction as to where to search next. This means
 * we will be pulling records out of sequence.
 *
 * However, since the most common use of the breader/offset call is for the
 * keyword search, and keyword searches proceed end->start, we will want
 * to keep the records that we hit and are earlier than where we searched.
 *
 * So, we need to keep more than one LU_PULL_AT_ONCE buffer in memory. We
 * expire those records which are furthest towards the end when we need
 * a new buffer.
 *
 * Also, the information about where to look for a given offset is highly
 * valuable. So, we will keep more of this summary information than the
 * actual data buffers themselves.
 */

/*------------------------------------------------ Constant parameters */

/* This is the maximum number of cache handles we allow to be used.
 * Since a keyword search must have one handle for every word in the search
 * criterea, and we don't like to allocate memory while running, this will
 * cap the number of keywords allowed in a search.
 */
#define LU_MAX_HANDLES	32

/* This is the size for our pull-records-at once.
 * We retrieve this many message_ids any single disk access.
 */
#define LU_PULL_AT_ONCE	4096

/* This is how much information about the  !!!
 */
#define LU_BOUNDARY_RECORDS	4096

/* This is how many cache records we keep for each keyword. Each one of them
 * holds LU_PULL_AT_ONCE records.
 */
#define	LU_CACHE_RECORDS	4

/* Total cache used: LU_MAX_HANDLES*LU_PULL_AT_ONCE*LU_CACHE_RECORDS*4
 *                   --> 2.0Mb
 * Total offset information: LU_MAX_HANDLES*LU_BOUNDARY_RECORDS*16
 *                   --> 2.0Mb
 */

/*------------------------------------------------ Private types */

/* Must be able to store up to LU_BOUNDARY_RECORDS */
typedef lu_word	my_breader_boundary_ptr;

typedef struct My_Breader_Boundary_T
{
	message_id		key;
	message_id		id;
	
	lu_byte			skew;
	my_breader_boundary_ptr	left;
	my_breader_boundary_ptr	right;
} My_Breader_Boundary;

struct My_Breader_Handle_T
{
	Lu_Flatfile_Handle	handle;
};

/*------------------------------------------------ Private global vars */

static My_Breader_Boundary* table;

/*------------------------------------------------ Private helper methods */

inline int my_breader_compare(message_id a, message_id b)
{
	if (a < b)
		return -1;
	if (a > b) 
		return 1;
	return 0;
}

LU_BTREE_DEFINE(
	breader, 
	my_breader_boundary_ptr, 
	0xFFFFUL,
	My_Breader_Boundary,
	table,
	my_breader_compare)

/*------------------------------------------------ Public component methods */

int lu_breader_init()
{
	return 0;
}

int lu_breader_open()
{
	return 0;
}

int lu_breader_sync()
{
	return 0;
}

int lu_breader_close()
{
	return 0;
}

int lu_breader_quit()
{
	return 0;
}

Lu_Breader_Handle lu_breader_new(
	const char* keyword)
{
	return 0;
}

int lu_breader_offset(
	Lu_Breader_Handle h,
	message_id  id,
	message_id* index)
{
	return -1;
}

int lu_breader_read(
	Lu_Breader_Handle h,
	message_id index,
	message_id count,
	message_id* out)
{
	return -1;
}

void lu_breader_free(
	Lu_Breader_Handle h)
{
}
