/*  $Id: btree.c,v 1.1 2002-07-01 11:16:25 terpstra Exp $
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

#define LIBKAP_BTREE_HEAD	"libkap btree v0.1\n\004"
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
	
#ifdef USE_MMAP
	char*	mmap;
#endif
};

struct Sector_Header
{
	unsigned leaf  : 1;
	unsigned count : 15;
};

/***************************************** Strategies for accessing the disk */

#ifdef USE_MMAP
# define  READ_SECTOR(k, s, b) (memcpy(b, &k->mmap[s*k->sector_size], k->sector_size), 0)
# define WRITE_SECTOR(k, s, b) (memcpy(&k->mmap[s*k->sector_size], b, k->sector_size), 0)

static size_t round_mmap_up(size_t amt)
{
	size_t out = 0xC000; /* Can't quite map 2Gb- so use 110000... */
	while (out < amt) out >>= 1;
	return out;
}

#else

static int READ_SECTOR(struct Kap_Btree* k, off_t sector, void* buf)
{
	if (lseek(k->fd, sector*k->sector_size, SEEK_SET) != 
	    sector*k->sector_size)
		return -1;
	
	if (read(k->fd, buf, k->sector_size) !=
	    k->sector_size)
		return -1;
	
	return 0;
}

static int WRITE_SECTOR(struct Kap_Btree* k, off_t sector, void* buf)
{
	if (lseek(k->fd, sector*k->sector_size, SEEK_SET) != 
	    sector*k->sector_size)
		return -1;
	
	if (write(k->fd, buf, k->sector_size) !=
	    k->sector_size)
		return -1;
	
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
		*out |= where[len--];
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
	if (k->btree->sector_size != tmp) return KAP_BTREE_CORRUPT;
		
	decode_offset(scan, &tmp, 4); scan += 4;
	if (k->btree->max_key_size != tmp) return KAP_BTREE_CORRUPT;
	
	decode_offset(scan, &tmp, 1); scan += 1;
	if (k->btree->tree_size != tmp) return KAP_BTREE_CORRUPT;
	
	decode_offset(scan, &tmp, 1); scan += 1;
	if (k->btree->leaf_size != tmp) return KAP_BTREE_CORRUPT;
	
	decode_offset(scan, &k->btree->size, 8); scan += 8;
	decode_offset(scan, &k->btree->root, 8); scan += 8;
	
	if (k->btree->root >= k->btree->size)
		return KAP_BTREE_CORRUPT;
	
	return 0;
}

/***************************************** Control btree parameters */

struct Kap_Btree* btree_init(void)
{
	struct Kap_Btree* btree = malloc(sizeof(struct Kap_Btree));
	if (!btree) return 0;
	
	assert (sizeof(struct Sector_Header) == 2);
	
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
	if (3+k->btree->leaf_size + k->btree->max_key_size > size)
		return ERANGE;
	if (2+2*k->btree->tree_size + k->btree->max_key_size > size)
		return ERANGE;
	
	k->btree->sector_size = size;
	return 0;
}

int kap_btree_set_maxkeysize(Kap k, ssize_t size)
{
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size < 2)
		return ERANGE;
	if (3+k->btree->leaf_size + size > k->btree->sector_size)
		return ERANGE;
	if (2+2*k->btree->tree_size + size > k->btree->sector_size)
		return ERANGE;
	
	k->btree->max_key_size = size;
	return 0;
}

int kap_btree_set_treesize(Kap k, short size)
{
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size < 1)
		return ERANGE;
	if (size > sizeof(off_t))
		return ERANGE;
	
	if (2+2*size + k->btree->max_key_size > k->btree->sector_size)
		return ERANGE;
	
	k->btree->tree_size = size;
	return 0;
}

int kap_btree_set_leafsize(Kap k, ssize_t size)
{
	if (!k->btree) return KAP_NO_BTREE;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size < 1)
		return ERANGE;
	if (size > 255)
		return ERANGE;
	
	if (2+size + k->btree->max_key_size > k->btree->sector_size)
		return ERANGE;
	
	k->btree->leaf_size = size;
	return 0;
}

/***************************************** Open the btree */

int kap_btree_open(Kap k, const char* dir, const char* prefix)
{
	ssize_t	got;
	char	buf[200];
	
	struct Sector_Header root;
	
	if (!k->btree) return 0;
	if (k->btree->fd != -1) return KAP_ALREADY_OPEN;
	
	k->btree->secta = malloc(k->btree->sector_size);
	k->btree->sectb = malloc(k->btree->sector_size);
	k->btree->sectc = malloc(k->btree->sector_size);
	
	if (!k->btree->secta || !k->btree->sectb || !k->btree->sectc)
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
		
		root.leaf = 1;
		root.count = 0;
		memset(k->btree->secta, 0, k->btree->sector_size);
		memcpy(k->btree->secta, &root, sizeof(struct Sector_Header));
		
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

int kap_btree_close(Kap k)
{
	if (k->btree->secta) free(k->btree->secta);
	if (k->btree->sectb) free(k->btree->sectb);
	if (k->btree->sectc) free(k->btree->sectc);
	
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
	
	memset(k->btree->sectc, 0, k->btree->sector_size);
	
	go = lseek(k->btree->fd, k->btree->sector_size*k->btree->size, SEEK_SET);
	if (go != k->btree->sector_size*k->btree->size)
		return 0;
	
	did = write(k->btree->fd, k->btree->sectc, k->btree->sector_size);
	if (did != k->btree->sector_size)
		return 0;
	
	k->btree->size++;
	encode_header(k, k->btree->sectc);
	
	if (WRITE_SECTOR(k->btree, 0, k->btree->sectc) != 0)
		return 0;
	
	return out;
}

/***************************************** Functions to parse the sector */

static int is_full(off_t x)
{
}

/***************************************** Btree insertion code */

/* Precondition: x is loaded in secta
 *               y is loaded in sectb
 *               sectc is available for overwrite
 *               off is the offset of the position in secta to insert the split
 * This method will split a full child y of x (which is not full) and insert
 * the new key into offset off in x.
 */
static int split_child(Kap k, off_t x, off_t y, unsigned char* off)
{
	int		count;
	unsigned char*	mid;
	
	off_t z = allocate_cell(k);
	if (z == 0) return KAP_NO_SPACE;
	
	memset(k->btree->sectc, 0, k->btree->sector_size);
	((struct Sector_Header*)k->btree->sectc)->leaf = 
		((struct Sector_Header*)k->btree->sectb)->leaf;
	
	mid = k->btree->sectb + k->btree->sector_size/2;
}

