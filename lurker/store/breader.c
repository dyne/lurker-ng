/*  $Id: breader.c,v 1.16 2002-06-11 13:48:34 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "keyword.h"

#include "flatfile.h"
#include "breader.h"
#include "btree.h"

#include <stdlib.h>
#include <string.h>

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

/* This is how many records about offset information we keep.
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

/* This is the maximum value my_breader_ptr can attain.
 */
#define MY_BREADER_PTR_MAX	0xFFFFUL

/*------------------------------------------------ Private types */

/* Must be able to store up to LU_BOUNDARY_RECORDS */
typedef lu_word	my_breader_ptr;

typedef struct My_Breader_Boundary_T
{
	message_id	key;	/* id */
	message_id	index;
	
	lu_byte		skew;
	my_breader_ptr	left;
	my_breader_ptr	right;
} My_Breader_Boundary;

typedef struct My_Breader_Cache_T
{
	message_id buffer[LU_PULL_AT_ONCE];
	message_id offset;
} My_Breader_Cache;

typedef struct My_Breader_Record_T
{
	My_Breader_Cache	cache   [LU_CACHE_RECORDS];
	My_Breader_Boundary	boundary[LU_BOUNDARY_RECORDS];
	
	my_breader_ptr		boundary_usage;
	my_breader_ptr		boundary_root;
	
	Lu_Flatfile_Handle	flatfile;
	message_id		count;
	message_id		last_id;
	
	char			keyword[LU_KEYWORD_LEN+1];
	int			usage;
	lu_quad			age;
} My_Breader_Record;

/*------------------------------------------------ Private global vars */

static My_Breader_Record*   my_breader_records;
static My_Breader_Boundary* my_breader_table;

/*------------------------------------------------ Private helper methods */

inline int my_breader_compare(message_id a, message_id b)
{
	if (a < b)
		return -1;
	if (a > b) 
		return 1;
	return 0;
}

int my_breader_bounded(void* bound, message_id id)
{
	return (id <= *(message_id*)bound);
}

LU_BTREE_DEFINE(
	breader, 
	my_breader_ptr, 
	MY_BREADER_PTR_MAX,
	My_Breader_Boundary,
	my_breader_table,
	my_breader_compare)

static int my_breader_fetch_sector(
	My_Breader_Record* record, 
	message_id index)
{
	message_id	amount;
	message_id	oldest = 0;
	int		which  = 0;
	int		i;
	int		out;
	my_breader_ptr	tmp;
	
	assert(index < record->count);
	index -= (index % LU_PULL_AT_ONCE);
	
	/* Find the largest index record to dump
	 * Note, free = lu_common_minvalid -- the largest possible value
	 */
	for (i = 0; i < LU_CACHE_RECORDS; i++)
	{
		if (record->cache[i].offset == index)
		{
			/* We already have the sector loaded */
			return 0;
		}
		
		if (record->cache[i].offset > oldest)
		{
			which = i;
			oldest = record->cache[i].offset;
		}
	}
	
	amount = record->count - index;
	if (amount > LU_PULL_AT_ONCE)
		amount = LU_PULL_AT_ONCE;
	
	record->cache[which].offset = index;
	
#ifdef DEBUG
	printf("****** DISK HIT ****** (%d %d)\n", index, amount);
#endif
	
	out = lu_flatfile_handle_read(
		record->flatfile,
		index,
		&record->cache[which].buffer[0],
		amount);
	
	if (out != 0) return out;
	
	/* Remember the boundary information we learn about this sector.
	 */
	
	if (record->boundary_usage == LU_BOUNDARY_RECORDS)
	{
		/*!!! Should free a record for reuse */
		return 0;
	}
	
	record->boundary[record->boundary_usage].key   = record->cache[which].buffer[0];
	record->boundary[record->boundary_usage].index = index;
	
	/* Push the new record and increase usage if it's not a duplicate */
	my_breader_table = &record->boundary[0];
	tmp = my_btree_breader_insert(record->boundary_root, record->boundary_usage);
	if (tmp != MY_BREADER_PTR_MAX)
	{
		record->boundary_root = tmp;
		record->boundary_usage++;
	}
	
	return 0;
}

