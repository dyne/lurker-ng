/*  $Id: keyword.c,v 1.11 2002-02-03 03:10:53 terpstra Exp $
 *  
 *  keyword.c - manages a database for keyword searching
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
 
/* These constants control how much data keyword indexing will try to pull 
 * from the disk at once to take advantage of no seeking.
 */

#include "keyword.h"
#include "io.h"

#include <string.h>
#include <stdlib.h>
#include <db.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifndef assert
#define assert(x) do { if (!x) { printf("\nASSERT FAILURE: %s:%i: '%s'\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
#endif

// #define DEBUG 1

#ifndef DEBUG
#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#else
#define syslog(x, y, ...)
#endif /* HAVE_SYSLOG_H */
#else
#define syslog(x, ...) printf(__VA_ARGS__)
#endif

int	lu_keyword_fd;
DB*	lu_keyword_db;

/* Alright, how do we deal with keyword lookups?
 * 
 * Well, I presume that the reader already understands how to do set
 * set intersection in O(n+m) time when |S1|=n |S2|=m and both are in sorted
 * order. We use this algorithm, but we count from the right -> newer documents
 * are the result of a search in preference to older documents.
 * 
 * We first grab a handle for each keyword that is involved in the search. 
 * Rather than walking all the lists in order, we try to jump ahead to where
 * we expect the record to be. This is done with a linear prediction which
 * falls back to a binary search if the data does not seem uniformly
 * distributed.
 * 
 * Because we will end up reading records ahead of our location when we do
 * our jump prediction, we can cache them. How many records we keep cached
 * in memory for each keyword is configured below.
 */

/* How many records do we pull at a time from the disk.
 * Default: 8192 -> 32k
 */
#ifdef DEBUG
/* Makes failure cases show up if this is small */
#define LU_SPECULATIVE_READ_AHEAD	2
#else
#define LU_SPECULATIVE_READ_AHEAD	8192
#endif

/* How many read_ahead buffers will we cache per keyword
 * Default: 16
 */
#define LU_PER_KEYWORD_BUFFER		16

/* Worst case memory usage: terms*16*8192*4 = 0.5Mb a keyword */

/* This records a fragment of the ondisk record.
 * Offset is the message_id offset from the first record #.
 * used is how much of the buffer is full
 */
struct CacheRecord
{
	message_id	offset;
	message_id	used;
	message_id	buf[LU_SPECULATIVE_READ_AHEAD];
};

/* This is all the records currently cached.
 * The indexes work thusly [where], [where+1], [where+2], ... [where+used]
 * are locations after the last search and are all (mod LU_PER_KEYWORD_BUFFER).
 * used never exceeds LU_PER_KEYWORD_BUFFER.
 */
struct Keyword
{
	Handle		handle;
	message_id	last;
	
	/* The record which is furthest right in our search */
	int 		cache_end_m1;
	
	/* The number of records actually containing data. 
	 * Note: this even counts the record to the right of the search.
	 */
	int 		cache_fill;
	
	struct CacheRecord index[LU_PER_KEYWORD_BUFFER];
};

struct Search_T
{
	int		keywords;
	struct Keyword	keyword[0];
};

/* This function will read into the next available CacheRecord as much of the
 * message ids surrounding offset as possible. It trys to put offset in the
 * middle of the buffer subject to the record boundaries.
 */
static int lu_pull_kw_record(struct Keyword* kw, message_id offset)
{
	struct CacheRecord* record;
	message_id left;
	message_id right;
	
	record = &kw->index[kw->cache_end_m1];
	
	if (lu_handle_records(kw->handle) <= LU_SPECULATIVE_READ_AHEAD)
	{	/* Is the entire record so small it fits in our cache? */
		left  = 0;
		right = lu_handle_records(kw->handle);
	}
	else
	{	/* Would the offset run off the end of the record? */
		if (offset + LU_SPECULATIVE_READ_AHEAD/2 >= 
			lu_handle_records(kw->handle))
		{
			left  = lu_handle_records(kw->handle) - LU_SPECULATIVE_READ_AHEAD;
			right = lu_handle_records(kw->handle);
		}
		else
		{	/* Would the offset end up with us before the front? */
			if (offset <= LU_SPECULATIVE_READ_AHEAD/2)
			{
				left = 0;
				right = LU_SPECULATIVE_READ_AHEAD;
			}
			else
			{	/* Ok, we can just center on the offset. */
				left  = offset - LU_SPECULATIVE_READ_AHEAD/2;
				right = offset + LU_SPECULATIVE_READ_AHEAD/2;
			}
		}
	}
	
	/* Ok, we now have the bounds on the request. */
	record->offset = left;
	record->used   = right - left;
	if (lu_handle_read(kw->handle, record->offset,
		&record->buf[0], record->used) != 0)
	{	/* Failed to read cache */
		return -1;
	}
	
	return 0;
}

