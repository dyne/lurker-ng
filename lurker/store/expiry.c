/*  $Id: expiry.c,v 1.1 2002-05-03 05:21:27 terpstra Exp $
 *  
 *  expiry.c - Record when pages should be destroyed
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
#include "prefix.h"

#include "config.h"
#include "expiry.h"

#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

/*------------------------------------------------ Constant parameters */

/*------------------------------------------------ Private types */

typedef struct My_Expiry_Heap_T
{
	time_t	expiry;		/* expiry time */
	long	size;		/* file size */
	
	/* Ptrs use 0 for no record */
	lu_word	next_list;	/* next record in list */
	lu_word	prev_list;	/* previous list element */
	
	lu_word	next_time;	/* next created entry */
	lu_word prev_time;	/* prev created entry */
	
	lu_word	list;		/* Which list this is in - 0xFFFF for none */
	lu_word id;
} My_Expiry_Heap;

typedef struct My_Expiry_List_T
{
	lu_word	list; /* for the 0th entry, this is 'head' */
	lu_word	tail;
} My_Expiry_List;

static int	my_expiry_heap_fd	= -1;
static int	my_expiry_list_fd	= -1;

static DB*	my_expiry_names_db	= 0;

static long	my_expiry_files_used	= 0;
static long	my_expiry_space_used	= 0;

static long my_expiry_heaps = 0;
My_Expiry_Heap*	my_expiry_heap = 0;	/* Stored as -1 of mmap */
My_Expiry_List*	my_expiry_list = 0;

#define PARENT(i)	(i >> 1)
#define LEFT(i)		(i << 1)
#define RIGHT(i)	((i << 1) | 1)

inline void my_expiry_update_pos(lu_word x)
{
	lu_word nl, pl, nt, pt, ls;
	
	if ((pl = my_expiry_heap[x].prev_list) != 0)
		my_expiry_heap[pl].next_list = x;
	if ((nl = my_expiry_heap[x].next_list) != 0)
		my_expiry_heap[nl].prev_list = x;
	else if ((ls = my_expiry_heap[x].list) != LU_EXPIRY_NO_LIST)
	{
		lu_word ch = lu_config_find_list(ls)->cache_head;
		my_expiry_list[ch].tail = x;
	}
	
	if ((pt = my_expiry_heap[x].prev_time) != 0)
		my_expiry_heap[pt].next_time = x;
	else	my_expiry_list[0].list = x;
	if ((nt = my_expiry_heap[x].next_time) != 0)
		my_expiry_heap[nt].prev_time = x;
	else	my_expiry_list[0].tail = x;
}

static void my_expiry_swap_pos(lu_word a, lu_word b)
{
	My_Expiry_Heap tmp;
	
	memcpy(&tmp,               &my_expiry_heap[a], sizeof(My_Expiry_Heap));
	memcpy(&my_expiry_heap[a], &my_expiry_heap[b], sizeof(My_Expiry_Heap));
	memcpy(&my_expiry_heap[b], &tmp,               sizeof(My_Expiry_Heap));
	
	my_expiry_update_pos(a);
	my_expiry_update_pos(b);
}

