/*  $Id: btree.c,v 1.27 2002-08-24 20:22:09 terpstra Exp $
 *  
 *  btree.c - Implementation of the btree access methods.
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

#include "../config.h"
#include "private.h"

/* If you need btrees larger than 2Gb you must define this: */
/* #define DISABLE_MMAP */

#if defined(HAVE_MMAP) && defined(HAVE_MUNMAP) && !defined(DISABLE_MMAP)
# define USE_MMAP
# include <sys/mman.h> 
#endif

#ifdef HAVE_ERRNO_H 
# include <errno.h>
#endif

#ifdef HAVE_ASSERT_H
# include <assert.h> 
#endif

#ifndef assert
# define assert(x) do { if (!x) { printf("\nASSERT FAILURE: %s:%i: '%s'\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
#endif

#ifdef HAVE_IO_H
# include <io.h> 
#endif

#ifdef HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#ifdef HAVE_UNISTD_H
# include <unistd.h> 
#endif

#ifdef HAVE_FCNTL_H
# include <fcntl.h> 
#endif

#ifdef HAVE_SYS_FCNTL_H
# include <sys/fcntl.h>
#endif

#ifndef O_BINARY
# define O_BINARY 0
#endif

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef DMALLOC
# include <dmalloc.h>
#endif

/* Okay, what is the format of this file?
 * 
 * This is almost a textbook implementation of a btree. However, there are
 * some important differences. We have internal leaves which hold only
 * pointers to children and keys. The leaves hold only keys and data, but no
 * pointers. Because we want to be able to do a single-pass search for the
 * first key which is >= a search term, we need to use righ-closed intervals.
 * (This is true for any sort of "first occurance" search).
 * 
 * Also, our keys are not fixed length (neither is the record). Therefore,
 * we cannot simply decide when a record is full based off of usage
 * counting. Instead, we require that a sector always have space for at least
 * one more entry of maximal size. This means that it is a good idea to have
 * a large sector size compared to the size of key+data.
 *
 * So, the structure for an internal node is:
 * <ptr to (-inf, a]> <key a> <ptr to (a, b]> <key b> <ptr to (b, inf)>
 * with the additional guarantee that there will be a record with key a in
 * the range (inf, a] and a record with key b in the range (a, b].
 *
 * A leaf has the structure:
 * <key a> <len a> <data a> ... <key b> <len b> <data b>
 *
 * With these contraints, we can implement a single pass search.
 *
 */

/***************************************** Internal btree data */

#define LIBKAP_BTREE_HEAD	"libkap btree v0.1\n\377"
#define LIBKAP_BTREE_HEAD_LEN	20

struct Kap_Btree
{
	ssize_t sector_size;
	ssize_t	max_key_size;
	
	int	tree_size;
	size_t	leaf_size;
	
	int	fd;
	off_t	root;
	off_t	size;
	
	/* We will at most ever need three sectors loaded off the disk */
	unsigned char*	secta;
	unsigned char*	sectb;
	unsigned char*	sectc;
	
	/* Buffer for callback */
	unsigned char*	scratch;
	
#ifdef USE_MMAP
	char*	mmap;
#endif
};

/***************************************** Strategies for accessing the disk */

static int IO_READ_SECTOR(struct Kap_Btree* k, off_t sector, void* buf)
{
	if (lseek(k->fd, sector*k->sector_size, SEEK_SET) != 
	    sector*k->sector_size)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	if (kap_read_full(k->fd, buf, k->sector_size) != 0)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	return 0;
}

static int IO_WRITE_SECTOR(struct Kap_Btree* k, off_t sector, const void* buf)
{
	if (lseek(k->fd, sector*k->sector_size, SEEK_SET) != 
	    sector*k->sector_size)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	if (kap_write_full(k->fd, buf, k->sector_size) != 0)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	return 0;
}

#ifdef USE_MMAP

inline int READ_SECTOR(struct Kap_Btree* k, off_t sector, void* buf)
{
	if (k->mmap != MAP_FAILED)
	{
		memcpy(buf, &k->mmap[sector*k->sector_size], k->sector_size);
		return 0;
	}
	else
	{
		return IO_READ_SECTOR(k, sector, buf);
	}
}

inline int WRITE_SECTOR(struct Kap_Btree* k, off_t sector, const void* buf)
{
	if (k->mmap != MAP_FAILED)
	{
		memcpy(&k->mmap[sector*k->sector_size], buf, k->sector_size);
		return 0;
	}
	else
	{
		return IO_WRITE_SECTOR(k, sector, buf);
	}
}

static size_t round_mmap_up(size_t amt)
{
	size_t out = 0xC000; /* Can't quite map 2Gb- so use 110000... */
	while (out < amt) out <<= 1;
	
	if (sizeof(void*) == 4 && out > 512*1024*1024)
	{
		/* Intentionally fail; greedy is bad */
		return 0xFFFFFFFFUL;
	}
	
	return out;
}