/* This will find the id which is less than or equal to id in the current
 * record.
 */
static int lu_gather_id(struct Keyword* kw, message_id id)
{
	struct CacheRecord* record;
	int left, right, mid;
	
	record = &kw->index[kw->cache_end_m1];
	right = record->used;
	left  = 0;
	
	while (right - left > 1)
	{
		mid = (right + left) / 2;
		if (id > record->buf[mid])
		{
			left = mid;
		}
		else if (id < record->buf[mid])
		{
			right = mid;
		}
		else
		{	/* The record exists */
			kw->last = id;
			return 0;
		}
	}
	
	/* It is less than the search criterea */
	kw->last = record->buf[left];
	return 0;
}

/* Find the message_id that is <= id. We require that the caller always call
 * us in descending id order. The result is placed in kw->last.
 */
static int lu_search_find_id(struct Keyword* kw, message_id id)
{
	/*!!! add binary search fallback behaviour */
	
	lu_addr		accum;
	message_id	buf_left;
	message_id	buf_right;
	message_id	off_left;
	message_id	off_right;
	message_id	predict;
	int		last_where;
	struct CacheRecord* record;
	
	/* As always, the search is in range [left, right) */
	
	off_left  = 0;
	off_right = lu_handle_records(kw->handle);
	
	if (kw->cache_end_m1 == -1)
	{	/* We have no records loaded. */
		predict = off_right;
		
		kw->cache_end_m1 = 0;
		kw->cache_fill = 1;
	}
	else
	{	/* Start freeing cache until we find the record, pass it, or
		 * run out of cache.
		 */
		while (1)
		{
			record = &kw->index[kw->cache_end_m1];
			if (id < record->buf[0])
			{	/* It's definitely not here, and the cache
				 * lies to the right (useless)
				 */
				off_right = record->offset;
				
				kw->cache_fill--;
				
				if (kw->cache_end_m1 == 0)
					kw->cache_end_m1 = LU_PER_KEYWORD_BUFFER;
				kw->cache_end_m1--;
			}
			else if (id > record->buf[record->used-1])
			{
				off_left = record->offset + record->used - 1;
			}
			else
			{	/* The answer lies in this range */
				return lu_gather_id(kw, id);
			}
		}
	}
	
	while (1)
	{
		/* Don't do anything stupid */
		if (predict <= off_left)
		{
			predict = off_left + LU_SPECULATIVE_READ_AHEAD/2;
		}
		if (predict >= off_right)
		{
			predict = off_right - LU_SPECULATIVE_READ_AHEAD/2;
		}
		
		if (lu_pull_kw_record(kw, predict) != 0)
			return -1;
		
		/* On which side of our search did this fall? */
		last_where = kw->cache_end_m1;
		record = &kw->index[kw->cache_end_m1];
		if (record->buf[0] > id)
		{	/* it is to the right */
			off_right = record->offset;
		}
		else if (record->buf[record->used - 1] < id)
		{	/* it is to the left - keep this cached */
			if (kw->cache_fill  != LU_PER_KEYWORD_BUFFER)
				kw->cache_fill++;
			kw->cache_end_m1++;
			if (kw->cache_end_m1 == LU_PER_KEYWORD_BUFFER)
				kw->cache_end_m1 = 0;
			
			off_left = record->offset + record->used - 1;
		}
		else
		{	/* the answer lies in this range */
			return lu_gather_id(kw, id);
		}
		
		/* Use what we know to predict again */
		
		record = &kw->index[last_where];
		buf_left  = record->buf[0];
		buf_right = record->buf[record->used - 1];
		
		if (id > buf_left)
		{
			accum = (id - buf_left) * record->used;
			accum /= buf_right - buf_left;
			
			if (accum > lu_handle_records(kw->handle) - record->offset)
				predict = lu_handle_records(kw->handle);
			else
				predict = record->offset + accum;
		}
		else
		{
			accum = (buf_left - id) * record->used;
			accum /= buf_right - buf_left;
			
			if (accum > record->offset)
				predict = 0;
			else
				predict = record->offset - accum;
		}
	}
}

