/*  $Id: expiry.c,v 1.15 2002-08-12 13:02:52 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"

#include "config.h"
#include "expiry.h"

#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>

#include <st.h>

#ifdef DMALLOC
# include <dmalloc.h>
#endif

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
	
	char	file[200];
} My_Expiry_Heap;

typedef struct My_Expiry_List_T
{
	lu_word	list; /* for the 0th entry, this is 'head' */
	lu_word	tail;
} My_Expiry_List;

static int	my_expiry_stop_watch	= 0;
static int	my_expiry_skip_watch	= 0;

static int	my_expiry_heap_fd	= -1;
static int	my_expiry_list_fd	= -1;

static long	my_expiry_space_used	= 0;

My_Expiry_Heap*	my_expiry_heap = 0;	/* Stored as -1 of mmap */
My_Expiry_List*	my_expiry_list = 0;

static long my_expiry_heaps = 0;
static long my_expiry_lists = 0;

#define PARENT(i)	(i >> 1)
#define LEFT(i)		(i << 1)
#define RIGHT(i)	((i << 1) | 1)

static lu_word my_expiry_find_head(lu_word list)
{
	Lu_Config_List* ls;
	
	assert (list != LU_EXPIRY_NO_LIST);
	if (list == LU_EXPIRY_ANY_LIST)
		return 1;
	
	ls = lu_config_find_listi(lu_config_file, list);
	assert(ls);
	return ls->cache_head;
}

inline void my_expiry_update_pos(lu_word x, lu_word y)
{
	lu_word nl = my_expiry_heap[x].next_list;
	lu_word pl = my_expiry_heap[x].prev_list;
	lu_word nt = my_expiry_heap[x].next_time;
	lu_word pt = my_expiry_heap[x].prev_time;
	lu_word ls = my_expiry_heap[x].list;
	
	if      (pl == y)	my_expiry_heap[ x].prev_list = x;
	else if (pl != 0)	my_expiry_heap[pl].next_list = y;
	
	if      (nl == y)	my_expiry_heap[ x].next_list = x;
	else if (nl != 0)	my_expiry_heap[nl].prev_list = y;
	else if (ls != LU_EXPIRY_NO_LIST)
			my_expiry_list[my_expiry_find_head(ls)].tail = y;
	
	if      (pt == y)	my_expiry_heap[ x].prev_time = x;
	else if (pt != 0)	my_expiry_heap[pt].next_time = y;
	else			my_expiry_list[ 0].list = y;
	
	if	(nt == y)	my_expiry_heap[ x].next_time = x;
	else if (nt != 0)	my_expiry_heap[nt].prev_time = y;
	else			my_expiry_list[ 0].tail = y;
}

static void my_expiry_swap_pos(lu_word a, lu_word b)
{
	My_Expiry_Heap tmp;
	
	my_expiry_update_pos(a, b);
	my_expiry_update_pos(b, a);
	
	memcpy(&tmp,               &my_expiry_heap[a], sizeof(My_Expiry_Heap));
	memcpy(&my_expiry_heap[a], &my_expiry_heap[b], sizeof(My_Expiry_Heap));
	memcpy(&my_expiry_heap[b], &tmp,               sizeof(My_Expiry_Heap));
}

