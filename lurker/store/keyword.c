/*  $Id: keyword.c,v 1.8 2002-01-27 02:40:23 terpstra Exp $
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

#define DEBUG 1

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

/* Ok, how does this work? (indexing)
 *
 * So, first we have the db3 btree which turns keyword strings into their
 * flat file offsets.
 *
 * Then in the flat file we have lists of document ids for that keyword.
 * These lists MUST be in sorted order.
 *
 * Block sizes may be: 4, 8, 16, 32, 64, 128, 256, ..., 2^(bits in message_id)
 * We maintain a list of free block sizes at the start of the flat file. For
 * each block size we have a head pointer. Each free block's first off_t
 * bytes refer to the next available free block.
 *
 * Whenever we need a given block size, first we check the free list. If we
 * have a free block available, it is assigned. Otherwise, we grab a new
 * block off the end of the file.
 *
 * Non-free (used) blocks have this structure:
 * <4 byte record count>
 * The document ids follow immediately in sorted order.
 *
 * The record count does not tell you how big the record is. However, you
 * can check if the end of the cell for 0xFFFFFFFFUL. If it has this value,
 * it is twice as big otherwise it fits in the cell_type of messages. This
 * works b/c the record count of the next entry could never be this large.
 *
 * If the first docid == 0xFFFFFFFFUL then there is a special case:
 * In this case, the first part of the record is located elsewhere.
 * The next off_t sized bytes say where.
 * The record count at the other location says how much of the data is there.
 *
 * As document ids are appended to a keyword we increment the counter and 
 * write to the end of the record. If the record would overflow, we move
 * the data to a block of double the size.
 *
 * Worst case scenario we waste 3/4 of the consumed file: The records could
 * all be exactly half full. Their old locations could never have been 
 * reclaimed. This is not too bad and it is a very unlikely worst case. It
 * is far more likely that each record is 3/4 full, and nearly all the 
 * free records have been reclaimed -> 1/4 wasted. Of course, 0/4 is possible
 * also extremely unlikely.
 *
 * We define an offset of '0' to be invalid. The front of the file holds a
 * table, so this address is never needed.
 *
 * How the direct access and search methods work are explained further down.
 */

/* We don't allow 2^0 or 2^1 sized blocks.
 * We do allow up to 2^2 ... 2^(bits in message_id) sized blocks
 */
#define LU_TABLE_SIZE	(sizeof(message_id) * 8 - 1)

/* A constant with 0xFFF... in it */
message_id lu_kw_invalid;

/* Helper method to do a complete write 
 */
static ssize_t lu_write_full(int fd, const void* buf, size_t count, const char* msg)
{
	ssize_t out;
	ssize_t did;
	
	if (count == 0) return 0;
	
	out = 0;
	while (count)
	{
		did = write(fd, buf, count);
		
		if (did <= 0 && errno != EINTR)
		{
			syslog(LOG_ERR, "%s keyword.flat: %s\n", 
				msg, strerror(errno));
			return -1;
		}
		
		count -= did;
		buf   += did;
		out   += did;
	}
	
	return out;
}

/* Helper method to do a complete read
 */
static ssize_t lu_read_full(int fd, void* buf, size_t count, const char* msg)
{
	ssize_t out;
	ssize_t did;
	
	if (count == 0) return 0;
	
	out = 0;
	while (count)
	{
		did = read(fd, buf, count);
		
		if (did < 0 && errno != EINTR)
		{
			syslog(LOG_ERR, "%s keyword.flat: %s\n", 
				msg, strerror(errno));
			return -1;
		}
		
		if (did == 0) return out;
		
		count -= did;
		buf   += did;
		out   += did;
	}
	
	return out;
}

/* Helper method which does a seek and then a write
 */
