/*  $Id: flatfile.c,v 1.13 2002-06-21 18:19:03 terpstra Exp $
 *  
 *  flatfile.c - Knows how to manage the keyword flatfile database
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
#include "io.h"

#include "config.h"
#include "flatfile.h"
#include "wbuffer.h"

#include <string.h>
#include <stdlib.h>

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

/*------------------------------------------------ Constant parameters */

/* What is the maximum number of records we'll move to defragment on open?
 * Default: 2Mb
 */
#define LU_DEFRAGMENT_MAXIMUM	524288

/* What is the block size we should use for disk I/O?
 * Default: 16k
 */
#define LU_BLOCK_SIZE 16384

/* We don't allow 2^0 or 2^1 sized blocks.
 * We do allow up to 2^2 ... 2^(bits in message_id) sized blocks
 */
#define LU_TABLE_SIZE	(sizeof(message_id) * 8 - 1)

/*------------------------------------------------ Private types */

struct My_Flatfile_Handle_T
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

/*------------------------------------------------ Private global vars */

static int my_flatfile_fd;
static DB* my_flatfile_db;

/*------------------------------------------------ Private helper methods */

/* Helper method to do a complete write 
 */
static ssize_t my_flatfile_write_full(
	int fd, 
	const void* buf, 
	size_t count, 
	const char* msg)
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
static ssize_t my_flatfile_read_full(
	int fd, 
	void* buf, 
	size_t count, 
	const char* msg)
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
static ssize_t my_flatfile_swrite(
	int fd, 
	off_t off, 
	const void* buf, 
	size_t count, 
	const char* msg)
{
	if (lseek(fd, off, SEEK_SET) != off)
	{
		syslog(LOG_ERR, _("%s seeking keyword.flat (%lld): %s\n"),
			gettext(msg), off, strerror(errno));
		return -1;
	}
	
	return my_flatfile_write_full(fd, buf, count, msg);
}

/* Helper method which does a seek and then a write
 */
static ssize_t my_flatfile_sread(
	int fd, 
	off_t off, 
	void* buf, 
	size_t count, 
	const char* msg)
{
	if (lseek(fd, off, SEEK_SET) != off)
	{
		syslog(LOG_ERR, _("%s seeking keyword.flat (%lld): %s\n"),
			gettext(msg), off, strerror(errno));
		return -1;
	}
	
	return my_flatfile_read_full(fd, buf, count, msg);
}

/* What is the free-block index for this size.
 */