static int my_breader_find_which(
	My_Breader_Record* record,
	message_id offset)
{
	int i;
	
	for (i = 0; i < LU_CACHE_RECORDS; i++)
	{
		if (offset >= record->cache[i].offset &&
		    offset < record->cache[i].offset + LU_PULL_AT_ONCE)
		{
			return i;
		}
	}
	
	return -1;
}

static int my_breader_read(
	My_Breader_Record* record, 
	message_id offset, 
	message_id count,
	message_id* out)
{
	message_id	got, ind;
	int		stat;
	int		i;
	
	while (count)
	{
		stat = my_breader_fetch_sector(record, offset);
		if (stat != 0) return stat;
		
		i = my_breader_find_which(record, offset);
		ind = offset - record->cache[i].offset;
		got = LU_PULL_AT_ONCE - ind;
		
		if (got > count)
		{
			got = count;
		}
			
		memcpy(	out, 
			&record->cache[i].buffer[ind],
			got * sizeof(message_id));
		
		out    += got;
		count  -= got;
		offset += got;
	}
	
	return 0;
}

static int my_breader_find(
	My_Breader_Record*	record,
	int			(*test)(void* arg, message_id id),
	void*			arg,
	message_id*		offset)
{
	int		which;
	my_breader_ptr	ptr;
	
	message_id	left_off, right_off, mid_off;
	int		l, r, m;
	
	/* This function implements a three-stage binary-search.
	 * First, it searches the boundary cache to narrow the end-points.
	 * Next, it searches disk sectors till it finds one with the end-points
	 * that we need.
	 * Finally, it searches the retrieved sector.
	 */
	
	/* These are the crude end-points that we know */
	left_off = 0;
	right_off = record->count;
	
	/* The id must lie in the range [left_off, right_off) or not at all */
	/* Another invariant is that left_off is a multiple of LU_PULL_AT_ONCE */
	
#ifdef DEBUG
	printf ("%d %d\n", left_off, right_off);
#endif
	
	/* Now, use the boundary data to refine them */
	ptr = record->boundary_root;
	while (ptr != MY_BREADER_PTR_MAX)
	{
#ifdef DEBUG
		printf("--> %d %d\n", record->boundary[ptr].key,
					record->boundary[ptr].index);
#endif
		/* We are seeking the largest object which passes */
		
		if (test(arg, record->boundary[ptr].key))
		{	/* This object passed the test, so we know
			 * the last match is in the range [m, inf).
			 * Interesect with invariant [l, r) to get [m, r)
			 */
			left_off = record->boundary[ptr].index;
			ptr = record->boundary[ptr].right;
		}
		else
		{	/* This object failed the test, so we know that
			 * the last match must be before it: (-inf, m).
			 * Intersect with invariant [l, r) to get [l, m)
			 */
			right_off = record->boundary[ptr].index;
			ptr = record->boundary[ptr].left;
		}
	}
	
#ifdef DEBUG
	printf("== %d %d\n", left_off, right_off);
#endif
	assert (left_off <= right_off);
	assert (left_off % LU_PULL_AT_ONCE == 0);
	
	/* Ok, using the boundary data we now have a range in which it must
	 * lie. Lets refine this by reading the disk.
	 */
	
	/* Note, even though we pull an entire sector, only testing the first 
	 * position is helpful. This is for several reasons:
	 *  Sometimes the test is expensive so the extra check is a loss
	 *  Most importantly, there is no way to use the data to preserve
	 *  the invariant that left is boundary aligned:
	 *   Disk range, [a, b], cases: (note b cannot be aligned)
	 *     a &  b -> [b, inf)  -> [b, r) --- unaligned!!
	 *     a & !b -> [a, b)    -> [a, b)
	 *    !a & !b -> (-inf, a) -> [l, a)
	 *   Further, the problem is symmetric with a flipped number line to
	 *   'find the first element with a given property' so this doesn't 
	 *   help.
	 *   Before, yaou change this code, REALLY think about it. I thought 
	 *   about it for hours before I wrote it, and then later found it was 
	 *   wrong because of the alignment issue. This is the best way. Honest.
	 */
	while (right_off - left_off > LU_PULL_AT_ONCE)
	{
		assert (left_off % LU_PULL_AT_ONCE == 0);
		
		mid_off = (left_off + right_off) / 2;
		mid_off += LU_PULL_AT_ONCE-1;
		mid_off -= (mid_off % LU_PULL_AT_ONCE);
		
		assert (mid_off % LU_PULL_AT_ONCE == 0);
		assert (mid_off > left_off && mid_off < right_off);
		
		my_breader_fetch_sector(record, mid_off);
		which = my_breader_find_which(record, mid_off);
		assert (which != -1);
		
		assert (record->cache[which].offset == mid_off);
		
		/* We are seeking the largest object which passes */
		if (test(arg, record->cache[which].buffer[0]))
		{	/* This object passes, so: [m, inf) -> [m, r) */
			left_off = mid_off;
		}
		else
		{	/* This object fails, so: [-inf, m) -> [l, m) */
			right_off = mid_off;
		}
	}
	
	assert (right_off >= left_off);
	
	if (right_off == left_off)
	{	/* No matches */
		assert(left_off == 0);
		*offset = lu_common_minvalid;
		return 0;
	}
	
	/* Invariant was: largest matching element in range [l, r)
	 *                l is sector aligned
	 * Loop criterea failed:  r-l < LU_PULL_AT_ONCE
	 * Test above failed: 0 < r-l
	 * Thus: Pull [l, l+LU_PULL_AT_ONCE) records and the match is in it
	 * (if there is a match)
	 */
	
	my_breader_fetch_sector(record, left_off);
	which = my_breader_find_which(record, left_off);
	assert (which != -1);
	assert (record->cache[which].offset == left_off);
	
	/* Begin binary searching the array */
	l = 0;
	r = right_off - left_off;
	
	assert (r > 0 && r <= LU_PULL_AT_ONCE);
	
	/* Invariant is now: hit is in range [l, r) in buffer */
	while (r - l > 1)
	{
		m = (l+r)/2;
		
		if (test(arg, record->cache[which].buffer[m]))
		{	/* This object passes, so: [m, inf) -> [m, r) */
			l = m;
		}
		else
		{	/* This object fails, so: [-inf, m) -> [l, m) */
			r = m;
		}
	}
	
	/* Ok, the range the answer lies in is now [l, r), however,
	 * r-l <= 1 thus the answer lies in [l, l+1) = [l, l]
	 * Test to see that it satisfies the test!
	 */
	
	if (test(arg, record->cache[which].buffer[l]))
	{	/* Positive match */
		*offset = left_off + l;
	}
	else
	{	/* This is the only record it could have been, and it fails. */
		*offset = lu_common_minvalid;
		assert(l == 0 && left_off == 0);
	}
		
	return 0;
}

