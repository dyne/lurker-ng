/*  $Id: rbuffer.c,v 1.11 2003-01-04 20:21:21 terpstra Exp $
 *  
 *  rbuffer.c - Implements a buffering system that read combines
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: LGPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation;
 *    version 2 of the License.
 *    
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *    
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "../config.h"
#include "private.h"
#include "avl.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef HAVE_ERRNO_H
# include <errno.h>
#endif

#ifdef HAVE_ASSERT_H
# include <assert.h> 
#endif

#ifndef assert
# define assert(x) do { if (!x) { printf("\nASSERT FAILURE: %s:%i: '%s'\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
#endif

#ifdef DMALLOC
# include <dmalloc.h>
#endif

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
 * cap the number of cached keywords allowed in a search.
 */
#define LU_MAX_HANDLES	32

/* This is the size for our pull-records-at once.
 * We retrieve this many bytes any single disk access.
 */
#define LU_PULL_AT_ONCE	8192

/* This is how many records about offset information we keep.
 */
#define LU_BOUNDARY_RECORDS	512

/* This is how many cache records we keep for each keyword. Each one of them
 * holds LU_PULL_AT_ONCE bytes.
 */
#define	LU_CACHE_RECORDS	8

/* The maximum age of a buffer
 */
#define MAX_AGE			30000

/* Total cache used: LU_MAX_HANDLES*LU_PULL_AT_ONCE*LU_CACHE_RECORDS
 *                   --> 2.0Mb
 * Total offset information: LU_MAX_HANDLES*LU_BOUNDARY_RECORDS*16
 *                   --> 512Kb
 */

/* This is the maximum value ptr can attain.
 */
#define BPTR_MAX	0xFFFFUL

/*------------------------------------------------ Private types */

/* Must be able to store up to LU_BOUNDARY_RECORDS */
typedef unsigned short	bptr;

typedef struct Boundary_T
{
	size_t		key; /* index */
	
	bptr		left;
	bptr		right;
	
	unsigned char	skew;
} Boundary;

typedef struct Cache_T
{
	unsigned char	buffer[LU_PULL_AT_ONCE];
	size_t		offset;
	int		age;
} Cache;

typedef struct Record_T
{
	Cache		cache   [LU_CACHE_RECORDS];
	Boundary	boundary[LU_BOUNDARY_RECORDS];
	
	bptr		boundary_usage;
	bptr		boundary_root;
	
	/* This is the record which corresponds to a given AVL table entry */
	unsigned char*	table_records;
	
	off_t		id;	/* identity of the cache record */
	int		usage;
	int		age;
} Record;

/*------------------------------------------------ Private global vars */

struct Kap_Rbuffer
{
	Record*   	records;
	ssize_t   	recsize;
	
	/* One big malloc which covers all the table_records in records */
	unsigned char*	tables;
};

/*------------------------------------------------ Private helper methods */

inline int compare(size_t a, size_t b)
{
	if (a < b)	return -1;
	if (a > b)	return 1;
	return 0;
}

AVL_DEFINE_INSERT(rb, bptr, BPTR_MAX, Boundary, compare)
AVL_DEFINE_REMOVE(rb, bptr, BPTR_MAX, Boundary, compare)

