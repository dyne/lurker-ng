/*  $Id: keyword.c,v 1.3 2002-01-24 23:11:54 terpstra Exp $
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

#define LU_BLOCK_AT_ONCE	10240

#include "keyword.h"
#include "io.h"

#include <string.h>
#include <db.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#else
#define syslog(x, y, ...)
#endif

int	lu_keyword_fd;
DB*	lu_keyword_db;

/* Ok, how does this work?
 *
 * So, first we have the db3 btree which turns keyword strings into their
 * flat file offsets.
 *
 * Then in the flat file we have lists of document ids for that keyword.
 * These lists MUST be in sorted order.
 *
 * Our defragmentation is simple: we don't.
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
 * The record count is *guaranteed* to be such that lu_cell_type will tell you
 * what the cell-type of it is.
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
 * How a search is executed is explained further down.
 */

/* We don't allow 2^0 or 2^1 sized blocks.
 * We do allow up to 2^2 ... 2^(bits in message_id) sized blocks
 */
#define TABLE_SIZE	(sizeof(message_id) * 8 - 1)

/* Helper method to do a complete write 
 */
static ssize_t lu_write_full(int fd, const void* buf, size_t count, const char* msg)
{
	ssize_t out;
	ssize_t did;
	
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
	
	out = 0;
	while (count)
	{
		did = read(fd, buf, count);
		
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
	
	off_t records[TABLE_SIZE];
	off_t off = lseek(lu_keyword_fd, 0, SEEK_END);
	
	if (off == -1)
	{
		perror("lseek'ing the keyword.flat");
		return -1;
	}
	
	if (off < sizeof(message_id) * TABLE_SIZE)
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
static int lu_create_empty_record(char cell_type)
{
	message_id junk = 0;
	off_t amount;
	off_t ind;
	off_t where = lseek(lu_keyword_fd, 0, SEEK_END);
	
	if (where == -1)
	{
		syslog(LOG_ERR, "seeking to eof keyword.flat: %s\n",
			strerror(errno));
		goto lu_create_empty_record_error0;
	}
	
	amount = 1 << (cell_type + 2); /* n is actualy 2^(n+2) */
	amount *= sizeof(message_id); /* We need to fit message_ids */
	ind = where + amount - sizeof(message_id);
	
	if (lu_swrite(lu_keyword_fd, ind, &junk, sizeof(message_id), 
		"writing to extend file") != sizeof(message_id))
	{
		goto lu_create_empty_record_error0;
	}
	
	/* Alright, we've got the space. - unwind with error1 now
	 * Now we just have to mark it as free.
	 */
	
	ind = cell_type;
	ind *= sizeof(off_t);
	if (lu_swrite(lu_keyword_fd, ind, &where, sizeof(off_t),
		"marking as free record") != sizeof(off_t))
	{
		goto lu_create_empty_record_error1;
	}
	
	/* Score! It all worked! */
	return 0;
	
lu_create_empty_record_error1:
	if (ftruncate(lu_keyword_fd, where) != 0)
		syslog(LOG_ERR, "could not release claimed space: %llu\n",
			amount);
	
lu_create_empty_record_error0:
	return -1;
}

/** This function sticks a cell on the free list.
 *  It may be reclaimed by later allocations.
 */
static int lu_free_cell(off_t which, message_id msgs)
{
	off_t prior;
	off_t where = lu_cell_type(msgs);
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
	
	if (lu_create_empty_record(cell_type) != 0)
		return -1;
	
	if (lu_pop_free_list(cell_type, out) != 0)
		return -1;
	
	if (*out == 0)
	{
		syslog(LOG_ERR, "We created a free record, but it wasn't in keyword.flat\n");
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
	val.size = sizeof(off_t);
	
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
	
	error = lu_keyword_db->get(lu_keyword_db, 0, &key, &val, 0);
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
	int		need_resize;
	
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
				if (lu_free_cell(spot, count) != 0)
					syslog(LOG_ERR, "Permanently lost storage\n");
				
				return -1;
			}
			
			where = spot;
		}
	}
	
	ind = where + ((records + 1) * sizeof(off_t));
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