#else

inline int READ_SECTOR(struct Kap_Btree* k, off_t sector, void* buf)
{
	return IO_READ_SECTOR(k, sector, buf);
}

inline int WRITE_SECTOR(struct Kap_Btree* k, off_t sector, const void* buf)
{
	return IO_WRITE_SECTOR(k, sector, buf);
}

#endif

/***************************************** Decoder methods */

/* The header is always 46 bytes */
static void encode_header(Kap k, unsigned char* scan)
{
	memcpy(scan, LIBKAP_BTREE_HEAD, LIBKAP_BTREE_HEAD_LEN);
	scan += LIBKAP_BTREE_HEAD_LEN;
	
	kap_encode_offset(scan, k->btree->sector_size,  4); scan += 4;
	kap_encode_offset(scan, k->btree->max_key_size, 4); scan += 4;
	kap_encode_offset(scan, k->btree->leaf_size,    4); scan += 4;
	kap_encode_offset(scan, k->btree->tree_size,    1); scan += 1;
	kap_encode_offset(scan, k->btree->size,         8); scan += 8;
	kap_encode_offset(scan, k->btree->root,         8); scan += 8;
}

/* The header is always 46 bytes */
static int decode_header(Kap k, unsigned char* scan)
{
	off_t tmp;
	
	if (memcmp(scan, LIBKAP_BTREE_HEAD, LIBKAP_BTREE_HEAD_LEN))
		return KAP_BTREE_CORRUPT;
	scan += LIBKAP_BTREE_HEAD_LEN;
	
	kap_decode_offset(scan, &tmp, 4); scan += 4;
	if (k->btree->sector_size != tmp) return KAP_WRONG_SECTOR_SIZE;
		
	kap_decode_offset(scan, &tmp, 4); scan += 4;
	if (k->btree->max_key_size != tmp) return KAP_WRONG_MAX_KEY_SIZE;
	
	kap_decode_offset(scan, &tmp, 1); scan += 4;
	if (k->btree->leaf_size != tmp) return KAP_WRONG_LEAF_SIZE;
	
	kap_decode_offset(scan, &tmp, 1); scan += 1;
	if (k->btree->tree_size != tmp) return KAP_WRONG_TREE_SIZE;
	
	kap_decode_offset(scan, &k->btree->size, 8); scan += 8;
	kap_decode_offset(scan, &k->btree->root, 8); scan += 8;
	
	if (k->btree->root >= k->btree->size)
		return KAP_BTREE_CORRUPT;
	
	return 0;
}

#define SECTOR_HEADER_SIZE	2

void decode_sector_header(const unsigned char* scan, int* leaf, int* hits)
{
	*leaf = (*scan & 0x80)?1:0;
	*hits = (*scan++ & 0x7F) << 8;
	*hits |= *scan;
	
}

void encode_sector_header(unsigned char* scan, int leaf, int hits)
{
	*scan = (hits >> 8) & 0x7F;
	if (leaf) *scan |= 0x80;
	scan++;
	*scan = hits;
}

inline size_t needed_for_leaf_entry(Kap k)
{
	return 1 + k->btree->leaf_size + k->btree->max_key_size;
}

inline size_t needed_for_tree_entry(Kap k)
{
	return k->btree->tree_size + k->btree->max_key_size;
}

inline size_t needed_for_leaf_sector(Kap k)
{	/* Must be able to fit in the header and at least two entries */
	return needed_for_leaf_entry(k) * 2 + SECTOR_HEADER_SIZE;
}

inline size_t needed_for_tree_sector(Kap k)
{	/* Must be able to fit the header, two entries, and the right ptr */
	return needed_for_tree_entry(k)*2 + SECTOR_HEADER_SIZE + k->btree->tree_size;
}

/***************************************** Control btree parameters */

struct Kap_Btree* btree_init(void)
{
	struct Kap_Btree* btree = malloc(sizeof(struct Kap_Btree));
	if (!btree) return 0;
	
	btree->sector_size  = 8*1024;
	btree->max_key_size = 100;
	btree->tree_size    = 3;
	btree->leaf_size    = 4+(32*5);
	
	btree->fd = -1;
#ifdef USE_MMAP
	btree->mmap = MAP_FAILED;
#endif
	
	btree->secta = 0;
	btree->sectb = 0;
	btree->sectc = 0;
	btree->scratch = 0;
	
	return btree;
}

int kap_btree_set_sectorsize(Kap k, ssize_t size)
{
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size > 1024*1024*256)
		return ERANGE;
	if (size < 32)
		return ERANGE;
	if (needed_for_leaf_sector(k) > size)
		return ERANGE;
	if (needed_for_tree_sector(k) > size)
		return ERANGE;
	
	k->btree->sector_size = size;
	return 0;
}