static ssize_t lu_swrite(int fd, off_t off, const void* buf, size_t count, const char* msg)
{
	if (lseek(fd, off, SEEK_SET) != off)
	{
		syslog(LOG_ERR, "%s seeking keyword.flat (%lld): %s\n",
			msg, off, strerror(errno));
		return -1;
	}
	
	return lu_write_full(fd, buf, count, msg);
}

/* Helper method which does a seek and then a write
 */
static ssize_t lu_sread(int fd, off_t off, void* buf, size_t count, const char* msg)
{
	if (lseek(fd, off, SEEK_SET) != off)
	{
		syslog(LOG_ERR, "%s seeking keyword.flat (%lld): %s\n",
			msg, off, strerror(errno));
		return -1;
	}
	
	return lu_read_full(fd, buf, count, msg);
}

/* This checks if the keyword db is empty. If it is, we write empty records
 * for the free list.
 */
int lu_keyword_init()
{
	/* We need to be able to handle having a keyword which is in
	 * every message_id. There are 8*sizeof(message_id) bits.
	 * Supposing we allow for 0 to bits or bits+1 records.
	 */
	
	off_t records[LU_TABLE_SIZE];
	off_t off = lseek(lu_keyword_fd, 0, SEEK_END);
	
	memset(&lu_kw_invalid, 0xFF, sizeof(message_id));
	
	if (off == -1)
	{
		perror("lseek'ing the keyword.flat");
		return -1;
	}
	
	if (off < sizeof(message_id) * LU_TABLE_SIZE)
	{	/* It's not got the free tables */
		if (off != 0)
		{	/* Wtf there was stuff in it?! - it's not new */
			fprintf(stderr, "keyword.flat is corrupt\n");
			return -1;
		}
		
		memset(&records[0], 0, sizeof(records));
		if (write(lu_keyword_fd, &records[0], sizeof(records)) 
			!= sizeof(records))
		{
			perror("initializing keyword.flat");
			return -1;
		}
	}
	
	return 0;
}

/* What is the free-block index for this size.
 */
static char lu_cell_type(message_id hits)
{
	/* The first record is used by the counter. This means that what
	 * we need is actually hits + 1.
	 *
	 * If the number needed if 5-8 we need 8, 9-16 we need 16.
	 *
	 * However, the value we get is one less ...
	 * ie: 4-7 = 8 = 2^3, 8-15 = 16 = 2^4.
	 *
	 * This is perfect. We simply find the highest bit and put 2^(n+1)
	 * Then, knock off 2 b/c we don't allow 2^0 or 2^1
	 */
	
	char out = 0;
	
	/* This code counts the number to the highest bit */
	while (hits)
	{
		out++;
		hits >>= 1;
	}
	
	if (out < 2)
	{	/* We don't have 1 and 2 sized records [ 2^0 2^1 ] */
		out = 2;
	}
	
	return out-2;
}

/* Create a record at the end of the disk. This record will be sparse if
 * the filesystem supports it.
 * Precondition: there must be no other free blocks of this size.
 */
static int lu_create_empty_record(char cell_type, off_t* out)
{
	message_id junk = 0;
	off_t amount;
	off_t ind;
	off_t where = lseek(lu_keyword_fd, 0, SEEK_END);
	
	if (where == -1)
	{
		syslog(LOG_ERR, "seeking to eof keyword.flat: %s\n",
			strerror(errno));
		return -1;
	}
	
	amount = 1 << (cell_type + 2); /* n is actualy 2^(n+2) */
	amount *= sizeof(message_id); /* We need to fit message_ids */
	ind = where + amount - sizeof(message_id);
	
	if (lu_swrite(lu_keyword_fd, ind, &junk, sizeof(message_id), 
		"writing to extend file") != sizeof(message_id))
	{
		return -1;
	}
	
	/* Alright, we've got the space. - unwind with error1 now
	 * Now we just have to mark it as free.
	 */
	*out = where;
	return 0;
}

/** This function sticks a cell on the free list.
 *  It may be reclaimed by later allocations.
 */