static char my_flatfile_cell_type(
	message_id hits)
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
static int my_flatfile_create_empty_record(
	char cell_type, 
	off_t* out)
{
	message_id junk = 0;
	off_t amount;
	off_t ind;
	off_t where = lseek(my_flatfile_fd, 0, SEEK_END);
	
	if (where == -1)
	{
		syslog(LOG_ERR, _("seeking to eof keyword.flat: %s\n"),
			strerror(errno));
		return -1;
	}
	
	amount = 1 << (cell_type + 2); /* n is actualy 2^(n+2) */
	amount *= sizeof(message_id); /* We need to fit message_ids */
	ind = where + amount - sizeof(message_id);
	
	if (my_flatfile_swrite(my_flatfile_fd, ind, &junk, sizeof(message_id), 
		N_("writing to extend file")) != sizeof(message_id))
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
static int my_flatfile_free_cell(
	off_t which, 
	message_id msgs)
{
	message_id next_count;
	off_t prior;
	off_t amount;
	off_t where = my_flatfile_cell_type(msgs);
	
	amount = 1 << (my_flatfile_cell_type(msgs) + 2); /* n is actualy 2^(n+2) */
	amount *= sizeof(message_id); /* We need to fit message_ids */
	
	while (1)
	{
		next_count = 0;
		if (my_flatfile_sread(my_flatfile_fd, which + amount, &next_count, sizeof(message_id),
			N_("detecting fragment on free")) == -1)
		{
			goto my_flatfile_free_cell_error0;
		}
		
		if (next_count != lu_common_minvalid) break;
		
		/* It's at least twice as big as it's holding */
		amount <<= 1;
		where++;
	}
	
	where *= sizeof(off_t);
	if (my_flatfile_sread(my_flatfile_fd, where, &prior, sizeof(off_t),
		N_("reading old free index")) != sizeof(off_t))
	{
		goto my_flatfile_free_cell_error0;
	}
	
	if (my_flatfile_swrite(my_flatfile_fd, which, &prior, sizeof(off_t),
		N_("writing next free ptr")) != sizeof(off_t))
	{
		goto my_flatfile_free_cell_error0;
	}
	
	if (my_flatfile_swrite(my_flatfile_fd, where, &which, sizeof(off_t),
		N_("reseting free head ptr")) != sizeof(off_t))
	{
		goto my_flatfile_free_cell_error1;
	}
	
	return 0;

my_flatfile_free_cell_error1:
	if (my_flatfile_swrite(my_flatfile_fd, which, &msgs, sizeof(off_t),
		N_("LOST SPACE: restoring message counter")) != sizeof(off_t))
	{
		goto my_flatfile_free_cell_error0;
	}
	
my_flatfile_free_cell_error0:
	return -1;
}

/* This function should be called whenever the size of a record is being
 * reduced. It will see if the new size does not indicate how large the
 * record's space really is. It will place all the fragment markers necessary
 * to keep the free function aware of the real size.
 */
static int my_flatfile_push_fragment_markers(
	off_t ind, 
	message_id old, 
	message_id new)
{
	off_t		amount;
	char		old_type = my_flatfile_cell_type(old);
	char		new_type = my_flatfile_cell_type(new);
	
	amount = 1 << (new_type + 2);
	amount *= sizeof(message_id);
	while (new_type < old_type)
	{
		
		if (my_flatfile_swrite(my_flatfile_fd, ind + amount, &lu_common_minvalid, sizeof(message_id),
			N_("writing free space fragment marker")) != sizeof(message_id))
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
static int my_flatfile_pop_free_list(
	char cell_type, 
	off_t* out)
{
	off_t next;
	off_t where = cell_type;
	where *= sizeof(off_t);
	
	if (my_flatfile_sread(my_flatfile_fd, where, out, sizeof(off_t),
		N_("reading free record")) != sizeof(off_t))
	{
		return -1;
	}
	
	if (*out == 0)
	{
		return 0;
	}
	
	if (my_flatfile_sread(my_flatfile_fd, *out, &next, sizeof(off_t),
		N_("reading next free record")) != sizeof(off_t))
	{
		return -1;
	}
	
	if (my_flatfile_swrite(my_flatfile_fd, where, &next, sizeof(off_t),
		N_("moving next free to head")) != sizeof(off_t))
	{
		return -1;
	}
	
	return 0;
}

/* Allocate a free block, possibly resusing the free list or creating a new
 * block.
 */
static int my_flatfile_allocate_cell(
	char cell_type, 
	off_t* out)
{
	if (my_flatfile_pop_free_list(cell_type, out) != 0)
		return -1;
	
	if (*out != 0)
	{
		return 0;
	}
	
	if (my_flatfile_create_empty_record(cell_type, out) != 0)
		return -1;
	
	if (*out == 0)
	{
		syslog(LOG_ERR, 
			_("We created a free record, but we didn't get it\n"));
		return -1;
	}
	
	return 0;
}

/** This method locates the current offset in the flatfile which houses
 *  the specified keyword. On success 0 is returned, else -1
 *  If this is a new keyword out == 0.
 */
static int my_flatfile_locate_keyword(
	const char* keyword, 
	off_t* out)
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
	
	error = my_flatfile_db->get(my_flatfile_db, 0, &key, &val, 0);
	if (error == DB_NOTFOUND)
	{
		*out = 0;
		return 0;
	}
	else if (error)
	{
		syslog(LOG_ERR, 
			_("Accessing keyword %s: %s\n"), 
			keyword, db_strerror(error));
		return -1;
	}
	
	return 0;
}

/** This method updates the offset at which a keyword resides.
 */
static int my_flatfile_reset_keyword(
	const char* keyword, 
	off_t in)
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
	
	error = my_flatfile_db->put(my_flatfile_db, 0, &key, &val, 0);
	if (error)
	{
		syslog(LOG_ERR, 
			_("Writing keyword %s: %s\n"), 
			keyword, db_strerror(error));
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

/* This routine will defragment at most one chunk of the handle. It will also
 * not put in more effort than LU_DEFRAGMENT_MAXIMUM.
 */
static int my_flatfile_defrag_record(Lu_Flatfile_Handle h)
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

/*------------------------------------------------ Public append methods */

/** Push some more keyword records onto an entry.
 *  It is intended that this be called with several records at once that have
 *  been queued up. Hence we may end up jumping to several orders of magnitude
 *  larger storage size.
 */
message_id lu_flatfile_write_keyword_block(
	const char*	keyword, 
	message_id*	buf,
	message_id	count)
{
	message_id	records;
	off_t		where;
	off_t		spot;
	off_t		new_records;
	off_t		ind;
	
	if (my_flatfile_locate_keyword(keyword, &where) != 0)
		return lu_common_minvalid;
	
	/* Is this an old record? */
	if (where == 0)
	{	/* Nope, hence no current size - make new record */
		if (my_flatfile_allocate_cell(my_flatfile_cell_type(count), &spot) != 0)
		{
			return lu_common_minvalid;
		}
		
		records = 0;
		new_records = records + count;
		
		if (my_flatfile_swrite(my_flatfile_fd, spot, &records, sizeof(message_id),
			N_("preping space for new keyword")) != sizeof(message_id))
		{
			if (my_flatfile_free_cell(spot, count) != 0)
				syslog(LOG_ERR, _("Permanently lost storage\n"));
			
			return lu_common_minvalid;
		}
		
		if (my_flatfile_reset_keyword(keyword, spot) != 0)
		{
			if (my_flatfile_free_cell(spot, count) != 0)
				syslog(LOG_ERR, _("Permanently lost storage\n"));
			
			return lu_common_minvalid;
		}
		
		where = spot;
	}
	else
	{	/* Yep, get the old size */
		if (my_flatfile_sread(my_flatfile_fd, where, &records, sizeof(message_id),
			N_("reading old message count")) != sizeof(message_id))
		{
			return lu_common_minvalid;
		}
		
		new_records = records + count;
		
		if (my_flatfile_cell_type(records) != my_flatfile_cell_type(new_records))
		{
			char buf[sizeof(message_id) * 2 + sizeof(off_t)];
			
			if (my_flatfile_allocate_cell(
				my_flatfile_cell_type(new_records), &spot) != 0)
			{
				return lu_common_minvalid;
			}
			
			/* Record that this cell is large */
			if (my_flatfile_push_fragment_markers(spot, new_records, records) != 0)
			{
				if (my_flatfile_free_cell(spot, new_records) != 0)
					syslog(LOG_ERR, _("Permanently lost storage\n"));
				
				return lu_common_minvalid;
			}
			
			memcpy(&buf[0], &records, sizeof(message_id));
			memset(&buf[sizeof(message_id)], 0xFF, sizeof(message_id));
			memcpy(&buf[sizeof(message_id)*2], &where, sizeof(off_t));
			
			if (my_flatfile_swrite(my_flatfile_fd, spot, &buf[0], sizeof(buf),
				N_("backward reference")) != sizeof(buf))
			{
				if (my_flatfile_free_cell(spot, new_records) != 0)
					syslog(LOG_ERR, _("Permanently lost storage\n"));
				
				return lu_common_minvalid;
			}
			
			if (my_flatfile_reset_keyword(keyword, spot) != 0)
			{
				if (my_flatfile_free_cell(spot, new_records) != 0)
					syslog(LOG_ERR, _("Permanently lost storage\n"));
				
				return lu_common_minvalid;
			}
			
			where = spot;
		}
	}
	
	ind = records + 1; /* +1 to skip count */
	ind *= sizeof(message_id);
	ind += where;
	
	if (my_flatfile_swrite(my_flatfile_fd, ind, buf, count * sizeof(message_id),
		N_("writing new message_id records")) != count * sizeof(message_id))
	{
		return lu_common_minvalid;
	}
	
	if (my_flatfile_swrite(my_flatfile_fd, where, &new_records, sizeof(message_id),
		N_("writing new message_id counter")) != sizeof(message_id))
	{
		return lu_common_minvalid;
	}
	
	return new_records;
}

int lu_flatfile_pop_keyword(
	const char* keyword, 
	message_id id)
{
	off_t		where;
	message_id	count;
	message_id	last;
	off_t		amount;
	
	if (lu_wbuffer_flush(keyword) != 0)
	{
		return 0;
	}
	
	if (my_flatfile_locate_keyword(keyword, &where) != 0)
	{
		return -1;
	}
	
	if (where == 0)
	{	/* can't pop a non-existant record */
		return -1;
	}
	
	if (my_flatfile_sread(my_flatfile_fd, where, &count, sizeof(message_id),
		N_("reading old message counter")) != sizeof(message_id))
	{
		return -1;
	}
	
	if (count == 0)
	{	/* can't pop an empty record */
		return -1;
	}
	
	amount = count; /* Recall -1 for last, +1 to skip count */
	amount *= sizeof(message_id);
	if (my_flatfile_sread(my_flatfile_fd, where + amount, &last, sizeof(message_id),
		N_("reading last record")) != sizeof(message_id))
	{	/* need to check last entry */
		return -1;
	}
	
	if (id != last)
	{	/* Not what we were asked to pop */
		return 0;
	}
	
	if (my_flatfile_push_fragment_markers(where, count, count - 1) != 0)
	{	/* can't mark this record as bigger than it seems */
		return -1;
	}
	
	count--;
	if (my_flatfile_swrite(my_flatfile_fd, where, &count, sizeof(message_id),
		N_("pushing popped message counter")) != sizeof(message_id))
	{
		return -1;
	}
	
	return 0;
}

/*------------------------------------------------ Public access methods */

Lu_Flatfile_Handle lu_flatfile_open_handle(
	const char* keyword)
{
	Lu_Flatfile_Handle		out;
	off_t		scan;
	int		where;
	message_id	buf[2];
	
	if (lu_wbuffer_flush(keyword) != 0)
	{
		return 0;
	}
	
	if (my_flatfile_locate_keyword(keyword, &scan) != 0)
	{
		return 0;
	}
	
	/*!!! It seems silly that we malloc here and nowhere else! */
	if ((out = malloc(sizeof(struct My_Flatfile_Handle_T))) == 0)
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
		
		if (my_flatfile_sread(my_flatfile_fd, scan, 
			&buf[0], sizeof(buf),
			N_("reading record count")) != sizeof(buf))
		{
			free(out);
			return 0;
		}
		
		out->index[where].where   = scan;
		out->index[where].records = buf[0];
		
		if (read(my_flatfile_fd, &scan, sizeof(off_t)) != sizeof(off_t))
		{
			free(out);
			return 0;
		}
	} while(buf[1] == lu_common_minvalid);
	
	out->index[where].after = 0;
	for (; where > 0; where--)
	{
		out->index[where-1].after = out->index[where].records;
	}
	
	/* Don't really care if it fails */
	if (my_flatfile_defrag_record(out) != 0)
	{
		syslog(LOG_WARNING, _("Defragmentation of %s failed\n"),
			keyword);
	}
	
	return out;
}

message_id lu_flatfile_handle_records(
	Lu_Flatfile_Handle h)
{
	return h->index[0].records;
}

message_id lu_flatfile_records(
	const char* keyword)
{
	message_id	out;
	off_t		scan;
	
	/*!!! Don't flush; if cached get the answer FROM cache. */
	
	if (lu_wbuffer_flush(keyword) != 0)
	{
		return lu_common_minvalid;
	}
	
	if (my_flatfile_locate_keyword(keyword, &scan) != 0)
	{
		return lu_common_minvalid;
	}
	
	if (my_flatfile_sread(my_flatfile_fd, scan, 
		&out, sizeof(out),
		N_("quick reading record count")) != sizeof(out))
	{
		return lu_common_minvalid;
	}
	
	return out;
}

int lu_flatfile_handle_read(
	Lu_Flatfile_Handle h, 
	message_id index, 
	message_id* buf, 
	message_id count)
{
	int		where;
	message_id	range;
	off_t		spot;
	
	if (index + count > lu_flatfile_handle_records(h))
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
		if (my_flatfile_sread(my_flatfile_fd, spot, buf, range*sizeof(message_id),
			N_("reading document ids")) != range*sizeof(message_id))
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

void lu_flatfile_close_handle(
	Lu_Flatfile_Handle h)
{
	free(h);
}

/*------------------------------------------------ Public component methods */

int lu_flatfile_init()
{
	/* noop */
	return 0;
}

int lu_flatfile_open()
{
	int error;
	off_t records[LU_TABLE_SIZE];
	off_t off = lseek(my_flatfile_fd, 0, SEEK_END);
	
	my_flatfile_fd = open("keyword.flat", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (my_flatfile_fd == -1)
	{
		perror("keyword.flat");
		return -1;
	}
	
	if ((error = db_create(&my_flatfile_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, _("Creating a db3 database: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_flatfile_db->open(my_flatfile_db, "keyword.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, _("Opening db3 database: keyword.btree: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	/* We need to be able to handle having a keyword which is in
	 * every message_id. There are 8*sizeof(message_id) bits.
	 * Supposing we allow for 0 to bits or bits+1 records.
	 */
	
	off = lseek(my_flatfile_fd, 0, SEEK_END);
	if (off == -1)
	{
		perror(_("lseek'ing the keyword.flat"));
		return -1;
	}
	
	if (off < sizeof(message_id) * LU_TABLE_SIZE)
	{	/* It's not got the free tables */
		if (off != 0)
		{	/* Wtf there was stuff in it?! - it's not new */
			fputs(_("keyword.flat is corrupt\n"), stderr);
			return -1;
		}
		
		memset(&records[0], 0, sizeof(records));
		if (write(my_flatfile_fd, &records[0], sizeof(records)) 
			!= sizeof(records))
		{
			perror(_("initializing keyword.flat"));
			return -1;
		}
	}
	
	return 0;
}

int lu_flatfile_sync()
{
	int error;
	int fail = 0;
	
	if ((error = my_flatfile_db->sync(my_flatfile_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Syncing db3 database: keyword.btree: %s\n"),
			db_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_flatfile_close()
{
	int error;
	int fail = 0;
	
	if ((error = my_flatfile_db->close(my_flatfile_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Closing db3 database: keyword.btree: %s\n"),
			db_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_flatfile_quit()
{
	return 0;
}
