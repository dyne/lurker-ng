/*  $Id: btree.c,v 1.9 2002-07-02 21:10:17 terpstra Exp $
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
 *    License as published by the Free Software Foundation; either
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
# define assert(x) do { if (!x) { printf("\nASSERT FAILURE: %s:%i: '%s'\n", __FI
#endif

#ifdef HAVE_IO_H
# include <io.h> 
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

#ifdef USE_MMAP
# define  READ_SECTOR(k, s, b) (memcpy(b, &k->mmap[s*k->sector_size], k->sector_size), 0)
# define WRITE_SECTOR(k, s, b) (memcpy(&k->mmap[s*k->sector_size], b, k->sector_size), 0)

static size_t round_mmap_up(size_t amt)
{
	size_t out = 0xC000; /* Can't quite map 2Gb- so use 110000... */
	while (out < amt) out <<= 1;
	return out;
}

#else

static int READ_SECTOR(struct Kap_Btree* k, off_t sector, void* buf)
{
	if (lseek(k->fd, sector*k->sector_size, SEEK_SET) != 
	    sector*k->sector_size)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	if (read(k->fd, buf, k->sector_size) !=
	    k->sector_size)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	return 0;
}

static int WRITE_SECTOR(struct Kap_Btree* k, off_t sector, void* buf)
{
	if (lseek(k->fd, sector*k->sector_size, SEEK_SET) != 
	    sector*k->sector_size)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	if (write(k->fd, buf, k->sector_size) !=
	    k->sector_size)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	return 0;
}

#endif

/***************************************** Decoder methods */

static void decode_offset(const unsigned char* where, off_t* out, short len)
{
	*out = 0;
	while (len)
	{
		*out <<= 8;
		*out |= where[--len];
	}
}

static void encode_offset(unsigned char* where, off_t rec, short len)
{
	while (len--)
	{
		*where++ = rec;
		rec >>= 8;
	}
}

/* The header is always 46 bytes */
static void encode_header(Kap k, unsigned char* scan)
{
	memcpy(scan, LIBKAP_BTREE_HEAD, LIBKAP_BTREE_HEAD_LEN);
	scan += LIBKAP_BTREE_HEAD_LEN;
	
	encode_offset(scan, k->btree->sector_size,  4); scan += 4;
	encode_offset(scan, k->btree->max_key_size, 4); scan += 4;
	encode_offset(scan, k->btree->leaf_size,    4); scan += 4;
	encode_offset(scan, k->btree->tree_size,    1); scan += 1;
	encode_offset(scan, k->btree->size,         8); scan += 8;
	encode_offset(scan, k->btree->root,         8); scan += 8;
}

/* The header is always 46 bytes */
static int decode_header(Kap k, unsigned char* scan)
{
	off_t tmp;
	
	if (memcmp(scan, LIBKAP_BTREE_HEAD, LIBKAP_BTREE_HEAD_LEN))
		return KAP_BTREE_CORRUPT;
	scan += LIBKAP_BTREE_HEAD_LEN;
	
	decode_offset(scan, &tmp, 4); scan += 4;
	if (k->btree->sector_size != tmp) return KAP_WRONG_SECTOR_SIZE;
		
	decode_offset(scan, &tmp, 4); scan += 4;
	if (k->btree->max_key_size != tmp) return KAP_WRONG_MAX_KEY_SIZE;
	
	decode_offset(scan, &tmp, 1); scan += 1;
	if (k->btree->tree_size != tmp) return KAP_WRONG_TREE_SIZE;
	
	decode_offset(scan, &tmp, 1); scan += 1;
	if (k->btree->leaf_size != tmp) return KAP_WRONG_LEAF_SIZE;
	
	decode_offset(scan, &k->btree->size, 8); scan += 8;
	decode_offset(scan, &k->btree->root, 8); scan += 8;
	
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
	
	btree->sector_size  = 32*1024;
	btree->max_key_size = 100;
	btree->tree_size    = 3;
	btree->leaf_size    = 255;
	
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
	
	if (!k->btree) return 0;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	k->btree->secta = malloc(k->btree->sector_size);
	k->btree->sectb = malloc(k->btree->sector_size);
	k->btree->sectc = malloc(k->btree->sector_size);
	k->btree->scratch = malloc(k->btree->leaf_size);
	
	if (!k->btree->secta || !k->btree->sectb || !k->btree->sectc ||
		!k->btree->scratch)
		return ENOMEM;
	
	snprintf(&buf[0], sizeof(buf),
		"%s/%s.btree", dir, prefix);
	
	k->btree->fd = open(&buf[0], 
		O_BINARY | O_RDWR | O_CREAT,
		S_IREAD | S_IWRITE);
	if (k->btree->fd == -1)
		return errno;
	
	got = read(k->btree->fd, k->btree->secta, k->btree->sector_size);
	if (got == 0)
	{	/* New tree */
		k->btree->size = 2;
		k->btree->root = 1;
		
		memset(k->btree->secta, 0, k->btree->sector_size);
		encode_header(k, k->btree->secta);
		
		got = write(k->btree->fd, k->btree->secta, k->btree->sector_size);
		if (got != k->btree->sector_size)
		{
			if (errno)	return errno;
			else		return KAP_BTREE_CORRUPT;
		}
		
		memset(k->btree->secta, 0, k->btree->sector_size);
		encode_sector_header(k->btree->secta, 1, 0);
		
		got = write(k->btree->fd, k->btree->secta, k->btree->sector_size);
		if (got != k->btree->sector_size)
		{
			if (errno)	return errno;
			else		return KAP_BTREE_CORRUPT;
		}
	}
	else if (got == k->btree->sector_size)
	{
		if (decode_header(k, k->btree->secta) != 0)
			return KAP_BTREE_CORRUPT;
	}
	else
	{
		if (errno) return errno;
		return KAP_BTREE_CORRUPT;
	}
	
#ifdef USE_MMAP
	k->btree->mmap = mmap(
		0, 
		round_mmap_up(k->btree->size*k->btree->sector_size), 
		PROT_READ|PROT_WRITE, 
		MAP_SHARED,
		k->btree->fd, 
		0);
	if (k->btree->mmap == MAP_FAILED)
		return errno;
#endif
	
	return 0;
}

