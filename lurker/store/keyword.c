/*  $Id: keyword.c,v 1.2 2002-01-23 07:33:12 terpstra Exp $
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
 * How a search is executed is explained further down.
 */

/* We don't allow 2^0 or 2^1 sized blocks.
 * We do allow up to 2^2 ... 2^(bits in message_id) sized blocks
 */
#define TABLE_SIZE	(sizeof(message_id) * 8 - 1)

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
static char lu_celltype(message_id hits)
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
 * the filesystem supports it. However, the function makes sure that
 * promise records have been allocated by the OS.
 * Precondition: there must be no other free blocks of this size.
 */
static int lu_create_empty_record(char cell_type, message_id promise)
{
	message_id buf[1024];
	
	ssize_t did;
	off_t amount;
	off_t where = lseek(lu_keyword_fd, 0, SEEK_END);
	
	if (where == -1)
	{
		syslog(LOG_ERR, "seeking to eof keyword.flat: %s\n",
			strerror(errno));
		return -1;
	}
	
	amount = 1 << (cell_type + 2); /* n is actualy 2^(n+2) */
	/* assert (promise <= amount); */
	
	amount *= sizeof(message_id); /* We need to fit message_ids */
	
	/* The only way to be sure that we have these records locked down
	 * is to write stuff to the disk there.
	 */
	memset(&buf[0], 0, sizeof(buf));
	while (promise)
	{
		did = write(lu_keyword_fd, &buf[0], sizeof(buf));
		if (did <= 0)
		{	/* No space available - not a critical error */
			syslog(LOG_WARNING, "out of free disk space\n");
			
			if (ftruncate(lu_keyword_fd, where) != 0)
				syslog(LOG_ERR, "could not release claimed space: %llu\n",
					amount);
			
			return -1;
		}
		
		promise -= did / sizeof(message_id);
	}
	
	if (lseek(lu_keyword_fd, where + amount - sizeof(message_id), SEEK_SET)
		!= where + amount - sizeof(message_id))
	{
		syslog(LOG_ERR, "failed to seek into the ether keyword.flat: %s\n",
			strerror(errno));
		if (ftruncate(lu_keyword_fd, where) != 0)
			syslog(LOG_ERR, "could not release claimed space: %llu\n",
				amount);
		return -1;
	}
	
	if (write(lu_keyword_fd, &buf[0], sizeof(message_id)) != sizeof(message_id))
	{	/* No space available - not a critical error */
		syslog(LOG_WARNING, "out of free disk space\n");
		if (ftruncate(lu_keyword_fd, where) != 0)
			syslog(LOG_ERR, "could not release claimed space: %llu\n",
				amount);
		return -1;
	}
	
	/* Alright, we've got the space, made sure the promised area is
	 * available, and resized the file with sparse data in between.
	 * Now we just have to mark it as free.
	 */
	
	if (lseek(lu_keyword_fd, cell_type * sizeof(off_t), SEEK_SET) 
		!= cell_type * sizeof(off_t))
	{
		syslog(LOG_ERR, "seeking within bounds of keyword.flat: %s\n",
			strerror(errno));
		
		if (ftruncate(lu_keyword_fd, where) != 0)
			syslog(LOG_ERR, "could not release claimed space: %llu\n",
				amount);
		
		return -1;
	}
	
	if (write(lu_keyword_fd, &where, sizeof(off_t)) != sizeof(off_t))
	{
		syslog(LOG_ERR, "writing record as free in keyword.flat: %s\n",
			strerror(errno));
		
		if (ftruncate(lu_keyword_fd, where) != 0)
			syslog(LOG_ERR, "could not release claimed space: %llu\n",
				amount);
		
		return -1;
	}
	
	/* Score! It all worked! */
	
	return 0;
}

/** This method locates the current offset in the flatfile which houses
 *  the specified keyword. If this is a new keyword, -1 is returned. If there
 *  was an error -2 is returned and the caller should abort.
 */
static off_t lu_locate_keyword(const char* keyword)
{
	DBT	key;
	DBT	val;
	off_t	out;
	int	error;
	
	memset(&key, 0, sizeof(key));
	memset(&val, 0, sizeof(val));
	
	key.data = (char*)keyword;	/* It's const still honest */
	key.size = strlen(keyword);
	
	val.data = &out;
	val.size = sizeof(out);
	
	error = lu_keyword_db->get(lu_keyword_db, 0, &key, &val, 0);
	if (error == DB_NOTFOUND)
	{
		return -1;
	}
	else if (error)
	{
		syslog(LOG_ERR, "Accessing keyword %s: %s\n", keyword, db_strerror(error));
		return -1;
	}
	
	return out;
}

static int lu_write_keyword_block(
	const char*	keyword, 
	message_id*	buf,
	size_t		records)
{
	return -1;
}

int lu_push_keyword(const char* keyword, message_id msg)
{
	/*!!! Buffering needs to be done here. */
	return lu_write_keyword_block(keyword, &msg, 1);
}