static void my_breader_purify_record(
	My_Breader_Record* record, 
	const char* keyword)
{
	int i;
	
	strncpy(&record->keyword[0], keyword, LU_KEYWORD_LEN);
	record->keyword[LU_KEYWORD_LEN] = 0;
	
	record->boundary_root  = MY_BREADER_PTR_MAX;
	record->boundary_usage = 0;
	
	for (i = 0; i < LU_CACHE_RECORDS; i++)
	{
		record->cache[i].offset = lu_common_minvalid;
	}
}

/*------------------------------------------------ Public component methods */

int lu_breader_init()
{
	int i;
	
	my_breader_records = malloc(sizeof(My_Breader_Record)*LU_MAX_HANDLES);
	if (!my_breader_records)
	{
		fprintf(stderr, "Failed to allocate storage for breader buffers\n");
		return -1;
	}
	
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		my_breader_records[i].usage = 0;
		my_breader_records[i].age   = 0;
	}
	
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
	if (my_breader_records)
	{
		free(my_breader_records);
		my_breader_records = 0;
	}
	
	return 0;
}

/*------------------------------------------------ Public accessor methods */

Lu_Breader_Handle lu_breader_new(
	const char* keyword)
{
	static lu_quad age = 1;
	
	message_id new_off;
	int        which;
	
	int	i;
	int	best;
	
	/* Firstly, see if we already have this handle open.
	 */
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		if (!strcmp(&my_breader_records[i].keyword[0], keyword))
		{
			break;
		}
	}
	
	if (i == LU_MAX_HANDLES)
	{	/* Not loaded. Look for the largest aged free one. */
		best = -1;
		
		for (i = 0; i < LU_MAX_HANDLES; i++)
		{
			if (my_breader_records[i].usage == 0 &&
				(best == -1 || my_breader_records[i].age
					< my_breader_records[best].age))
			{
				best = i;
			}
		}
		
		if (best == -1)
		{	/* Unable to find an unused record */
			return 0;
		}
		
		my_breader_purify_record(&my_breader_records[best], keyword);
	}
	else
	{
		best = i;
	}
	
	if (my_breader_records[best].usage == 0)
	{
		my_breader_records[best].flatfile =
			lu_flatfile_open_handle(keyword);
		if (my_breader_records[best].flatfile == 0)
		{	/* Unable to access underlying flatfile */
			return 0;
		}
		
		new_off = lu_flatfile_handle_records(
				my_breader_records[best].flatfile);
		
		/* Kill off cache that is at the eof */
		if (my_breader_records[best].count != new_off)
		{
			which = my_breader_find_which(
				&my_breader_records[best],
				my_breader_records[best].count);
			if (which != -1)
			{
				my_breader_records[best].cache[which].offset = 
					lu_common_minvalid;
			}
			my_breader_records[best].count = new_off;
		}
		
		my_breader_records[best].last_id = lu_common_minvalid;
		
		if (my_breader_records[best].count > 0)
		{
			my_breader_read(
				&my_breader_records[best], 
				my_breader_records[best].count - 1, 
				1,
				&my_breader_records[best].last_id);
		}
	}
	
	my_breader_records[best].usage++;
	my_breader_records[best].age = age++;
	
	if (age == 0xFFFFFFFFUL) /* a big number that fits in a quad */
	{	/* Cycle the age by resetting everyone */
		age = 1;
		for (i = 0; i < LU_MAX_HANDLES; i++)
		{
			my_breader_records[i].age = 0;
		}
	}
	
	return best+1;
}