int kap_btree_sync(Kap k)
{
#ifdef USE_MMAP
	if (msync(
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
	int out;
	
	out = kap_btree_sync(k);
	if (out != 0) return out;
	
	if (k->btree->secta) free(k->btree->secta);
	if (k->btree->sectb) free(k->btree->sectb);
	if (k->btree->sectc) free(k->btree->sectc);
	if (k->btree->scratch) free(k->btree->scratch);
	
#ifdef USE_MMAP
	if (k->btree->mmap != MAP_FAILED)
		if (munmap(
			k->btree->mmap, 
			round_mmap_up(k->btree->size*k->btree->sector_size)) != 0)
			return errno;
#endif
	
	return close(k->btree->fd);
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
	
#ifdef USE_MMAP
	if (round_mmap_up(k->btree->sector_size * (k->btree->size + 1)) !=
	    round_mmap_up(k->btree->sector_size *  k->btree->size))
	{
		if (munmap(
			k->btree->mmap, 
			round_mmap_up(k->btree->size*k->btree->sector_size)) 
			!= 0)
		{
			return errno;
		}
		
		k->btree->mmap = mmap(
			0, 
			round_mmap_up((k->btree->size+1)*k->btree->sector_size), 
			PROT_READ|PROT_WRITE, 
			MAP_SHARED,
			k->btree->fd, 
			0);
		
		assert (k->btree->mmap != MAP_FAILED);
	}
#endif
	/* Step 1: pre-allocate storage on disk with writing 0s.
	 * This avoids making stupid sparse files.
	 */
	memset(k->btree->sectc, 0, k->btree->sector_size);
	
	go = lseek(k->btree->fd, k->btree->sector_size*k->btree->size, SEEK_SET);
	if (go != k->btree->sector_size*k->btree->size)
		return 0;
	
	did = write(k->btree->fd, k->btree->sectc, k->btree->sector_size);
	if (did != k->btree->sector_size)
		return 0;
	
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
		scan += strlen(scan) + 1;
		
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
	
	for (hits = 0, next = scan; next < mid; scan = next, hits++)
	{
		next = scan + strlen(scan) + 1;
		
		if (leaf)	next += 1 + (*next);
		else		next += k->btree->tree_size;
	}
	
	assert (hits < count);
	
	/* Scan now points at the key which overflows the mid point */
	remains = k->btree->sectb + k->btree->sector_size - scan;
	klen = strlen(scan) + 1;
	
	
	
	
	
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
	encode_offset(off, *new, k->btree->tree_size);
	
	/* Increase the hits in our parent */
	decode_sector_header(k->btree->secta, &pl, &ph);
	ph++;
	encode_sector_header(k->btree->secta, pl, ph);
	
	
	
	
	/********** Prepare the new child */
	
	
	memset(k->btree->sectc, 0, k->btree->sector_size);
	if (leaf)
	{
		sh = count - hits;
		memcpy(	k->btree->sectc + SECTOR_HEADER_SIZE,
			scan,
			remains);
	}
	else
	{
		sh = count - hits - 1;
		memcpy(	k->btree->sectc + SECTOR_HEADER_SIZE,
			scan+klen,
			remains-klen);
	}
	sl = leaf;
	encode_sector_header(k->btree->sectc, sl, sh);
	
	
	/********** Prepare the old child */
	
	
	/* Truncate the original record */
	memset(scan, 0, remains);
	encode_sector_header(k->btree->sectb, leaf, hits);
	
	
	/********** Done */
	return 0;
}

/* Precondition: x is loaded in secta
 *               sectb&c are available for overwrite
 *               x is not full.
 * 
 * Guarantee: the writes are done in an order that if interrupted at any point
 * the database will still function. This means that if we are using sync'd
 * IO this never fucks up on a power down. However, some space can be wasted.
 */
static int travel_down(Kap k, const char* key, off_t x,
	int (*decide)(void* arg, unsigned char* record, ssize_t* len),
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
	
	/* Scheme for tree nodes: (-inf, a) <a> [a, b) <b> [b, inf) */
	
	while (decode_sector_header(k->btree->secta, &leaf, &hits),
	       !leaf)
	{
		/* Find the first address (ptr) such that the following
		 * key (scan) obeys: key < scan
		 */
		
		for (i = 0, ptr = k->btree->secta + SECTOR_HEADER_SIZE;
		     i < hits;
		     i++, ptr = scan + klen + 1)
		{
			scan = ptr + k->btree->tree_size;
			klen = strlen(scan);
			
			if (strcmp(key, scan) < 0)
				break;
		}
		
		/* Pull the chid off disk */
		decode_offset(ptr, &child, k->btree->tree_size);
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
			
			if (strcmp(key, ptr) >= 0)
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
	out = -1;
	scan = k->btree->secta + SECTOR_HEADER_SIZE;
	while (i < hits)
	{
		out = strcmp(key, scan);
		if (out <= 0) break;
		
		scan += strlen(scan)+1;
		scan += 1 + (*scan);
		i++;
	}
	
	if (out == 0)
	{	/* key exists */
		ptr = scan + strlen(scan)+1;
		dlen = *ptr++;
		
		memcpy(k->btree->scratch, ptr, dlen);
		memset(k->btree->scratch+dlen, 0, k->btree->leaf_size-dlen);
	}
	else
	{	/* key dne */
		memset(k->btree->scratch, 0, k->btree->leaf_size);
		dlen = -1;
	}
	
	nlen = dlen;
	put = decide(arg, k->btree->scratch, &nlen);
	
	/* If we aren't writing; just return */
	if (!put) return 0;
	
	/* If we are deleting a non-existant record, just return */
	if (nlen == -1 && dlen == -1) return 0;
	
	/*!!! Don't delete for now 
	 * Concern is about case where the record becomes empty.
	 */
	assert (nlen != -1);
	
	if (out == 0)
	{	/* key already exists; just new data. */
		/* Shift remains */
		klen = strlen(scan)+1;
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
	int (*decide)(void* arg, unsigned char* record, ssize_t* len),
	void* arg)
{
	off_t		root = k->btree->root;
	unsigned char*	swap;
	unsigned char*	w;
	int		out;
	
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd == -1) return KAP_NOT_OPEN;
	
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
		encode_offset(w, root, k->btree->tree_size);
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
	unsigned char*	buf;
	size_t*		len;
	
};

static int kap_read_back(void* arg, unsigned char* buf, ssize_t* len)
{
	struct Read_Back* nfo = arg;
	
	if (*len != -1) memcpy(nfo->buf, buf, *len);
	*nfo->len = *len;
	
	return 0;
}

int kap_btree_read(Kap k, const char* key,
	unsigned char* buf, ssize_t* len)
{
	int out;
	struct Read_Back nfo;
	
	nfo.buf = buf;
	nfo.len = len;
	out = kap_btree_op(k, key, &kap_read_back, &nfo);
	
	if (out) return out;
	if (*len == -1) return KAP_NOT_FOUND;
	
	return 0;
}

struct Write_Back
{
	const unsigned char*	buf;
	const size_t*		len;
	int			fail;
};

static int kap_write_back(void* arg, unsigned char* buf, ssize_t* len)
{
	struct Write_Back* nfo = arg;
	
	if (*len != -1)
	{
		nfo->fail = 1;
		return 0;
	}
	
	memcpy(buf, nfo->buf, *nfo->len);
	*len = *nfo->len;
	
	return 1;
}

int kap_btree_write(Kap k, const char* key,
	const unsigned char* buf, const ssize_t* len)
{
	int out;
	struct Write_Back nfo;
	
	nfo.buf = buf;
	nfo.len = len;
	nfo.fail = 0;
	out = kap_btree_op(k, key, &kap_write_back, &nfo);
	
	if (out) return out;
	if (nfo.fail) return KAP_KEY_EXIST;
	
	return 0;
}