static int fetch_sector(
	Kap		k,
	const KRecord*	kr,
	Record*		record, 
	off_t		off)
{
	size_t	amount;
	int	oldest = 0;
	int	which  = 0;
	int	i;
	int	out;
	bptr	kill;
	bptr	scan;
	
	Boundary*	avl_table;
	
	assert(off < kr->records);
	off -= (off % (LU_PULL_AT_ONCE/k->rbuffer->recsize));
	
	/* Find the largest off record to dump
	 * free records have preset age MAX_AGE
	 */
	for (i = 0; i < LU_CACHE_RECORDS; i++)
	{
		if (record->cache[i].age != MAX_AGE &&
		    record->cache[i].offset == off)
		{
			/* We already have the sector loaded */
			return 0;
		}
		
		if (record->cache[i].age < MAX_AGE)
			record->cache[i].age++;
		
		if (record->cache[i].age > oldest)
		{
			which = i;
			oldest = record->cache[i].age;
		}
	}
	
	amount = kr->records - off;
	if (amount > LU_PULL_AT_ONCE/k->rbuffer->recsize)
		amount = LU_PULL_AT_ONCE/k->rbuffer->recsize;
	
	record->cache[which].offset = off;
	record->cache[which].age    = 0;
	
#ifdef DEBUG
	printf("****** DISK HIT ****** (%ld %ld)\n", (long)off, (long)amount);
#endif
	
	out = kap_append_read(k, kr,
		off,
		&record->cache[which].buffer[0],
		amount);
	
	if (out != 0) return out;
	
	/* Remember the boundary information we learn about this sector.
	 */
	
	/* We are operating on this rb table */
	avl_table = &record->boundary[0];
	
	scan = record->boundary_root;
	while (scan != BPTR_MAX)
	{
		if (off < avl_table[scan].key)
			scan = avl_table[scan].left;
		else if (off > avl_table[scan].key)
			scan = avl_table[scan].right;
		else
		{	/* We have already seen this key */
			return 0;
		}
	}
	
	if (record->boundary_usage == LU_BOUNDARY_RECORDS)
	{	/* Forget about the rightmost value -- less useful */
		scan = record->boundary_root;
		while (avl_table[scan].right != BPTR_MAX)
			scan = avl_table[scan].right;
		
		record->boundary_root = 
			my_avl_rb_remove(avl_table, record->boundary_root, scan);
		assert (record->boundary_root != BPTR_MAX);
		
		kill = scan;
	}
	else
	{
		kill = record->boundary_usage++;
	}
	
	record->boundary[kill].key = off;
	memcpy(	&record->table_records[kill*k->rbuffer->recsize],
		&record->cache[which].buffer[0],
		k->rbuffer->recsize);
	
	/* Push the new record */
	record->boundary_root = 
		my_avl_rb_insert(avl_table, record->boundary_root, kill);
	assert (record->boundary_root != BPTR_MAX);
	
	return 0;
}

static int find_which(
	Kap	k,
	Record*	record,
	size_t	offset)
{
	int i;
	
	for (i = 0; i < LU_CACHE_RECORDS; i++)
	{
		if (record->cache[i].age != MAX_AGE &&
		    offset >= record->cache[i].offset &&
		    offset < record->cache[i].offset + 
		    		LU_PULL_AT_ONCE/k->rbuffer->recsize)
		{
			return i;
		}
	}
	
	return -1;
}

static int read(
	Kap		k,
	const KRecord*	kr,
	Record*		record, 
	size_t		offset, 
	void*		out,
	size_t		count)
{
	size_t	got, ind;
	int	stat;
	int	i;
	
	while (count)
	{
		stat = fetch_sector(k, kr, record, offset);
		if (stat != 0) return stat;
		
		i = find_which(k, record, offset);
		ind = offset - record->cache[i].offset;
		got = LU_PULL_AT_ONCE - ind;
		
		if (got > count)
		{
			got = count;
		}
			
		memcpy(	out, 
			&record->cache[i].buffer[ind*k->rbuffer->recsize],
			got * k->rbuffer->recsize);
		
		out    = ((unsigned char*)out) + got*k->rbuffer->recsize;
		count  -= got;
		offset += got;
	}
	
	return 0;
}

static void write(
	Kap		k,
	const KRecord*	kr,
	Record*		record, 
	size_t		offset, 
	const void*	out,
	size_t		count)
{
	int i;
	size_t amt = LU_PULL_AT_ONCE/k->rbuffer->recsize;
	
	size_t	start, end;
	
	for (i = 0; i < LU_CACHE_RECORDS; i++)
	{
		/* Only update non-expired cache */
		if (record->cache[i].age == MAX_AGE)
			continue;
		
		/* Only update if the ranges overlap */
		if (offset + count < record->cache[i].offset ||
		    record->cache[i].offset + amt < offset)
			continue;
		
		/* Compute the left-side */
		if (record->cache[i].offset < offset)
			start = offset;
		else	start = record->cache[i].offset;
		
		/* Compute the right-side */
		if (record->cache[i].offset + amt < offset + count)
			end = record->cache[i].offset + amt;
		else	end = offset + count;
		
		/* Copy the write to cache */
		memcpy(	&record->cache[i].buffer[k->rbuffer->recsize*
				(start - record->cache[i].offset)],
			((const unsigned char*)out) + k->rbuffer->recsize*
				(start - offset),
			(end - start) * k->rbuffer->recsize);
	}
}