int lu_breader_offset(
	Lu_Breader_Handle	h,
	int			(*test)(void* arg, message_id id),
	void*			arg,
	message_id*		index)
{
	My_Breader_Record* record;
	
	assert (h <= LU_MAX_HANDLES && h > 0);
	assert (my_breader_records[h-1].usage > 0);
	
	record = &my_breader_records[h-1];
	
	return my_breader_find(record, test, arg, index);
}

int lu_breader_offset_id(
	Lu_Breader_Handle	h,
	message_id		id,
	message_id*		index)
{
	My_Breader_Record* record;
	
	assert (h <= LU_MAX_HANDLES && h > 0);
	assert (my_breader_records[h-1].usage > 0);
	
	record = &my_breader_records[h-1];
	
	/* Find the largest record with the propery <= id */
	return my_breader_find(record, &my_breader_bounded, &id, index);
}

message_id lu_breader_records(
	Lu_Breader_Handle h)
{
	My_Breader_Record* record;
	
	assert (h <= LU_MAX_HANDLES && h > 0);
	assert (my_breader_records[h-1].usage > 0);
	
	record = &my_breader_records[h-1];
	
	return record->count;
}

message_id lu_breader_quick_records(
	const char* keyword)
{
	return lu_flatfile_records(keyword);
}

message_id lu_breader_last(
	Lu_Breader_Handle h)
{
	My_Breader_Record* record;
	
	assert (h <= LU_MAX_HANDLES && h > 0);
	assert (my_breader_records[h-1].usage > 0);
	
	record = &my_breader_records[h-1];
	
	return record->last_id;
}

int lu_breader_read(
	Lu_Breader_Handle h,
	message_id index,
	message_id count,
	message_id* out)
{
	My_Breader_Record* record;
	
	assert (h <= LU_MAX_HANDLES && h > 0);
	assert (my_breader_records[h-1].usage > 0);
	
	record = &my_breader_records[h-1];
	
	return my_breader_read(record, index, count, out);
}

void lu_breader_free(
	Lu_Breader_Handle h)
{
	assert (h <= LU_MAX_HANDLES && h > 0);
	assert (my_breader_records[h-1].usage > 0);
	
	my_breader_records[h-1].usage--;
	if (my_breader_records[h-1].usage == 0)
	{
		/* On the last reference, close the flatfile handle.
		 * However, we still retain boundary data and cache.
		 */
		lu_flatfile_close_handle(my_breader_records[h-1].flatfile);
	}
}