Search lu_new_search(const char* const * words)
{
	Search	out;
	int	i;
	
	/* Count the entries */
	for (i = 0; words[i]; i++)
	{
		/* noop */
	}
	
	if (i == 0)
	{	/* no search? */
		return 0;
	}
	
	out = malloc(sizeof(struct Search_T) + i * sizeof(struct Keyword));
	if (out == 0)
	{
		return 0;
	}
	
	out->keywords = i;
	for (i = 0; i < out->keywords; i++)
	{
		out->keyword[i].handle = lu_open_handle(words[i]);
		out->keyword[i].cache_fill   = 0;
		out->keyword[i].cache_end_m1 = -1;
		
		if (out->keyword[i].handle == 0)
			break;
		
		if (lu_handle_records(out->keyword[i].handle))
		{
			/* Find the last message id */
			if (lu_handle_read(out->keyword[i].handle,
				lu_handle_records(out->keyword[i].handle)-1,
				&out->keyword[i].last,
				1) != 0)
			{
				break;
			}
		}
		else
		{	/* No records -> no hits */
			out->keyword[i].last = lu_kw_invalid;
		}
	}
	
	if (i != out->keywords)
	{
		while (i)
		{
			lu_close_handle(out->keyword[--i].handle);
		}
		
		free(out);
		return 0;
	}
	
	return out;
}

void lu_free_search(Search s)
{
	int i;
	
	for (i = 0; i < s->keywords; i++)
		lu_close_handle(s->keyword[i].handle);
	free(s);
}

int lu_search_skip(Search s, int records)
{
	/*!!! optimize for keywords == 1 */
	while (records--) lu_search_next(s);
	return 0;
}

message_id lu_search_next(Search s)
{
	int		i;
	message_id	big;
	message_id	small;
	int		i_big;
	int		i_small;
	
	/* Remember, the search runs backwards.
	 */
	while (1)
	{
		small   = s->keyword[0].last;
		i_small = 0;
		big     = s->keyword[0].last;
		i_big   = 0;
		
		for (i = 1; i < s->keywords; i++)
		{
			if (s->keyword[i].last == lu_kw_invalid)
			{	/* No more hits available */
				return lu_kw_invalid;
			}
			
			if (s->keyword[i].last < small)
			{
				small   = s->keyword[i].last;
				i_small = i;
			}
			
			if (s->keyword[i].last > big)
			{
				big   = s->keyword[i].last;
				i_big = i;
			}
		}
		
		if (big == small)
		{
			/* We have a hit! */
			/* First, move one of the records forward so the next
			 * call doesn't have them all equal
			 */
			lu_search_find_id(&s->keyword[0], big-1);
			
			/* Spit out the hit */
			return big;
		}
		else
		{
			/* Move the largest to the smallest or earlier */
			if (lu_search_find_id(&s->keyword[i_big], small) != 0)
				return lu_kw_invalid;
		}
	}
		
	return 0;
}


/************************************************************* Testing!! */

int lu_test_index()
{
#ifndef DEBUG
	return 0;
#else
	int stat;
	off_t o1, o2, o3;
	
	printf("Running test on underlying data methods... (must wipe db to succeed)\n");
	
	printf("  lu_cell_type: "); fflush(stdout);
	assert(lu_cell_type( 0) == 0);
	assert(lu_cell_type( 3) == 0);
	assert(lu_cell_type( 4) == 1);
	assert(lu_cell_type( 7) == 1);
	assert(lu_cell_type( 8) == 2);
	assert(lu_cell_type(15) == 2);
	assert(lu_cell_type(16) == 3);
	assert(lu_cell_type(31) == 3);
	assert(lu_cell_type(32) == 4);
	assert(lu_cell_type(lu_kw_invalid) == sizeof(message_id)*8-2);
	assert(lu_cell_type(lu_kw_invalid>>1) == sizeof(message_id)*8-3);
	printf("ok\n");
	
	printf("  lu_create_empty_record: "); fflush(stdout);
	stat = lu_create_empty_record(1, &o1); assert(stat == 0);
	stat = lu_create_empty_record(2, &o2); assert(stat == 0);
	stat = lu_create_empty_record(0, &o3); assert(stat == 0);
	assert(o2 == o1+8*sizeof(message_id));
	assert(o3 == o2+16*sizeof(message_id));
	printf("ok\n");
	
	printf("  lu_free_cell: "); fflush(stdout);
	stat = lu_free_cell(o1, 7);  assert(stat == 0);
	stat = lu_free_cell(o2, 15); assert(stat == 0);
	stat = lu_free_cell(o3, 3);  assert(stat == 0);
	printf("ok\n");
	
	printf("  lu_pop_free_list: "); fflush(stdout);
	stat = lu_pop_free_list(2, &o1); assert(stat == 0);
	stat = lu_pop_free_list(2, &o3); assert(stat == 0);
	assert(o1 == o2);
	assert(o3 == 0);
	printf("ok\n");
	
	printf("  lu_push_fragment_markers: "); fflush(stdout);
	stat = lu_push_fragment_markers(o1, 15, 0); assert(stat == 0);
	stat = lu_free_cell(o1, 0); assert(stat == 0);
	stat = lu_pop_free_list(2, &o2); assert(stat == 0);
	assert(o2 == o1);
	stat = lu_free_cell(o2, 0);
	printf("ok\n");
	
	printf("  lu_allocate_cell: "); fflush(stdout);
	stat = lu_allocate_cell(2, &o2); assert(stat == 0);
	assert(o1 == o2);
	stat = lu_allocate_cell(2, &o3); assert(stat == 0);
	assert(o3 == o1+20*sizeof(message_id));
	stat = lu_free_cell(o2, 15); assert(stat == 0);
	stat = lu_free_cell(o3, 15); assert(stat == 0);
	stat = lu_pop_free_list(2, &o1); assert(stat == 0);
	assert(o1 == o3);
	stat = lu_pop_free_list(2, &o1); assert(stat == 0);
	assert(o1 == o2);
	stat = lu_pop_free_list(2, &o1); assert(stat == 0);
	assert(o1 == 0);
	stat = lu_free_cell(o2, 15); assert(stat == 0);
	stat = lu_free_cell(o3, 15); assert(stat == 0);
	printf("ok\n");
	
	printf("  lu_reset_keyword: "); fflush(stdout);
	stat = lu_reset_keyword("babar", 42); assert(stat == 0);
	stat = lu_reset_keyword("bzzr",  10); assert(stat == 0);
	stat = lu_reset_keyword("babar", 30); assert(stat == 0);
	printf("ok\n");
	
	printf("  lu_locate_keyword: "); fflush(stdout);
	stat = lu_locate_keyword("baz", &o1); assert(stat == 0);
	assert(o1 == 0);
	stat = lu_locate_keyword("babar", &o1); assert(stat == 0);
	assert(o1 == 30);
	stat = lu_locate_keyword("bzzr", &o1); assert(stat == 0);
	assert(o1 == 10);
	printf("ok\n");
	
	printf("\n");
	return 0;
#endif
}