static int lu_free_cell(off_t which, message_id msgs)
{
	message_id next_count;
	off_t prior;
	off_t amount;
	off_t where = lu_cell_type(msgs);
	
	amount = 1 << (lu_cell_type(msgs) + 2); /* n is actualy 2^(n+2) */
	amount *= sizeof(message_id); /* We need to fit message_ids */
	
	while (1)
	{
		next_count = 0;
		if (lu_sread(lu_keyword_fd, which + amount, &next_count, sizeof(message_id),
			"detecting fragment on free") == -1)
		{
			goto lu_free_cell_error0;
		}
		
		if (next_count != lu_kw_invalid) break;
		
		/* It's at least twice as big as it's holding */
		amount <<= 1;
		where++;
	}
	
	where *= sizeof(off_t);
	if (lu_sread(lu_keyword_fd, where, &prior, sizeof(off_t),
		"reading old free index") != sizeof(off_t))
	{
		goto lu_free_cell_error0;
	}
	
	if (lu_swrite(lu_keyword_fd, which, &prior, sizeof(off_t),
		"writing next free ptr") != sizeof(off_t))
	{
		goto lu_free_cell_error0;
	}
	
	if (lu_swrite(lu_keyword_fd, where, &which, sizeof(off_t),
		"reseting free head ptr") != sizeof(off_t))
	{
		goto lu_free_cell_error1;
	}
	
	return 0;

lu_free_cell_error1:
	if (lu_swrite(lu_keyword_fd, which, &msgs, sizeof(off_t),
		"LOST SPACE: restoring message counter") != sizeof(off_t))
	{
		goto lu_free_cell_error0;
	}
	
lu_free_cell_error0:
	return -1;
}

/* This function should be called whenever the size of a record is being
 * reduced. It will see if the new size does not indicate how large the
 * record's space really is. It will place all the fragment markers necessary
 * to keep the free function aware of the real size.
 */
int lu_push_fragment_markers(off_t ind, message_id old, message_id new)
{
	off_t		amount;
	char		old_type = lu_cell_type(old);
	char		new_type = lu_cell_type(new);
	
	amount = 1 << (new_type + 2);
	amount *= sizeof(message_id);
	while (new_type < old_type)
	{
		
		if (lu_swrite(lu_keyword_fd, ind + amount, &lu_kw_invalid, sizeof(message_id),
			"writing free space fragment marker") != sizeof(message_id))
		{
			return -1;
		}
		
		new_type++;
		amount <<= 1;
	}
	
	return 0;
}

/** Pop a record off the free list. Return 0 if there is none.
 */
static int lu_pop_free_list(char cell_type, off_t* out)
{
	off_t next;
	off_t where = cell_type;
	where *= sizeof(off_t);
	
	if (lu_sread(lu_keyword_fd, where, out, sizeof(off_t),
		"reading free record") != sizeof(off_t))
	{
		return -1;
	}
	
	if (*out == 0)
	{
		return 0;
	}
	
	if (lu_sread(lu_keyword_fd, *out, &next, sizeof(off_t),
		"reading next free record") != sizeof(off_t))
	{
		return -1;
	}
	
	if (lu_swrite(lu_keyword_fd, where, &next, sizeof(off_t),
		"moving next free to head") != sizeof(off_t))
	{
		return -1;
	}
	
	return 0;
}

/* Allocate a free block, possibly resusing the free list or creating a new
 * block.
 */
static int lu_allocate_cell(char cell_type, off_t* out)
{
	if (lu_pop_free_list(cell_type, out) != 0)
		return -1;
	
	if (*out != 0)
	{
		return 0;
	}
	
	if (lu_create_empty_record(cell_type, out) != 0)
		return -1;
	
	if (*out == 0)
	{
		syslog(LOG_ERR, "We created a free record, but we didn't get it\n");
		return -1;
	}
	
	return 0;
}

/** This method locates the current offset in the flatfile which houses
 *  the specified keyword. On success 0 is returned, else -1
 *  If this is a new keyword out == 0.
 */