static int find(
	Kap 		k,
	const KRecord*	kr,
	Record*		record,
	int		(*testfn)(const void* arg, const void* rec),
	const void*	arg,
	ssize_t*	offset,
	void*		rec)
{
	int	which;
	bptr	ptr;
	
	size_t	cache_size;
	size_t	left_off, right_off, mid_off;
	int	l, r, m;
	
	/* This function implements a three-stage binary-search.
	 * First, it searches the boundary cache to narrow the end-points.
	 * Next, it searches disk sectors till it finds one with the end-points
	 * that we need.
	 * Finally, it searches the retrieved sector.
	 */
	
	/* These are the crude end-points that we know */
	left_off = 0;
	right_off = kr->records;
	
	/* The id must lie in the range [left_off, right_off) or not at all */
	/* Another invariant is that left_off is a multiple of LU_PULL_AT_ONCE */
	
#ifdef DEBUG
	printf ("%d %d\n", left_off, right_off);
#endif
	
	/* Now, use the boundary data to refine them */
	ptr = record->boundary_root;
	while (ptr != BPTR_MAX)
	{
#ifdef DEBUG
		printf("--> %d\n", record->boundary[ptr].key);
#endif
		/* We are seeking the largest object which passes */
		
		if (testfn(arg, &record->table_records[ptr*k->rbuffer->recsize]))
		{	/* This object passed the test, so we know
			 * the last match is in the range [m, inf).
			 * Interesect with invariant [l, r) to get [m, r)
			 */
			left_off = record->boundary[ptr].key;
			ptr = record->boundary[ptr].right;
		}
		else
		{	/* This object failed the test, so we know that
			 * the last match must be before it: (-inf, m).
			 * Intersect with invariant [l, r) to get [l, m)
			 */
			right_off = record->boundary[ptr].key;
			ptr = record->boundary[ptr].left;
		}
	}
	
#ifdef DEBUG
	printf("== %d %d\n", left_off, right_off);
#endif
	cache_size = LU_PULL_AT_ONCE / k->rbuffer->recsize;
	
	assert (left_off <= right_off);
	assert (left_off % cache_size == 0);
	
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
	while (right_off - left_off > cache_size)
	{
		assert (left_off % cache_size == 0);
		
		mid_off = (left_off + right_off) / 2;
		mid_off += cache_size-1;
		mid_off -= (mid_off % cache_size);
		
		assert (mid_off % cache_size == 0);
		assert (mid_off > left_off && mid_off < right_off);
		
		fetch_sector(k, kr, record, mid_off);
		which = find_which(k, record, mid_off);
		assert (which != -1);
		
		assert (record->cache[which].offset == mid_off);
		
		/* We are seeking the largest object which passes */
		if (testfn(arg, &record->cache[which].buffer[0]))
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
		*offset = -1;
		return KAP_NOT_FOUND;
	}
	
	/* Invariant was: largest matching element in range [l, r)
	 *                l is sector aligned
	 * Loop criterea failed:  r-l < LU_PULL_AT_ONCE
	 * Test above failed: 0 < r-l
	 * Thus: Pull [l, l+LU_PULL_AT_ONCE) records and the match is in it
	 * (if there is a match)
	 */
	
	fetch_sector(k, kr, record, left_off);
	which = find_which(k, record, left_off);
	assert (which != -1);
	assert (record->cache[which].offset == left_off);
	
	/* Begin binary searching the array */
	l = 0;
	r = right_off - left_off;
	
	assert (r > 0 && r <= cache_size);
	
	/* Invariant is now: hit is in range [l, r) in buffer */
	while (r - l > 1)
	{
		m = (l+r)/2;
		
		if (testfn(arg, &record->cache[which].buffer[m*k->rbuffer->recsize]))
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
	
	if (testfn(arg, &record->cache[which].buffer[l*k->rbuffer->recsize]))
	{	/* Positive match */
		*offset = left_off + l;
		memcpy(	rec, 
			&record->cache[which].buffer[l*k->rbuffer->recsize], 
			k->rbuffer->recsize);
	}
	else
	{	/* This is the only record it could have been, and it fails. */
		*offset = -1;
		assert(l == 0 && left_off == 0);
		return KAP_NOT_FOUND;
	}
		
	return 0;
}

static void purify_record(
	Record* record, off_t id)
{
	int i;
	
	record->boundary_root  = BPTR_MAX;
	record->boundary_usage = 0;
	record->id             = id;
	
	for (i = 0; i < LU_CACHE_RECORDS; i++)
	{
		record->cache[i].offset = 0;
		record->cache[i].age = MAX_AGE;
	}
}

/*------------------------------------------------ Public component methods */

struct Kap_Rbuffer* rbuffer_init(Kap k)
{
	struct Kap_Rbuffer* rbuffer = malloc(sizeof(struct Kap_Rbuffer));
	if (!rbuffer) return 0;
	
	rbuffer->records = 0;
	rbuffer->tables  = 0;
	
	return rbuffer;
}

int kap_rbuffer_open(Kap k, const char* dir, const char* prefix)
{
	int i;
	
	kap_append_get_recordsize(k, &k->rbuffer->recsize);
	
	k->rbuffer->records = malloc(sizeof(Record)*LU_MAX_HANDLES);
	k->rbuffer->tables  = malloc(k->rbuffer->recsize
				*LU_MAX_HANDLES*LU_BOUNDARY_RECORDS);
	
	if (!k->rbuffer->records || !k->rbuffer->tables)
		return ENOMEM;
	
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		k->rbuffer->records[i].id    = -1;
		k->rbuffer->records[i].usage = 0;
		k->rbuffer->records[i].age   = 0;
		
		k->rbuffer->records[i].table_records = 
			&k->rbuffer->tables[k->rbuffer->recsize
				*i*LU_BOUNDARY_RECORDS];
	}
	
	return 0;
}