int kap_btree_set_maxkeysize(Kap k, ssize_t size)
{
	ssize_t bak;
	
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size < 2)
		return ERANGE;
	
	bak = k->btree->max_key_size;
	k->btree->max_key_size = size;

	if (needed_for_leaf_sector(k) > k->btree->sector_size ||
	    needed_for_tree_sector(k) > k->btree->sector_size)
	{
		k->btree->max_key_size = bak;
		return ERANGE;
	}
	
	return 0;
}

int kap_btree_set_treesize(Kap k, short size)
{
	short bak;
	
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size < 1)
		return ERANGE;
	if (size > sizeof(off_t))
		return ERANGE;
	
	bak = k->btree->tree_size;
	k->btree->tree_size = size;
	
	if (needed_for_tree_sector(k) > k->btree->sector_size)
	{
		k->btree->tree_size = bak;
		return ERANGE;
	}
	
	return 0;
}

int kap_btree_set_leafsize(Kap k, ssize_t size)
{
	ssize_t bak;
	
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size < 1)
		return ERANGE;
	if (size > 255)
		return ERANGE;
	
	bak = k->btree->leaf_size;
	k->btree->leaf_size = size;
	
	if (needed_for_leaf_sector(k) > k->btree->sector_size)
	{
		k->btree->leaf_size = bak;
		return ERANGE;
	}
	
	return 0;
}

/***************************************** Open the btree */

int kap_btree_open(Kap k, const char* dir, const char* prefix)
{
	ssize_t	got;
	char	buf[200];
	int	out;
	int	block_device;
	
#ifdef HAVE_SYS_STAT_H
	struct stat sbuf;
#endif
	
	if (!k->btree) return 0;
	if (k->btree->fd != -1)
	{
		out = KAP_ALREADY_OPEN;
		goto kap_btree_open_error0;
	}
	
	k->btree->secta = malloc(k->btree->sector_size);
	k->btree->sectb = malloc(k->btree->sector_size);
	k->btree->sectc = malloc(k->btree->sector_size);
	k->btree->scratch = malloc(k->btree->leaf_size);
	
	if (!k->btree->secta || !k->btree->sectb || !k->btree->sectc ||
		!k->btree->scratch)
	{
		out = ENOMEM;
		goto kap_btree_open_error1;
	}
	
	snprintf(&buf[0], sizeof(buf),
		"%s/%s.btree", dir, prefix);
	
	k->btree->fd = open(&buf[0], 
		O_BINARY | O_RDWR | O_CREAT,
		S_IREAD | S_IWRITE);
	if (k->btree->fd == -1)
	{
		if (errno)	out = errno;
		else		out = ENOENT;
		goto kap_btree_open_error1;
	}
	
	if (kap_lock(k->btree->fd))
	{
		if (errno)	out = errno;
		else		out = EBUSY;
		goto kap_btree_open_error2;
	}
	
	block_device = 0;
#ifdef HAVE_SYS_STAT_H
	if (fstat(k->btree->fd, &sbuf) != 0)
	{
		if (errno)	out = errno;
		else		out = ENOENT;
		goto kap_btree_open_error3;
	}
	
	block_device = S_ISBLK(sbuf.st_mode);
#endif
	
	got = read(k->btree->fd, k->btree->secta, k->btree->sector_size);
	if (block_device && got == k->btree->sector_size &&
		memcmp(k->btree->secta, LIBKAP_BTREE_HEAD, LIBKAP_BTREE_HEAD_LEN))
	{	/* Newly initialized block device */
		got = 0;
		lseek(k->btree->fd, 0, SEEK_SET);
	}
	
	if (got == 0)
	{	/* New tree */
		k->btree->size = 2;
		k->btree->root = 1;
		
		memset(k->btree->secta, 0, k->btree->sector_size);
		encode_header(k, k->btree->secta);
		
		got = kap_write_full(k->btree->fd, k->btree->secta, k->btree->sector_size);
		if (got != 0)
		{
			if (errno)	out = errno;
			else		out = KAP_BTREE_CORRUPT;
			goto kap_btree_open_error3;
		}
		
		memset(k->btree->secta, 0, k->btree->sector_size);
		encode_sector_header(k->btree->secta, 1, 0);
		
		got = kap_write_full(k->btree->fd, k->btree->secta, k->btree->sector_size);
		if (got != 0)
		{
			if (errno)	out = errno;
			else		out = KAP_BTREE_CORRUPT;
			goto kap_btree_open_error3;
		}
	}
	else if (got == k->btree->sector_size)
	{
		out = decode_header(k, k->btree->secta);
		if (out != 0) goto kap_btree_open_error3;
	}
	else
	{
		if (errno)	out = errno;
		else		out = KAP_BTREE_CORRUPT;
		goto kap_btree_open_error3;
	}
	
#ifdef USE_MMAP
	k->btree->mmap = mmap(
		0, 
		round_mmap_up(k->btree->size*k->btree->sector_size), 
		PROT_READ|PROT_WRITE, 
		MAP_SHARED,
		k->btree->fd, 
		0);
	if (k->btree->mmap == MAP_FAILED && errno != ENOMEM)
	{
		if (errno)	out = errno;
		else		out = ENOMEM;
		goto kap_btree_open_error3;
	}
#endif
	
	return 0;

kap_btree_open_error3:
	kap_unlock(k->btree->fd);

kap_btree_open_error2:
	close(k->btree->fd);
	k->btree->fd = -1;
	
kap_btree_open_error1:
	if (k->btree->secta)   free(k->btree->secta);   k->btree->secta   = 0;
	if (k->btree->sectb)   free(k->btree->sectb);   k->btree->sectb   = 0;
	if (k->btree->sectc)   free(k->btree->sectc);   k->btree->sectc   = 0;
	if (k->btree->scratch) free(k->btree->scratch); k->btree->scratch = 0;
	
kap_btree_open_error0:
	return out;
}