static int lu_locate_keyword(const char* keyword, off_t* out)
{
	DBT	key;
	DBT	val;
	int	error;
	
	memset(&key, 0, sizeof(key));
	memset(&val, 0, sizeof(val));
	
	key.data = (char*)keyword;	/* It's const still honest */
	key.size = strlen(keyword);
	
	val.data = out;
	val.ulen = sizeof(off_t);
	val.flags = DB_DBT_USERMEM;
	
	error = lu_keyword_db->get(lu_keyword_db, 0, &key, &val, 0);
	if (error == DB_NOTFOUND)
	{
		*out = 0;
		return 0;
	}
	else if (error)
	{
		syslog(LOG_ERR, "Accessing keyword %s: %s\n", keyword, db_strerror(error));
		return -1;
	}
	
	return 0;
}

/** This method updates the offset at which a keyword resides.
 */
static int lu_reset_keyword(const char* keyword, off_t in)
{
	DBT	key;
	DBT	val;
	int	error;
	
	memset(&key, 0, sizeof(key));
	memset(&val, 0, sizeof(val));
	
	key.data = (char*)keyword;	/* It's const still honest */
	key.size = strlen(keyword);
	
	val.data = &in;
	val.size = sizeof(off_t);
	
	error = lu_keyword_db->put(lu_keyword_db, 0, &key, &val, 0);
	if (error)
	{
		syslog(LOG_ERR, "Writing keyword %s: %s\n", keyword, db_strerror(error));
		return -1;
	}
	
	return 0;
}

/** Push some more keyword records onto an entry.
 *  It is intended that this be called with several records at once that have
 *  been queued up. Hence we may end up jumping to several orders of magnitude
 *  larger storage size.
 */
static int lu_write_keyword_block(
	const char*	keyword, 
	message_id*	buf,
	message_id	count)
{
	message_id	records;
	off_t		where;
	off_t		spot;
	off_t		new_records;
	off_t		ind;
	
	if (lu_locate_keyword(keyword, &where) != 0)
		return -1;
	
	/* Is this an old record? */
	if (where == 0)
	{	/* Nope, hence no current size - make new record */
		if (lu_allocate_cell(lu_cell_type(count), &spot) != 0)
		{
			return -1;
		}
		
		records = 0;
		new_records = records + count;
		
		if (lu_swrite(lu_keyword_fd, spot, &records, sizeof(message_id),
			"preping space for new keyword") != sizeof(message_id))
		{
			if (lu_free_cell(spot, count) != 0)
				syslog(LOG_ERR, "Permanently lost storage\n");
			
			return -1;
		}
		
		if (lu_reset_keyword(keyword, spot) != 0)
		{
			if (lu_free_cell(spot, count) != 0)
				syslog(LOG_ERR, "Permanently lost storage\n");
			
			return -1;
		}
		
		where = spot;
	}
	else
	{	/* Yep, get the old size */
		if (lu_sread(lu_keyword_fd, where, &records, sizeof(message_id),
			"reading old message count") != sizeof(message_id))
		{
			return -1;
		}
		
		new_records = records + count;
		
		if (lu_cell_type(records) != lu_cell_type(new_records))
		{
			char buf[sizeof(message_id) * 2 + sizeof(off_t)];
			
			if (lu_allocate_cell(lu_cell_type(new_records), &spot) != 0)
			{
				return -1;
			}
			
			memcpy(&buf[0], &new_records, sizeof(message_id));
			memset(&buf[sizeof(message_id)], 0xFF, sizeof(message_id));
			memcpy(&buf[sizeof(message_id)*2], &where, sizeof(off_t));
			
			if (lu_swrite(lu_keyword_fd, spot, &buf[0], sizeof(buf),
				"backward reference") != sizeof(buf))
			{
				if (lu_free_cell(spot, new_records) != 0)
					syslog(LOG_ERR, "Permanently lost storage\n");
				
				return -1;
			}
			
			if (lu_reset_keyword(keyword, spot) != 0)
			{
				if (lu_free_cell(spot, new_records) != 0)
					syslog(LOG_ERR, "Permanently lost storage\n");
				
				return -1;
			}
			
			where = spot;
		}
	}
	
	ind = where + ((records + 1) * sizeof(off_t)); /* +1 to skip count */
	if (lu_swrite(lu_keyword_fd, where, buf, count * sizeof(message_id),
		"writing new message_id records") != count * sizeof(message_id))
	{
		return -1;
	}
	
	if (lu_swrite(lu_keyword_fd, where, &new_records, sizeof(message_id),
		"writing new message_id counter") != sizeof(message_id))
	{
		return -1;
	}
	
	return 0;
}