static int my_expiry_pop(lu_word x)
{
	lu_word pl = my_expiry_heap[x].prev_list;
	lu_word nl = my_expiry_heap[x].next_list;
	lu_word pt = my_expiry_heap[x].prev_time;
	lu_word nt = my_expiry_heap[x].next_time;
	lu_word ls, smallest, left, right;
	int error;
	DBT  key, val;
	char file[400];
	
	assert(x <= my_expiry_heaps);
	
	/* Actually delete the file */
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	key.data = &my_expiry_heap[x].id;
	key.size = sizeof(lu_word);
	val.data = &file[0];
	val.ulen = sizeof(file);
	val.flags = DB_DBT_USERMEM;
	
	error = my_expiry_names_db->get(
		my_expiry_names_db, 0, &key, &val, 0);
	if (error != 0)
	{
		syslog(LOG_ERR, "Finding a record to delete for %d: %s\n",
			my_expiry_heap[x].id,
			db_strerror(error));
		return -1;
	}
	error = my_expiry_names_db->del(
		my_expiry_names_db, 0, &key, 0);
	if (error != 0)
	{
		syslog(LOG_ERR, "Deleting a cache record for %s: %s\n",
			&file[0],
			strerror(errno));
		return -1;
	}
	/* Ignore the case that it's already been killed */
	if (unlink(&file[0]) && errno != ENOENT)
	{
		syslog(LOG_ERR, "Deleting file '%s' failed: %s\n",
			&file[0],
			strerror(errno));
	}
	
	/* Decrease the space trackers */
	my_expiry_files_used--;
	my_expiry_space_used -= my_expiry_heap[x].size;
	
	/* Pull it out of the list of things in watching a mailinglist */
	if (pl)	my_expiry_heap[pl].next_list = nl;
	if (nl)	my_expiry_heap[nl].prev_list = pl;
	else if ((ls = my_expiry_heap[x].list) != LU_EXPIRY_NO_LIST)
	{
		lu_word ch = lu_config_find_list(ls)->cache_head;
		my_expiry_list[ch].tail = pl;
	}
	
	/* Pull it out of the list deletion list */
	if (pt)	my_expiry_heap[pt].next_time = nt;
	else	my_expiry_list[0] .list      = nt;
	if (nt)	my_expiry_heap[nt].prev_time = pt;
	else	my_expiry_list[0] .tail      = pt;
	
	
	/* Maintain the heap property */
	if (x != my_expiry_heaps)
	{
		memcpy(&my_expiry_heap[x], &my_expiry_heap[my_expiry_heaps], sizeof(My_Expiry_Heap));
		my_expiry_update_pos(x);
	}
	
	my_expiry_heaps--;
	if (ftruncate(my_expiry_heap_fd, 
		sizeof(My_Expiry_Heap) * my_expiry_heaps) != 0)
	{
		syslog(LOG_ERR, "Unable to reduce disk space cache heap: %s\n",
			strerror(errno));
	}
	
	/* It may be obscure, but this will work even if x >= my_expiry_heaps
	 * because it has no children and will hit the break.
	 */
	while (1)
	{
		left  =  LEFT(x);
		right = RIGHT(x);
		
		if (left <= my_expiry_heaps && 
		    my_expiry_heap[left].expiry < my_expiry_heap[x].expiry)
			smallest = left;
		else	smallest = x;
		
		if (right <= my_expiry_heaps &&
		    my_expiry_heap[right].expiry < my_expiry_heap[smallest].expiry)
			smallest = right;
		
		if (smallest == x) break;
		
		my_expiry_swap_pos(smallest, x);
		x = smallest;
	}
	
	return 0;
}