int kap_btree_sync(Kap k)
{
#ifdef USE_MMAP
	if (k->btree->mmap != MAP_FAILED && msync(
		k->btree->mmap,
		round_mmap_up(k->btree->size*k->btree->sector_size),
		MS_SYNC | MS_INVALIDATE) != 0)
	{
		return errno;
	}
#endif
	
	if (fsync(k->btree->fd) != 0)
	{
		return errno;
	}
	
	return 0;
}

int kap_btree_close(Kap k)
{
	int out, ret;
	
	out = 0;
	
	if ((ret = kap_btree_sync(k)) != 0) out = ret;
	
	if (k->btree->secta)   free(k->btree->secta);   k->btree->secta   = 0;
	if (k->btree->sectb)   free(k->btree->sectb);   k->btree->sectb   = 0;
	if (k->btree->sectc)   free(k->btree->sectc);   k->btree->sectc   = 0;
	if (k->btree->scratch) free(k->btree->scratch); k->btree->scratch = 0;
	
#ifdef USE_MMAP
	if (k->btree->mmap != MAP_FAILED)
		if ((ret = munmap(
			k->btree->mmap, 
			round_mmap_up(k->btree->size*k->btree->sector_size))) != 0)
			out = ret;
#endif
	
	if ((ret = kap_unlock(k->btree->fd)) != 0) out = ret;
	
	if ((ret = close(k->btree->fd)) != 0) out = ret;
	
	return out;
}

/***************************************** Allocate more space */

/** Precondition: sectc is available for overwrite.
 *
 * Guarantee: the writes are done in an order that if interrupted at any point
 * the database will still function. This means that if we are using sync'd
 * IO this never fucks up on a power down. However, some space can be wasted.
 */
static off_t allocate_cell(Kap k)
{
	off_t	out = k->btree->size;
	off_t	go;
	ssize_t	did;
	void*	tmp;
	
	/* Step 1: pre-allocate storage on disk with writing 0s.
	 * This avoids making stupid sparse files.
	 */
	memset(k->btree->sectc, 0, k->btree->sector_size);
	
	go = lseek(k->btree->fd, k->btree->sector_size*k->btree->size, SEEK_SET);
	if (go != k->btree->sector_size*k->btree->size)
		return 0;
	
	did = kap_write_full(k->btree->fd, k->btree->sectc, k->btree->sector_size);
	if (did != 0) return 0;
	
#ifdef USE_MMAP
	if (round_mmap_up(k->btree->sector_size * (k->btree->size + 1)) !=
	    round_mmap_up(k->btree->sector_size *  k->btree->size))
	{
		tmp = mmap(
			0, 
			round_mmap_up((k->btree->size+1)*k->btree->sector_size), 
			PROT_READ|PROT_WRITE, 
			MAP_SHARED,
			k->btree->fd, 
			0);
		
		if (tmp == MAP_FAILED && errno != ENOMEM)
			return 0;
		
		if (k->btree->mmap != MAP_FAILED && munmap(
			k->btree->mmap, 
			round_mmap_up(k->btree->size*k->btree->sector_size)) 
			!= 0)
		{
			if (tmp != MAP_FAILED) munmap(tmp, round_mmap_up((k->btree->size+1)*k->btree->sector_size));
			return 0;
		}
		
		k->btree->mmap = tmp;
	}
#endif
	
	/* Only now record that we have consumed this space. 
	 * If we fuck up and don't use this space, it is lost, but that is
	 * fine: the db will be ok.
	 */
	k->btree->size++;
	encode_header(k, k->btree->sectc);
	
	if (WRITE_SECTOR(k->btree, 0, k->btree->sectc) != 0)
		return 0;
	
	return out;
}