int lu_push_keyword(const char* keyword, message_id msg)
{
	/*!!! Buffering needs to be done here. */
	return lu_write_keyword_block(keyword, &msg, 1);
}

static int lu_flush_buffer(const char* keyword)
{
	/*!!! Buffering needs to be done here. */
	return 0;
}

int lu_pop_keyword(const char* keyword, message_id id)
{
	off_t		where;
	message_id	count;
	message_id	last;
	off_t		amount;
	
	if (lu_flush_buffer(keyword) != 0)
	{
		return -1;
	}
	
	if (lu_locate_keyword(keyword, &where) != 0)
	{
		return -1;
	}
	
	if (where == 0)
	{	/* can't pop a non-existant record */
		return -1;
	}
	
	if (lu_sread(lu_keyword_fd, where, &count, sizeof(message_id),
		"reading old message counter") != sizeof(message_id))
	{
		return -1;
	}
	
	if (count == 0)
	{	/* can't pop an empty record */
		return -1;
	}
	
	amount = count; /* Recall -1 for last, +1 to skip count */
	amount *= sizeof(message_id);
	if (lu_sread(lu_keyword_fd, where + amount, &last, sizeof(message_id),
		"reading last record") != sizeof(message_id))
	{	/* need to check last entry */
		return -1;
	}
	
	if (id != last)
	{	/* Not what we were asked to pop */
		return 0;
	}
	
	if (lu_push_fragment_markers(where, count, count - 1) != 0)
	{	/* can't mark this record as bigger than it seems */
		return -1;
	}
	
	count--;
	if (lu_swrite(lu_keyword_fd, where, &count, sizeof(message_id),
		"pushing popped message counter") != sizeof(message_id))
	{
		return -1;
	}
	
	return 0;
}

/* Ok, the direct record access methods.
 * These are used by the keyword search algorithm and also by the index
 * rendering alogirhtms (which need to count from the front).
 * 
 * A handle just stores the information necessary to locate data that is
 * requested by the caller. It is indexed from the front of the record
 * not the rear (as in keyword searches). It also deals with records
 * that are buffered, but not flushed.
 * 
 * Each record may also be partially fragmented. When a handle is opened, we
 * partially defragment the record. We won't completely defragment the
 * record or we won't meet the < 0.001s requirement. :-) So, we will do up
 * to a constant number of disk ops to defragment it. Then allow reads into
 * the fragmented buffer.
 */

/* What is the maximum number of records we'll move to defragment on open?
 * Default: 2Mb
 */
#define LU_DEFRAGMENT_MAXIMUM	524288

/* What is the block size we should use for disk I/O?
 * Default: 16k
 */
#define LU_BLOCK_SIZE 16384

struct Handle_T
{
	/* Backwards sorted (after gets smaller) */
	struct IndexReference
	{
		/* Records [after, records-after) are at where */
		
		message_id	after;
		message_id	records;
		off_t		where;
	} index[LU_TABLE_SIZE+1];
};

/* This routine will defragment at most one chunk of the handle. It will also
 * not put in more effort than LU_DEFRAGMENT_MAXIMUM.
 */