int lu_expiry_record_file(
	const char*	path,
	long		size,
	time_t		ttl,
	lu_word		list_watch)
{
	lu_word old_time_tail;
	lu_word what_list;
	lu_word old_list_tail;
	lu_word where;
	lu_word id;
	time_t  key;
	DBT	dbkey;
	DBT	dbval;
	int error;
	
	if (strlen(path) >= 400)
		return 1;
	
	if (size >= lu_config_cache_cutoff)
		return 1;
	
	while (my_expiry_files_used >= lu_config_cache_files ||
	       my_expiry_space_used + size >= lu_config_cache_size)
	{	/* As long as we are over-weight, reduce the oldest */
		if (my_expiry_pop(my_expiry_list[0].list) != 0)
			return 1;	/* something went wrong - stop cache */
	}
	
	memset(&dbkey, 0, sizeof(DBT));
	memset(&dbval, 0, sizeof(DBT));
	
	dbkey.data = &id;
	dbkey.size = sizeof(lu_word);
	dbval.data = (void*)path;
	dbval.size = strlen(path) + 1;
	
	/* Find a random id to use for the DB key */
	while (1)
	{
		id = random();
		error = my_expiry_names_db->put(
			my_expiry_names_db, 0, &dbkey, &dbval, DB_NOOVERWRITE);
		if (error == 0) break;
		if (error != DB_KEYEXIST)
		{
			syslog(LOG_ERR, "Failed to write filename '%s' to cache db: %s\n",
				path, db_strerror(error));
			return 1;
		}
	}
	
	/* Push the record onto the end */
	my_expiry_heaps++;
	/* Grow the file */
	lseek(my_expiry_heap_fd, my_expiry_heaps * sizeof(My_Expiry_Heap) - 1, SEEK_SET);
	write(my_expiry_heap_fd, &size, 1);
	
	/* Preserve the heap propery */
	where = my_expiry_heaps;
	key = time(0) + ttl;
	while (where > 1 && 
	       key < my_expiry_heap[PARENT(where)].expiry)
	{
		memcpy(&my_expiry_heap[where], 
		       &my_expiry_heap[PARENT(where)],
		       sizeof(My_Expiry_Heap));
		my_expiry_update_pos(where);
		where = PARENT(where);
	}
	
	/* where is now the location of our insertion */
	
	my_expiry_heap[where].expiry = key;
	my_expiry_heap[where].size   = size;
	
	my_expiry_heap[where].list = list_watch;
	my_expiry_heap[where].id = id;
	
	/* Stick it on the 'order-of-creation' list */
	old_time_tail = my_expiry_list[0].tail;
	my_expiry_heap[where].next_time = 0;
	my_expiry_heap[where].prev_time = old_time_tail;
	my_expiry_list[0].tail = where;
	if (old_time_tail)
		my_expiry_heap[old_time_tail].next_time = where;
	else	my_expiry_list[0].list                  = where;
	
	/* Stick it on the list for it's mailing list */
	my_expiry_heap[where].next_list = 0;
	my_expiry_heap[where].prev_list = 0;
	if (list_watch != LU_EXPIRY_NO_LIST)
	{
		what_list = lu_config_find_list(list_watch)->cache_head;
		old_list_tail = my_expiry_list[what_list].tail;
		my_expiry_list[what_list].tail = where;
		my_expiry_heap[where].prev_list = old_list_tail;
		if (old_list_tail)
			my_expiry_heap[old_list_tail].next_list = where;
	}
	
	return 0;
}

int lu_expiry_notice_import(
	lu_word	list)
{
	lu_word what_list = lu_config_find_list(list)->cache_head;
	
	while (my_expiry_list[what_list].tail)
	{
		if (my_expiry_pop(my_expiry_list[what_list].tail) != 0)
			return -1;
	}
	
	return 0;
}

/*------------------------------------------------ Public component methods */

int lu_expiry_init()
{
	return 0;
}

int lu_expiry_open()
{
	int	error;
	
	my_expiry_heap_fd = open("expiry.heap", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (my_expiry_heap_fd == -1)
	{
		perror("expiry.heap");
		return -1;
	}
	
	my_expiry_list_fd = open("expiry.list", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (my_expiry_list_fd == -1)
	{
		perror("expiry.list");
		return -1;
	}
	
	if ((error = db_create(&my_expiry_names_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_expiry_names_db->open(
		my_expiry_names_db, "expiry.hash", 0,
		DB_HASH, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: expiry.hash: %s\n",
			db_strerror(error));
		return -1;
	}
	
	return 0;
}
	
int lu_expiry_sync()
{
	int error;
	
	if ((error = my_expiry_names_db->sync(
		my_expiry_names_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: expiry.hash: %s\n",
			db_strerror(error));
	}
	
	return 0;
}

int lu_expiry_close()
{
	int error;
	int fail = 0;
	
	if ((error = my_expiry_names_db->close(
		my_expiry_names_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: expiry.hash: %s\n",
			db_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_expiry_quit()
{
	return 0;
}