/***************************************** Functions to parse the sector */

/* Doesn't touch the disk
 */
static int is_full(Kap k, const unsigned char* sector)
{
	int			i, leaf, count;
	size_t			size, remain;
	const unsigned char*	scan;
	
	decode_sector_header(sector, &leaf, &count);
	
	if (leaf) scan = sector + SECTOR_HEADER_SIZE;
	else      scan = sector + SECTOR_HEADER_SIZE + k->btree->tree_size;
	
	for (i = 0; i < count; i++)
	{
		while (*scan) ++scan;
		++scan;
		
		if (leaf)	scan += 1 + (*scan);
		else		scan += k->btree->tree_size;
	}
	
	size = scan - sector;
	remain = k->btree->sector_size - size;
	
	if (leaf) return remain < needed_for_leaf_entry(k);
	else      return remain < needed_for_tree_entry(k);
}

/***************************************** Btree insertion code */

/* Precondition: parent is loaded in secta
 *               child  is loaded in sectb
 *               sectc is available for overwrite
 *               off is the offset of the position in secta to insert the 
 * 			splitting key
 * This method will split a full child y of x (which is not full) and insert
 * the new key into offset off in x.
 * 
 * Please note: aside from allocating space this does NOT read/write to the
 * disk.  You must write: *new (sectc), parent (secta), child (sectb) in
 * this order to preserve database integrity at all points. Some callers may
 * need to put additional writes in the middle for integrity.
 */
static int split_child(Kap k, off_t parent, off_t child, off_t* new,
			unsigned char* off)
{
	unsigned char* mid;
	unsigned char* next;
	unsigned char* scan;
	
	int hits, leaf, count;
	size_t klen, follows, remains, needed;
	
	int pl, ph, sl, sh;
	
	/* Amount after off in parent */
	follows = k->btree->secta + k->btree->sector_size - off;
	
	
	
	
	/* Find the (adjusted for needed free space) mid point of the child */
	decode_sector_header(k->btree->sectb, &leaf, &count);
	
	if (leaf)	needed = needed_for_leaf_entry(k);
	else		needed = needed_for_tree_entry(k);
	
	mid = k->btree->sectb + (k->btree->sector_size - needed) / 2;
	
	
	scan = k->btree->sectb + SECTOR_HEADER_SIZE;
	if (!leaf) scan += k->btree->tree_size;
	
	assert (scan < mid);
	
	/* Find the record scan whose end is past the midpoint
	 * Find the record next which follow scan
	 * Count the number of hits up to and including scan
	 */
	next = scan;
	for (hits = 0; next < mid; hits++)
	{
		scan = next;
		while (*next) next++;
		next++;
		
		if (leaf)	next += 1 + (*next);
		else		next += k->btree->tree_size;
	}
	
	assert (hits < count);
	assert (hits > 1);
	
	/* Scan now points at the key which overflows the mid point 
	 * next is the first entry to go in the split_child
	 */
	klen = strlen((char*)scan) + 1;
	
	
	
	
	
	/* Allocate the new cell */
	*new = allocate_cell(k);
	if (*new == 0) return KAP_NO_SPACE;
	
	
	
	
	
	/********** Prepare the parent */
	
	
	/* Move the memory of our parent to accomodate this record plus an
	 * additional tree_size
	 */
	memmove(off + klen + k->btree->tree_size,	/* new location */
		off,					/* old location */
		follows - klen - k->btree->tree_size);
	
	/* Throw in the new key and target the new record */
	memcpy(off, scan, klen);
	off += klen;
	kap_encode_offset(off, *new, k->btree->tree_size);
	
	/* Increase the hits in our parent */
	decode_sector_header(k->btree->secta, &pl, &ph);
	ph++;
	encode_sector_header(k->btree->secta, pl, ph);
	
	
	
	
	/********** Prepare the new child */
	
	
	memset(k->btree->sectc, 0, k->btree->sector_size);
	sh = count - hits;
	if (leaf)
	{
		remains = k->btree->sectb + k->btree->sector_size - next;
		memcpy(	k->btree->sectc + SECTOR_HEADER_SIZE,
			next,
			remains);
	}
	else
	{
		remains = k->btree->sectb + k->btree->sector_size - scan;
		memcpy(	k->btree->sectc + SECTOR_HEADER_SIZE,
			scan+klen,
			remains-klen);
	}
	sl = leaf;
	encode_sector_header(k->btree->sectc, sl, sh);
	
	
	/********** Prepare the old child */
	
	
	/* Truncate the original record */
	if (leaf)
	{
		memset(next, 0, remains);
		encode_sector_header(k->btree->sectb, leaf, hits);
	}
	else
	{
		memset(scan, 0, remains);
		encode_sector_header(k->btree->sectb, leaf, hits-1);
	}
	
	
	/********** Done */
	return 0;
}