int kap_rbuffer_sync(Kap k)
{
	return 0;
}

int kap_rbuffer_close(Kap k)
{
	if (k->rbuffer->records) free(k->rbuffer->records); k->rbuffer->records = 0;
	if (k->rbuffer->tables)  free(k->rbuffer->tables);  k->rbuffer->tables  = 0;
	
	return 0;
}

/*------------------------------------------------ Public accessor methods */

int kap_rbuffer_kopen(Kap k, const KRecord* kr)
{
	static int age = 1;
	
	int	i;
	int	best;
	
	/* Don't cache anything this small - it would be a waste */
	if (kr->records < 2)
		return 0;
	
	/* Firstly, see if we already have this handle open.
	 */
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		if (k->rbuffer->records[i].id == kr->jumps[0])
		{
			break;
		}
	}
	
	if (i == LU_MAX_HANDLES)
	{	/* Not loaded. Look for the largest aged free one. */
		best = -1;
		
		for (i = 0; i < LU_MAX_HANDLES; i++)
		{
			if (k->rbuffer->records[i].usage == 0 &&
				(best == -1 || k->rbuffer->records[i].age
					< k->rbuffer->records[best].age))
			{
				best = i;
			}
		}
		
		if (best == -1)
		{	/* Unable to find an unused record */
			/* No big deal - just uncached */
			return 0;
		}
		
		purify_record(&k->rbuffer->records[best], kr->jumps[0]);
	}
	else
	{
		best = i;
	}
	
	k->rbuffer->records[best].usage++;
	k->rbuffer->records[best].age = age++;
	
	if (age == MAX_AGE)
	{	/* Cycle the age by resetting everyone */
		age = 1;
		for (i = 0; i < LU_MAX_HANDLES; i++)
		{
			k->rbuffer->records[i].age = 0;
		}
	}
	
	return 0;
}

int kap_rbuffer_kclose(Kap k, const KRecord* kr)
{
	int i;	
	
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		if (k->rbuffer->records[i].id == kr->jumps[0])
		{
			/* On the last reference the record is up for kopens.
			 * However, we still retain boundary data and cache.
			 */
			k->rbuffer->records[i].usage--;
		}
	}
	
	return 0;
}

int kap_rbuffer_read(Kap k, const KRecord* kr, size_t offset, void* out, size_t amount)
{
	int i;	
	
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		if (k->rbuffer->records[i].id == kr->jumps[0])
		{
			return read(k, kr, &k->rbuffer->records[i],
				offset, out, amount);
		}
	}
	
#ifdef DEBUG
	printf("xxxxx READ CAHCE MISS %ld xxxxx\n", (long)kr->records);
#endif	
	/* uncached */
	return kap_append_read(k, kr, offset, out, amount);
}

void kap_rbuffer_write(Kap k, const KRecord* kr, 
	size_t offset, const void* out, size_t amount)
{
	int i;	
	
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		if (k->rbuffer->records[i].id == kr->jumps[0])
		{
			write(k, kr, &k->rbuffer->records[i],
				offset, out, amount);
			return;
		}
	}
	
#ifdef DEBUG
	printf("xxxxx WRITE CAHCE MISS %ld xxxxx\n", (long)kr->records);
#endif	
}

int kap_rbuffer_find(Kap k, KRecord* kr,
	int (*testfn)(const void* arg, const void* rec), const void* arg,
	ssize_t* offset, void* rec)
{
	int i;	
	
	for (i = 0; i < LU_MAX_HANDLES; i++)
	{
		if (k->rbuffer->records[i].id == kr->jumps[0])
		{
			return find(k, kr, &k->rbuffer->records[i],
				testfn, arg, offset, rec);
		}
	}

#ifdef DEBUG
	printf("xxxxx FIND CAHCE MISS %ld xxxxx\n", (long)kr->records);
#endif	
	/* uncached */
	return kap_append_find(k, kr, testfn, arg, offset, rec);
}