int lu_test_handle()
{
#ifndef DEBUG
	return 0;
#else
	int i, j, stat;
	message_id buf[1024];
	Handle h;
	
	printf("Running test on indexing and handle methods... \n");
	
	printf("  lu_write_keyword_block: "); fflush(stdout);
	for (i = 0; i < 20; i++) buf[i] = i*i;
	stat = lu_write_keyword_block("squares", &buf[0], 20); assert(stat == 0);
	for (i = 0; i < 20; i++) buf[i] = 401 + i;
	stat = lu_write_keyword_block("squares", &buf[0], 20); assert(stat == 0);
	stat = lu_write_keyword_block("squares", &buf[0], 0); assert(stat == 0);
	
	stat = lu_write_keyword_block("small", &buf[0], 0); assert(stat == 0);
	buf[0] = 16;
	stat = lu_write_keyword_block("small", &buf[0], 1); assert(stat == 0);
	buf[0] = 25;
	stat = lu_write_keyword_block("small", &buf[0], 1); assert(stat == 0);
	buf[0] = 26;
	stat = lu_write_keyword_block("small", &buf[0], 1); assert(stat == 0);
	buf[0] = 28;
	stat = lu_write_keyword_block("small", &buf[0], 1); assert(stat == 0);
	
	for (i = 0; i < 100; i++)
	{
		for (j = 0; j < 1024; j++) buf[j] = i * 1024 + j;
		stat = lu_write_keyword_block("big", &buf[0], 1024); assert(stat == 0);
	}
	
	printf("ok\n");
	
	/*!!! test pop_keyword */
	
	printf("  open_handle: "); fflush(stdout);
	h = lu_open_handle("big2"); assert(h != 0);
	assert(lu_handle_records(h) == 0);
	lu_close_handle(h);
	h = lu_open_handle("small"); assert (h != 0);
	assert(lu_handle_records(h) == 4);
	lu_close_handle(h);
	h = lu_open_handle("big"); assert (h != 0);
	assert(lu_handle_records(h) == 102400);
	printf("ok\n");
	
	printf("  lu_handle_read: "); fflush(stdout);
	stat = lu_handle_read(h, 0, &buf[0], 1024); assert(stat == 0);
	for (i = 0; i < 1024; i++) assert(buf[i] == i);
	stat = lu_handle_read(h, 100, &buf[0], 1024); assert(stat == 0);
	for (i = 0; i < 1024; i++) assert(buf[i] == i + 100);
	stat = lu_handle_read(h, 10240, &buf[0], 512); assert(stat == 0);
	for (i = 0; i < 512; i++) assert(buf[i] == i + 10240);
	printf("ok\n");
	
	printf("\n");
	return 0;
#endif
}

int lu_test_search()
{
#ifndef DEBUG
	return 0;
#else
	return -1;
#endif
}