/* Precondition: x is loaded in secta
 *               sectb&c are available for overwrite
 *               x is not full.
 * 
 * Attempt to find the first record which is >= key.
 *
 * Guarantee: the writes are done in an order that if interrupted at any point
 * the database will still function. This means that if we are using sync'd
 * IO this never fucks up on a power down. However, some space can be wasted.
 */
static int travel_down(Kap k, const char* key, off_t x,
	int (*decide)(void* arg, const char* key, unsigned char* record, ssize_t* len),
	void* arg)
{
	int i, out, put;
	
	unsigned char*	ptr;
	unsigned char*	scan;
	size_t		klen;
	off_t		child;
	off_t		split;
	unsigned char*	swap;
	ssize_t		dlen, nlen;
	size_t		remains;
	int		leaf, hits;
	const unsigned char*	s;
	
	/* Scheme for tree nodes: (-inf, a] <a> (a, b] <b> (b, inf) 
	 * Also, one is guaranteed to find a key <k> in (-inf, k] if one
	 * saw a <k> on the way down. This is how we can find the first
	 * key >= the search in a single pass. See top of file.
	 */
	
	while (decode_sector_header(k->btree->secta, &leaf, &hits),
	       !leaf)
	{
		/* Find the first address (ptr) such that the following
		 * key (scan) obeys: key <= scan
		 */
		
		for (i = 0, ptr = k->btree->secta + SECTOR_HEADER_SIZE;
		     i < hits;
		     i++, ptr = scan+1)
		{	/* Sorry for the unreadable code, but this is the bottleneck */
			scan = ptr + k->btree->tree_size;
			
			s = (const unsigned char*)key;
			while (*scan)
			{
				if (*s != *scan) break;
				++s;
				++scan;
			}
			if (*s <= *scan) break;
			
			while (*scan) ++scan;
		}
		
		/* Pull the chid off disk */
		kap_decode_offset(ptr, &child, k->btree->tree_size);
		ptr += k->btree->tree_size;
		if (READ_SECTOR(k->btree, child, k->btree->sectb) != 0)
			return errno;
		
		if (is_full(k, k->btree->sectb))
		{	/* Preserve the "not full" precondition */
			out = split_child(k, x, child, &split, ptr);
			if (out != 0) return out;
			
			if (WRITE_SECTOR(k->btree, split, k->btree->sectc))
				return errno;
			if (WRITE_SECTOR(k->btree, x, k->btree->secta))
				return errno;
			if (WRITE_SECTOR(k->btree, child, k->btree->sectb))
				return errno;
			
			s = (const unsigned char*)key;
			while (*ptr)
			{
				if (*s != *ptr) break;
				++ptr;
				++s;
			}
			
			if (*s > *ptr)
			{	/* We want to recurse to split instead */
				child = split;
				swap = k->btree->sectc;
				k->btree->sectc = k->btree->sectb;
				k->btree->sectb = swap;
			}
		}
		
		/* Continue scanning, but make the child the position */
		swap = k->btree->sectb;
		k->btree->sectb = k->btree->secta;
		k->btree->secta = swap;
		
		x = child;
	}
	
	/* Okay, we have reached the leaf */
	
	/* Find the first key which is >= the search key */
	i = 0;
	scan = k->btree->secta + SECTOR_HEADER_SIZE;
	while (i < hits)
	{	/* Sorry for the unreadable code, but this is the bottleneck */
		s = (const unsigned char*)key;
		while (*scan)
		{
			if (*s != *scan) break;
			++s;
			++scan;
		}
		
		if (*s <= *scan)
		{
			/* rewind now that we are out of the loop */
			scan -= ((long)s) - ((long)key);
			break;
		}
		
		while (*scan) ++scan;
		++scan;
		
		scan += 1 + (*scan);
		i++;
	}
	
	if (i != hits)
	{	/* found a key >= search */
		ptr = scan + strlen((char*)scan)+1;
		dlen = *ptr++;
		
		memcpy(k->btree->scratch, ptr, dlen);
		memset(k->btree->scratch+dlen, 0, k->btree->leaf_size-dlen);
		
		nlen = dlen;
		put = decide(arg, (const char*)scan, k->btree->scratch, &nlen);
	}
	else
	{	/* hit end of tree */
		memset(k->btree->scratch, 0, k->btree->leaf_size);
		dlen = -1;
		
		nlen = dlen;
		put = decide(arg, "", k->btree->scratch, &nlen);
	}
	
	/* If we aren't writing; just return */
	if (!put) return 0;
	
	assert (nlen <= k->btree->leaf_size);
	
	/* If we are deleting a non-existant record, just return */
	if (nlen == -1 && dlen == -1) return 0;
	
	/*!!! Don't delete for now 
	 * Concern is about case where the record becomes empty.
	 */
	assert (nlen != -1);
	
	if (i < hits && !strcmp(key, (char*)scan))
	{	/* key already exists; just new data. */
		/* Shift remains */
		klen = strlen((char*)scan)+1;
		ptr = scan + klen;
		*ptr++ = nlen;
		
		remains = k->btree->secta + k->btree->sector_size - ptr;
		if (dlen < nlen)
			memmove(ptr+nlen, ptr+dlen, remains - nlen);
		else
			memmove(ptr+nlen, ptr+dlen, remains - dlen);
			
		memcpy(ptr, k->btree->scratch, nlen);
	}
	else
	{	/* Key did not exist */
		klen = strlen(key)+1;
		ptr = scan+klen+1+nlen;
		
		remains = k->btree->secta + k->btree->sector_size - ptr;
		memmove(ptr, scan, remains);
		
		memcpy(scan, key, klen);
		scan += klen;
		
		*scan++ = nlen;
		memcpy(scan, k->btree->scratch, nlen);
		
		assert (scan + nlen <= k->btree->secta + k->btree->sector_size);
		
		encode_sector_header(k->btree->secta, leaf, hits+1);
	}
	
	if (WRITE_SECTOR(k->btree, x, k->btree->secta) != 0)
		return errno;
	
	return 0;
}