static int my_expiry_pop(lu_word x)
{
	lu_word pl = my_expiry_heap[x].prev_list;
	lu_word nl = my_expiry_heap[x].next_list;
	lu_word pt = my_expiry_heap[x].prev_time;
	lu_word nt = my_expiry_heap[x].next_time;
	lu_word ls, smallest, left, right;
	
	assert(x <= my_expiry_heaps);
	
	/* Actually delete the file */
#ifdef DEBUG
	printf("Killing expired file: %s\n", &my_expiry_heap[x].file[0]);
#endif
	/* Ignore the case that it's already been killed */
	if (unlink(&my_expiry_heap[x].file[0]) && errno != ENOENT)
	{
		syslog(LOG_ERR, _("Deleting file '%s' failed: %s\n"),
			&my_expiry_heap[x].file[0],
			strerror(errno));
	}
	
	/* Decrease the space tracker */
	my_expiry_space_used -= my_expiry_heap[x].size;
	
	/* Pull it out of the list of things in watching a mailinglist */
	if (pl)	my_expiry_heap[pl].next_list = nl;
	if (nl)	my_expiry_heap[nl].prev_list = pl;
	else if ((ls = my_expiry_heap[x].list) != LU_EXPIRY_NO_LIST)
	{
		my_expiry_list[my_expiry_find_head(ls)].tail = pl;
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
		my_expiry_update_pos(x, x);
	}
	
	my_expiry_heaps--;
	if (ftruncate(my_expiry_heap_fd, 
		sizeof(My_Expiry_Heap) * my_expiry_heaps) != 0)
	{
		syslog(LOG_ERR, _("Unable to reduce disk space cache heap: %s\n"),
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
	time_t  key;
	
	if (strlen(path) >= 400)
		return 1;
	
	if (size >= lu_config_file->cache_cutoff)
		return 1;
	
	while (my_expiry_heaps >= lu_config_file->cache_files ||
	       my_expiry_space_used + size >= lu_config_file->cache_size)
	{	/* As long as we are over-weight, reduce the oldest */
		if (my_expiry_pop(my_expiry_list[0].list) != 0)
			return 1;	/* something went wrong - stop cache */
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
		my_expiry_update_pos(where, where);
		where = PARENT(where);
	}
	
	/* where is now the location of our insertion */
	
	my_expiry_heap[where].expiry = key;
	my_expiry_heap[where].size   = size;
	
	my_expiry_heap[where].list = list_watch;
	snprintf(&my_expiry_heap[where].file[0], 
		sizeof(my_expiry_heap[where].file), 
		"%s", path);
	
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
		what_list = my_expiry_find_head(list_watch);
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
	lu_word what_list = my_expiry_find_head(list);
	
	while (my_expiry_list[what_list].tail)
	{
		if (my_expiry_pop(my_expiry_list[what_list].tail) != 0)
			return -1;
	}
	
	return 0;
}

static void* my_expiry_watch(void* arg)
{
	while (!my_expiry_stop_watch)
	{
		st_sleep(1);
		
		if (my_expiry_skip_watch)
			continue;
		
		while (my_expiry_heaps > 1 && my_expiry_heap[1].expiry < time(0))
		{
			if (my_expiry_pop(1) != 0)
				break;
		}
	}
	
	return 0;
}

/*------------------------------------------------ Public component methods */

int lu_expiry_init()
{
	my_expiry_stop_watch = 0;
	my_expiry_skip_watch = 1;
	st_thread_create(&my_expiry_watch, 0, 0, 0);
	return 0;
}

int lu_expiry_open()
{
	int		error;
	My_Expiry_List	list;
	My_Expiry_List	head[2];
	Lu_Config_List* clist;
	
	my_expiry_skip_watch = 0;
	
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
	
	my_expiry_heap = mmap(NULL, 
		lu_config_file->cache_files * sizeof(My_Expiry_Heap),
		PROT_READ | PROT_WRITE, MAP_SHARED, my_expiry_heap_fd, 0);
	if (my_expiry_heap == MAP_FAILED)
	{
		perror(_("mmap'ing the expiry heap"));
		return -1;
	}
	my_expiry_heap--; /* So we can address it from 1 */
	my_expiry_heaps = lseek(my_expiry_heap_fd, 0, SEEK_END) / sizeof(My_Expiry_Heap);
	
	error = read(my_expiry_list_fd, &head[0], sizeof(My_Expiry_List)*2);
	if (error == -1)
	{
		perror(_("reading the expiry head"));
		return -1;
	}
	
	my_expiry_lists = 2;
	if (error == 0)
	{	/* new file */
		memset(&head[0], 0, 2*sizeof(My_Expiry_List));
		head[1].list = LU_EXPIRY_ANY_LIST;
		
		if (write(my_expiry_list_fd, &head[0], sizeof(My_Expiry_List)*2)
			!= sizeof(My_Expiry_List)*2)
		{
			perror(_("writing the expiry head"));
			return -1;
		}
	}
	else
	{	/* scan for known lists */
		while ((error = read(my_expiry_list_fd, &list, sizeof(My_Expiry_List)))
			== sizeof(My_Expiry_List))
		{
			clist = lu_config_find_listi(lu_config_file, list.list);
			if (clist)
				clist->cache_head = my_expiry_lists;
			my_expiry_lists++;
		}
		
		if (error == -1)
		{
			perror(_("reading the expiry list"));
			return -1;
		}
	}
	
	for (clist = lu_config_file->list; 
	     clist != lu_config_file->list + lu_config_file->lists; 
	     clist++)
	{
		if (clist->cache_head == 0)
		{
			clist->cache_head = my_expiry_lists++;
			list.list = clist->key;
			list.tail = 0;
			if (write(my_expiry_list_fd, &list, sizeof(My_Expiry_List))
				!= sizeof(My_Expiry_List))
			{
				perror(_("writing new records to expiry list"));
				return -1;
			}
		}
	}
	
	my_expiry_list = mmap(NULL, 
		my_expiry_lists * sizeof(My_Expiry_List),
		PROT_READ | PROT_WRITE, MAP_SHARED, my_expiry_list_fd, 0);
	if (my_expiry_list == MAP_FAILED)
	{
		perror(_("mmap'ing the expiry list"));
		return -1;
	}
	
	lu_expiry_notice_import(LU_EXPIRY_ANY_LIST);
	
	return 0;
}
	
int lu_expiry_sync()
{
	if (msync(my_expiry_heap+1,  
		lu_config_file->cache_files * sizeof(My_Expiry_Heap),
		MS_SYNC) != 0)
	{
		syslog(LOG_ERR, _("Syncing the file heap map: %s\n"),
			strerror(errno));
	}
	
	if (msync(my_expiry_list,  
		my_expiry_lists * sizeof(My_Expiry_List),
		MS_SYNC) != 0)
	{
		syslog(LOG_ERR, _("Syncing the file list map: %s\n"),
			strerror(errno));
	}
	
	return 0;
}

int lu_expiry_close()
{
	int fail = 0;
	
	my_expiry_skip_watch = 1;
	
	if (munmap(my_expiry_heap+1, 
		lu_config_file->cache_files * sizeof(My_Expiry_Heap)) != 0)
	{
		syslog(LOG_ERR, _("Closing the file heap mmap: %s\n"),
			strerror(errno));
	}
	
	if (munmap(my_expiry_list,
		my_expiry_lists * sizeof(My_Expiry_List)) != 0)
	{
		syslog(LOG_ERR, _("Closing the file list mmap: %s\n"),
			strerror(errno));
	}
	
	close(my_expiry_heap_fd);
	close(my_expiry_list_fd);
	
	return fail;
}

int lu_expiry_quit()
{
	my_expiry_stop_watch = 1;
	return 0;
}