static int lu_defrag_record(Handle h)
{
#if 0
	/* Lets try doing nothing .. if it's fast searching maybe we will
	 * never defrag. Or at least lets see the impact later - when I can
	 * get real timings.
	 */
	 
	message_id amount;
	message_id frag_off;
	message_id buf[LU_BLOCK_SIZE / sizeof(message_id)];
	
	if (h->index[0].after == 0)
	{	/* Not fragmented - stop right here. */
		return 0;
	}
	
	if (h->index[0].after > LU_DEFRAGMENT_MAXIMUM)
	{
		amount = LU_DEFRAGMENT_MAXIMUM;
		frag_off = h->index[0].after - amount;
		
		if (frag_off <= 3)
		{	/* If you're going to be fragmented, you have to be
			 * at least 3 records fragmented. Just do it all.
			 */
			amount   = h->index[0].after;
			frag_off = h->index[1].after;
		}
	}
	else
	{
		amount   = h->index[0].after;
		frag_off = h->index[1].after;
	}
	
	/* Now, move [frag_off, frag_off+amount) into 0 from 1.
	 */
	
	/* Now, resize the records of ... */
#endif
	return 0;
}

Handle lu_open_handle(const char* keyword)
{
	Handle		out;
	off_t		scan;
	int		where;
	message_id	buf[2];
	
	if (lu_flush_buffer(keyword) != 0)
	{
		return 0;
	}
	
	if (lu_locate_keyword(keyword, &scan) != 0)
	{
		return 0;
	}
	
	if ((out = malloc(sizeof(struct Handle_T))) == 0)
	{
		return 0;
	}
	
	if (scan == 0)
	{
		out->index[0].where   = 0;
		out->index[0].after   = 0;
		out->index[0].records = 0;
		
		return out;
	}
	
	where = -1;
	do
	{
		where++;
		
		if (lu_sread(lu_keyword_fd, scan, 
			&buf[0], sizeof(message_id)*2,
			"reading record count") != sizeof(message_id)*2)
		{
			free(out);
			return 0;
		}
		
		out->index[where].where   = scan;
		out->index[where].records = buf[0];
		
		if (read(lu_keyword_fd, &scan, sizeof(off_t)) != sizeof(off_t))
		{
			free(out);
			return 0;
		}
	} while(buf[1] == lu_kw_invalid);
	
	out->index[where].after = 0;
	for (; where > 0; where--)
	{
		out->index[where-1].after = 
			out->index[where].after + out->index[where].records;
	}
	
	/* Don't really care if it fails */
	if (lu_defrag_record(out) != 0)
	{
		syslog(LOG_WARNING, "Defragmentation of %s failed\n",
			keyword);
	}
	
	return out;
}

message_id lu_handle_records(Handle h)
{
	return h->index[0].records;
}

int lu_handle_read(Handle h, message_id index, message_id* buf, message_id count)
{
	int		where;
	message_id	range;
	off_t		spot;
	
	if (index + count > lu_handle_records(h))
	{	/* Attempt to read out of bounds */
		return -1;
	}
	
	where = 0;
	while (h->index[where].after > index)
	{
		where++;
	}
	
	while (count)
	{
		if (index + count > h->index[where].records)
			range = h->index[where].records - index;
		else
			range = count;
		
		spot = h->index[where].where + ((index + 1) * sizeof(message_id));
		if (lu_sread(lu_keyword_fd, spot, buf, range*sizeof(message_id),
			"reading document ids") != range*sizeof(message_id))
		{
			return -1;
		}
		
		index += range;
		buf   += range;
		count -= range;
		where--;
	}
	
	return 0;
}

void lu_close_handle(Handle h)
{
	free(h);
}

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
	
	printf("Running test on indexing methods... (must wipe db to succeed)\n");
	
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
	
	return -1;
#endif
}

int lu_test_handle()
{
#ifndef DEBUG
	return 0;
#else
	return -1;
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