/***************************************** Module entry point */

/* Precondition: secta&b&c are all available for overwrite
 *               (should be true since we are the only method to access them,
 *                we don't recurse, and threading be damned - lock yourself)
 * 
 * Guarantee: the writes are done in an order that if interrupted at any point
 * the database will still function. This means that if we are using sync'd
 * IO this never fucks up on a power down. However, some space can be wasted.
 */
int kap_btree_op(Kap k, const char* key,
	int (*decide)(void* arg, const char* key, unsigned char* record, ssize_t* len),
	void* arg)
{
	off_t		root = k->btree->root;
	unsigned char*	swap;
	unsigned char*	w;
	int		out;
	
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd == -1) return KAP_NOT_OPEN;
	if (strlen(key) >= k->btree->max_key_size) return ERANGE;
	if (!*key) return ERANGE;
	
	if (READ_SECTOR(k->btree, root, k->btree->secta) != 0)
		return errno;
	
	if (is_full(k, k->btree->secta))
	{
		off_t split_root;
		off_t new_root;
		
		/* First get some space */
		new_root = allocate_cell(k);
		
		if (new_root == 0) return KAP_NO_SPACE;
		
		/* root is now a child, move it to sectb */
		swap = k->btree->secta;
		k->btree->secta = k->btree->sectb;
		k->btree->sectb = swap;
		
		/* Setup sector a(new_root) with root as a child */
		memset(k->btree->secta, 0, k->btree->sector_size);
		encode_sector_header(k->btree->secta, 0, 0);
		
		w = k->btree->secta + SECTOR_HEADER_SIZE;
		kap_encode_offset(w, root, k->btree->tree_size);
		w += k->btree->tree_size;
		
		/* Split the old root. We will need to obey this functions
		 * write order.
		 */
		out = split_child(k, 
			new_root,	/* new_root is the new parent */
			root,		/* root is the only child */
			&split_root,	/* the sector for the new root */
			w		/* The position to insert the key */
			);
		
		/* Proceed only if the split worked */
		if (out != 0) return out;
		
		/* Write out the newly allocated cell - unreferenced for now. */
		if (WRITE_SECTOR(k->btree, split_root, k->btree->sectc) != 0)
			return errno;
		
		/* Ok, that worked, now reference it from our new root
		 * (which is not yet referenced itself)
		 */
		if (WRITE_SECTOR(k->btree, new_root, k->btree->secta) != 0)
			return errno;
		
		/* Now, write this new root into our file magic header. This
		 * commits us to using it. Note: we could fail between here
		 * and the next step which would result in totally stupid
		 * keys in the old root. However, since we would never use
		 * them when we look at it's parent to decide where to
		 * traverse to, this is just wasted space. Also, we are done
		 * with sectc, so use it.
		 */
		k->btree->root = new_root;
		memset(k->btree->sectc, 0, k->btree->sector_size);
		encode_header(k, k->btree->sectc);
		if (WRITE_SECTOR(k->btree, 0, k->btree->sectc) != 0)
			return errno;
		
		/* Finally, write out the old root to remove pointless keys */
		if (WRITE_SECTOR(k->btree, root, k->btree->sectb) != 0)
			return errno;
		
		/* Alright! We have successfully increased the tree depth by 1.
		 * Now, we can use our helper method to do the rest of the work.
		 * The new_root is not full, is loaded in secta, and he can
		 * do whatever he likes to sectb&c
		 */
		
		return travel_down(k, key, new_root, decide, arg);
	}
	else
	{
		/* The root is not full, is loaded in secta, and we
		 * have not used sectb&c.
		 */
		return travel_down(k, key, root, decide, arg);
	}
}

struct Read_Back
{
	const char*	key;
	unsigned char*	buf;
	ssize_t*	len;
};

static int kap_read_back(void* arg, const char* key, unsigned char* buf, ssize_t* len)
{
	struct Read_Back* nfo = arg;
	
	if (!strcmp(key, nfo->key))
	{
		memcpy(nfo->buf, buf, *len);
		*nfo->len = *len;
	}
	else
	{
		*nfo->len = -1;
	}
	
	return 0;
}

int kap_btree_read(Kap k, const char* key,
	unsigned char* buf, ssize_t* len)
{
	int out;
	struct Read_Back nfo;
	
	nfo.key = key;
	nfo.buf = buf;
	nfo.len = len;
	out = kap_btree_op(k, key, &kap_read_back, &nfo);
	
	if (out) return out;
	if (*len == -1) return KAP_NOT_FOUND;
	
	return 0;
}

struct Write_Back
{
	const char*		key;
	const unsigned char*	buf;
	size_t			len;
	int			fail;
};

static int kap_write_back(void* arg, const char* key, unsigned char* buf, ssize_t* len)
{
	struct Write_Back* nfo = arg;
	
	if (!strcmp(key, nfo->key))
	{
		nfo->fail = 1;
		return 0;
	}
	else
	{
		memcpy(buf, nfo->buf, nfo->len);
		*len = nfo->len;
		return 1;
	}
}

int kap_btree_write(Kap k, const char* key,
	const unsigned char* buf, ssize_t len)
{
	int out;
	struct Write_Back nfo;	
	
	nfo.key = key;
	nfo.buf = buf;
	nfo.len = len;
	nfo.fail = 0;
	out = kap_btree_op(k, key, &kap_write_back, &nfo);
	
	if (out) return out;
	if (nfo.fail) return KAP_KEY_EXIST;
	
	return 0;
}

struct Rewrite_Back
{
	const unsigned char*	buf;
	size_t			len;
};

static int kap_rewrite_back(void* arg, const char* key, unsigned char* buf, ssize_t* len)
{
	struct Rewrite_Back* nfo = arg;
	
	memcpy(buf, nfo->buf, nfo->len);
	*len = nfo->len;
	return 1;
}

int kap_btree_rewrite(Kap k, const char* key,
	const unsigned char* buf, ssize_t len)
{
	int out;
	struct Rewrite_Back nfo;	
	
	nfo.buf = buf;
	nfo.len = len;
	out = kap_btree_op(k, key, &kap_rewrite_back, &nfo);
	
	if (out) return out;
	
	return 0;
}

struct ReadGe_Back
{
	char*		key;
	unsigned char*	buf;
	ssize_t*	len;
};

static int kap_read_ge_back(void* arg, const char* key, unsigned char* buf, ssize_t* len)
{
	struct ReadGe_Back* nfo = arg;
	
	strcpy(nfo->key, key);
	*nfo->len = *len;
	if (*len != -1) memcpy(nfo->buf, buf, *len);
	
	return 0;
}

int kap_btree_read_ge(Kap k, char* key,
	unsigned char* buf, ssize_t* len)
{
	int	out;
	struct ReadGe_Back nfo;
	
	nfo.key = key;
	nfo.buf = buf;
	nfo.len = len;
	out = kap_btree_op(k, key, &kap_read_ge_back, &nfo);
	
	if (out) return out;
	if (*nfo.len == -1) return KAP_NOT_FOUND;
	
	return 0;
}

int kap_btree_next_key(Kap k, char* key)
{
	unsigned char*	ks;
	
	ks =  (unsigned char*)key;
	while (*ks) ks++;
	
	if (ks+1 == (unsigned char*)key + k->btree->max_key_size)
	{
		ks--;
		while (ks != (unsigned char*)key)
		{
			if (*ks != 0xFF)
			{
				++*ks;
				break;
			}
			else
			{
				*ks = 0;
				--ks;
			}
		}
		if (ks == (unsigned char*)key) return KAP_NOT_FOUND;
	}
	else
	{
		*ks++ = 1;
		*ks = 0;
	}
	
	return 0;
}

int kap_btree_read_next(Kap k, char* key,
	unsigned char* buf, ssize_t* len)
{
	kap_btree_next_key(k, key);
	return kap_btree_read_ge(k, key, buf, len);
}
