/*  $Id: append.c,v 1.24 2002-08-27 17:15:09 terpstra Exp $
 *  
 *  append.c - Implementation of the append access methods.
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

/* #define DEBUG 1 */
/* #define PROFILE 1 */

/* If you need appends larger than 2Gb you must define this: */
/* #define DISABLE_MMAP 1 */

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

/* Now, how is this file laid out?
 * Why, simple I saw! It's simply a big ass flat file.
 * 
 * Now you ask, but how do we guarantee logarithm fragmentation?
 * I reply: oh, that ....
 * 
 * So, the basic plan is, allocate 1 record, and next time allocate double,
 * and next time, double that, ... This way a key which has say 100 records
 * will have allocated cells of size: 1, 2, 4, 8, 16, 32, 64. This will
 * total 127 records, so 27 will be wasted. The KRecord type stores the
 * offsets to these cells in the jumps[..] array. 
 * 
 * Also, on a block device, we allocate cells from the end of the file rather
 * from the start. This is so that we can coexist with a btree on the same
 * device. **** THIS IS DISABLED FOR NOW ***
 */


#define	GROW_DOWN	1
#define	GROW_UP		2

#define LIBKAP_APPEND_HEAD	"libkap append v0.1\n\377"
#define LIBKAP_APPEND_HEAD_LEN	21

#define	BLOCK_SIZE	8192
#define BLOCK_MASK	~0x2000

struct Kap_Append
{
	int	fd;
	int	grow_dir;
	size_t	record_size;
	off_t	eof;
	off_t	prealloc;
	
	off_t	cache_off;
	
	size_t		tail_size;
	unsigned char*	tail_cache;

#ifdef USE_MMAP
	char*	mmap;
#endif
};

#ifdef PROFILE

#define TRACK_DISTANCE		16384
#define	TRACK_SIZE		64

int appends[TRACK_DISTANCE][TRACK_SIZE];
int other_before;
int other_big;

#endif

/***************************************** Strategies for accessing the disk */

#ifdef USE_MMAP
static size_t round_mmap_up(size_t amt)
{
	size_t out = 0xC000; /* Can't quite map 2Gb- so use 110000... */
	while (out < amt) out <<= 1;
	
	if (sizeof(void*) == 4 && out > 512*1024*1024)
	{
		/* Intentionally fail; gre6edy is bad */
		return 0xFFFFFFFFUL;
	}
	
	return out;
}
#endif

static int WRITE_RECORDS(struct Kap_Append* k, off_t sector, const void* buf, size_t amount)
{
	off_t	cache_cut;
	size_t	cache_amt;

#if DEBUG	
	if (k->tail_cache) if (amount > 4) printf("B%d", amount);
#endif
	
	if (k->tail_cache &&
	    sector + amount > k->cache_off)
	{	/* Cache this write */
		if (sector > k->cache_off)
		{
			cache_cut = sector;
			cache_amt = amount;
		}
		else
		{
			cache_cut = k->cache_off;
			cache_amt = sector + amount - k->cache_off;
		}
		
		memcpy(	&k->tail_cache[cache_cut - k->cache_off],
			&((unsigned char*)buf)[cache_cut - sector],
			cache_amt);
		
#ifdef DEBUG
		if (sector + amount == k->eof)	printf("."); 
		else				printf(",");
		fflush(stdout);
#endif
		
		amount -= cache_amt;
	}
	
	if (amount == 0) return 0;
	
#ifdef DEBUG
	if (k->tail_cache)
	{
		if (sector + (k->tail_size*k->record_size) >= k->eof)
			printf("X");
		else	printf("x");
		fflush(stdout);
	}
#endif
	
#ifdef USE_MMAP
	if (k->mmap != MAP_FAILED)
	{
		memcpy(&k->mmap[sector], buf, amount);
		return 0;
	}
#endif
	
	if (lseek(k->fd, sector, SEEK_SET) != sector)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	if (kap_write_full(k->fd, buf, amount) != 0)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	return 0;
}

static int READ_RECORDS(struct Kap_Append* k, off_t sector, void* buf, size_t amount)
{
	off_t	cache_cut;
	size_t	cache_amt;
	
	if (k->tail_cache &&
	    sector + amount > k->cache_off)
	{	/* Read from cache */
		if (sector > k->cache_off)
		{
			cache_cut = sector;
			cache_amt = amount;
		}
		else
		{
			cache_cut = k->cache_off;
			cache_amt = sector + amount - k->cache_off;
		}
		
		memcpy(	&((unsigned char*)buf)[cache_cut - sector],
			&k->tail_cache[cache_cut - k->cache_off],
			cache_amt);
		
		amount -= cache_amt;
	}
	
	if (amount == 0) return 0;
	
#ifdef USE_MMAP
	if (k->mmap != MAP_FAILED)
	{
		memcpy(buf, &k->mmap[sector], amount);
		return 0;
	}
#endif
	
	if (lseek(k->fd, sector, SEEK_SET) != sector)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	if (kap_read_full(k->fd, buf, amount) != 0)
	{
		if (errno == 0) errno = EINTR;
		return -1;
	}
	
	return 0;
}

static int flush_tail(struct Kap_Append* k)
{
	unsigned char	out[sizeof(off_t)];
	size_t		amt;
	
	if (k->tail_cache)
	{
#ifdef DEBUG
		printf("F"); fflush(stdout);
#endif
		
		if (lseek(k->fd, k->cache_off, SEEK_SET) != k->cache_off)
		{
			if (errno == 0) errno = EINTR;
			return -1;
		}
		
		if (k->eof - k->cache_off > k->tail_size * k->record_size)
			amt = k->tail_size * k->record_size;
		else	amt = k->eof - k->cache_off;
		
		if (kap_write_full(k->fd, k->tail_cache, amt) != 0)
		{
			if (errno == 0) errno = EINTR;
			return -1;
		}
		
		k->cache_off = k->eof & BLOCK_MASK; /* 8k aligned */
		memset(k->tail_cache, 0, k->tail_size * k->record_size);
#ifdef DEBUG
		printf("\n\n\nCACHE OFF: %ld\n\n", k->cache_off);
#endif
	}
	
	kap_encode_offset(&out[0], k->eof, sizeof(off_t));
	if (WRITE_RECORDS(
		k, 
		LIBKAP_APPEND_HEAD_LEN,
		&out[0], 
		sizeof(off_t)) != 0)
	{
		return -1;
	}
	
	return 0;
}

/***************************************** Decoder methods */


/* Map a jump to it's length:
 * 	0 ->	1
 * 	1 ->	2
 * 	2 ->	4
 * 	3 ->	8
 * 	4 ->	16
 * 	5 ->	32
 * 	...
 */
static off_t jump_to_length(int jump)
{
	off_t out = 1;
	
	while (jump)
	{
		out <<= 1;
		jump--;
	}
	
	return out;
}

/* Map a jump to the first possible offset:
 *	0 ->	0
 *	1 ->	1	(1)
 *	2 ->	3	(2+1)
 *	3 ->	7	(4+2+1)
 *	4 ->	15	(8+4+2+1)
 *	5 ->	31	(16+8+4+2+1)
 *	...
 */
static off_t jump_to_offset(int jump)
{
	return jump_to_length(jump)-1;
}

/* Map an offset to a jump:
 * 	0	-> 0
 *	1-2	-> 1
 *	3-6	-> 2
 *	7-14	-> 3
 *	15-30	-> 4
 *	31-62	-> 5
 *	...
 */
static int offset_to_jump(off_t offset)
{
	int out = 0;
	
	offset++;
	while (offset != 1)
	{
		offset >>= 1;
		out++;
	}
	
	return out;
}

/* Allocate a cell appropriate for use in the the jump'th entry of a KRecord.
 * This storage is pulled from the EOF or the start depending on the value of
 * the grow_dir. Space is zero'd out to prevent sparse file behaviour.
 * 
 * 0 is returned on error.
 */
static off_t allocate_cell(Kap k, int jump)
{
	/* About allocated buffer */
	off_t	amt;
	off_t	where;
	
	/* About preallocated space */
	off_t	clear;
	off_t	remain;
	
	/* New official eof */
	off_t	eof;
#ifdef USE_MMAP
	void*	tmp;
#endif
	
	unsigned char	buf[BLOCK_SIZE];
	
	/* Calculate the buffer information */
	
	amt = jump_to_length(jump);
	amt *= k->append->record_size;
	
	where = k->append->eof;
	if (where + amt > k->append->prealloc)
	{
		clear = k->append->prealloc;
		remain = where + amt - k->append->prealloc;
		
		/* Align to buff size */
		remain += sizeof(buf) -1;
		remain -= remain % sizeof(buf);
		
		k->append->prealloc += remain;
	}
	else
	{	/* already zerod */
		clear = 0;
		remain = 0;
	}
	
	
	/* Write a whole whack of zeros to the file */
	
	if (remain)
	{
		if (lseek(k->append->fd, clear, SEEK_SET) != clear)
			return 0;
	
		memset(&buf[0], 0, sizeof(buf));
	}
	
	while (remain)
	{
		if (kap_write_full(k->append->fd, &buf[0], sizeof(buf)) != 0)
		{
			k->append->prealloc -= remain;
			return 0;
		}
		remain -= sizeof(buf);
	}
	
	eof = where + amt;
	
#ifdef	USE_MMAP
	if (round_mmap_up(k->append->eof) !=
	    round_mmap_up(eof))
	{
		tmp = mmap(
			0,
			round_mmap_up(eof),
			PROT_READ|PROT_WRITE,
			MAP_SHARED,
			k->append->fd,
			0);
		
		/* Allow us to run out of mmap space and fall back to normal
		 * disk IO.
		 */
		if (tmp == MAP_FAILED && errno != ENOMEM)
			return 0;
		
		if (k->append->mmap != MAP_FAILED && munmap(
			k->append->mmap,
			round_mmap_up(k->append->eof)) 
			!= 0)
		{
			if (tmp != MAP_FAILED) munmap(tmp, round_mmap_up(eof));
			return 0;
		}
		
		k->append->mmap = tmp;
	}
#endif
	
	/* Okay, we seem to have the storage, update the eof marker */
	k->append->eof = eof;
	
	if (!k->append->tail_cache ||
	    k->append->eof >= k->append->cache_off + 
	    		      k->append->tail_size * k->append->record_size)
	{
		if (flush_tail(k->append) != 0)
		{
			return 0;
		}
	}
	
	return where;
}

size_t kap_decode_krecord(const unsigned char* where, KRecord* kr)
{
	int recs, i;
	off_t tmp;
	
	kap_decode_offset(where, &tmp, 4);
	where += 4;
	kr->records = tmp;
	
	recs = offset_to_jump(kr->records-1);
	for (i = 0; i <= recs; i++)
	{
		kap_decode_offset(where, &kr->jumps[i], 5);
		where += 5;
	}
	
	return 4 + (recs+1)*5;
}

size_t kap_encode_krecord(unsigned char* where, const KRecord* kr)
{
	int recs, i;
	
	kap_encode_offset(where, kr->records, 4);
	where += 4;
	
	recs = offset_to_jump(kr->records-1);
	for (i = 0; i <= recs; i++)
	{
		kap_encode_offset(where, kr->jumps[i], 5);
		where += 5;
	}
	
	return 4 + (recs+1)*5;
}

int kap_append_read(Kap k, const KRecord* kr,
	size_t off, void* data, size_t len)
{
	int i, recs;
	
	if (!k->append) return KAP_NO_APPEND;
	if (k->append->fd == -1) return KAP_NOT_OPEN;
	
	/* We assume below that this is >= 1 */
	if (len == 0) return 0;
	
	recs = offset_to_jump(off+len-1); /* we need this cell */
	assert (off + len <= kr->records);
	
	/* Now, begin reading the data in */
	for (i = offset_to_jump(off); i <= recs; i++)
	{
		off_t start;	/* within buffer */
		off_t rstart;	/* within disk fragment */
		off_t amt;	/* amount in this pass */
		
		if (jump_to_offset(i) < off)
		{
			start  = 0;
			rstart = off - jump_to_offset(i);
		}
		else
		{
			start = jump_to_offset(i) - off;
			rstart = 0;
		}
		
		/* jump_to_offset(i+1) == jump_to_offset(i)+jump_to_length(i) */
		if (len+off > jump_to_offset(i+1))
		{
			amt = jump_to_length(i) - rstart;
		}
		else
		{
			amt = len - start;
		}
		
		if (READ_RECORDS(
			k->append,
			kr->jumps[i]+(rstart*k->append->record_size), 
			((unsigned char*)data)+(start*k->append->record_size),
			amt*k->append->record_size) != 0)
		{
			return errno;
		}
	}
	
	return 0;
}

int kap_append_write(Kap k, KRecord* kr,
	size_t off, void* data, size_t len)
{
	int i, recs;
	
	if (!k->append) return KAP_NO_APPEND;
	if (k->append->fd == -1) return KAP_NOT_OPEN;
	
	/* We assume below that this is >= 1 */
	if (len == 0) return 0;
	
	recs = offset_to_jump(off+len-1); /* we need this cell */
	
	if (kr->records == 0)	i = 0;
	else			i = offset_to_jump(kr->records-1)+1;
	
	for (; i <= recs; i++)
	{	/* for each cell we don't have */
		kr->jumps[i] = allocate_cell(k, i);
		if (kr->jumps[i] == 0)
			return errno;
	}
	
	if (off+len > kr->records)
	{	/* record new length */
		kr->records = off+len;
	}
	
	/* Now, begin writing the data out */
	for (i = offset_to_jump(off); i <= recs; i++)
	{
		off_t start;	/* within buffer */
		off_t rstart;	/* within disk fragment */
		off_t amt;	/* amount in this pass */
		
		off_t target_off;
#ifdef PROFILE
		int   distance;
		int   size;
#endif
				
		if (jump_to_offset(i) < off)
		{
			start  = 0;
			rstart = off - jump_to_offset(i);
		}
		else
		{
			start = jump_to_offset(i) - off;
			rstart = 0;
		}
		
		/* jump_to_offset(i+1) == jump_to_offset(i)+jump_to_length(i) */
		if (len+off > jump_to_offset(i+1))
		{
			amt = jump_to_length(i) - rstart;
		}
		else
		{
			amt = len - start;
		}
		
		target_off = kr->jumps[i] + (rstart*k->append->record_size);
		
#ifdef PROFILE
		distance = (k->append->prealloc - target_off) / k->append->record_size;
		size     = amt;
		if (size < TRACK_SIZE)
		{
			if (distance < TRACK_DISTANCE)
			{
				appends[distance][size]++;
			}
			else
			{
				other_before++;
			}
		}
		else
		{
			other_big++;
		}
#endif		
		if (WRITE_RECORDS(
			k->append,
			target_off, 
			((unsigned char*)data)+(start*k->append->record_size),
			amt*k->append->record_size) != 0)
		{
			return errno;
		}
	}
	
	if (k->rbuffer)
		kap_rbuffer_write(k, kr, off, data, len);
	
	return 0;
}

int kap_append_append(Kap k, KRecord* kr, void* data, size_t len)
{
	return kap_append_write(k, kr,
		kr->records, data, len);
}

/** Find the largest record which satisfies a given property.
 *  test is called with test(arg, record) to determine if record
 *  satisfies the property.
 */
int kap_append_find(
	Kap		k, 
	KRecord*	kr,
	int		(*testfn)(const void* arg, const void* rec),
	const void*	arg,
	ssize_t*	offset,
	void*		rec)
{
	int out;
	size_t	l, r, m;
	
	/* Begin binary searching the array.
	 * If there is a hit, it must be in the record: [0, count)
	 * Our invariant is that the hit (if it exists) is in [l, r)
	 *                       and r >= l
	 */
	l = 0;
	r = kr->records;
	
	/* While the range contains more than one record */
	while (r - l > 1)
	{
		m = (l+r)/2;
		
		out = kap_append_read(k, kr, m, rec, 1);
		if (out) return out;
		
		if (testfn(arg, rec))
		{	/* This object passes, so the largest with the
			 * property is >= m: [m, inf)
			 * Intersect with invariant [l, r) to get:
			 *   -> [m, r) 
			 */
			l = m;
		}
		else
		{	/* This object fails, so the largest with the
			 * property is < m: (-inf, m)
			 * Intersect with the invariant [l, r) to get:
			 *   -> [l, m)
			 */
			r = m;
		}
	}
	
	/* If the range is [l, l) = empty set.
	 * The invariant guarantees that if a hit exists it is in this range.
	 * Therefore there is no hit.
	 */
	if (r == l)
	{
		*offset = -1;
		return KAP_NOT_FOUND;
	}
	
	/* Ok, the range the answer lies in [l, r).
	 * r-l <= 1 since the while (r - l > 1) failed
	 * r>=l     by invariant
	 * r!=l     by if (r == l) failed
	 * Thus: r > l -> r-1>0 -> r-l>=1 and r-l<=1 -> r-l=1
	 * thus the answer lies in [l, l+1) = [l, l]
	 * Test to see that it satisfies the test!
	 */
	out = kap_append_read(k, kr, l, rec, 1);
	if (out) return out;
	
	if (testfn(arg, rec))
	{	/* Positive match */
		*offset = l;
	}
	else
	{	/* This is the only record it could have been, and it fails. */
		*offset = -1;
		
		assert(l == 0);
		return KAP_NOT_FOUND;
	}
	
	return 0;
}

struct Kap_Append* append_init(Kap k)
{
	struct Kap_Append* out = malloc(sizeof(struct Kap_Append));
	if (!out) return 0;
	
	out->fd          = -1;
	out->record_size = 4;
	
	out->tail_size	= 0;
	out->tail_cache	= 0;

#ifdef PROFILE
	memset(&appends, 0, sizeof(appends));
	other_big = 0;
	other_before = 0;
#endif
	
#ifdef USE_MMAP
	out->mmap = MAP_FAILED;
#endif
	
	return out;
}

int kap_append_sync(Kap k)
{
	if (flush_tail(k->append) != 0)
	{
		return errno;
	}
	
#ifdef USE_MMAP
	if (k->append->mmap != MAP_FAILED && msync(
		k->append->mmap,
		round_mmap_up(k->append->eof),
		MS_SYNC|MS_INVALIDATE) != 0)
	{
		return errno;
	}
#endif
		
	if (fsync(k->append->fd) != 0)
	{
		return errno;
	}
	
	return 0;
}

int kap_append_set_tail_cache(Kap k, ssize_t size)
{
	if (!k->append) return KAP_NO_APPEND;
	if (k->append->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size > 32*1024*1024) return ERANGE;
	if ((size * k->append->record_size) % BLOCK_SIZE != 0) return ERANGE;
	
	k->append->tail_size = size;
	return 0;
}

int kap_append_set_recordsize(Kap k, ssize_t size)
{
	if (!k->append) return KAP_NO_APPEND;
	if (k->append->fd != -1) return KAP_ALREADY_OPEN;
	
	if (size == 0) return ERANGE;
	if (size > 16384) return ERANGE;
	
	k->append->record_size = size;
	return 0;
}

int kap_append_get_recordsize(Kap k, ssize_t* size)
{
	if (!k->append) return KAP_NO_APPEND;
	
	*size = k->append->record_size;
	return 0;
}

int kap_append_close(Kap k)
{
	int out, ret;
#ifdef PROFILE
	FILE* f;
	int i, j;
#endif
	
	out = 0;
	
	if ((ret = kap_append_sync(k))        != 0) out = ret;
	if ((ret = kap_unlock(k->append->fd)) != 0) out = ret;
	if ((ret = close(k->append->fd))      != 0) out = ret;
	
	if (k->append->tail_cache)
		free(k->append->tail_cache);
	
#ifdef PROFILE
	f = fopen("/tmp/append.profile", "w");
	fprintf(f, "other = %d %d\n", other_big, other_before);
	
	for (i = 0; i < TRACK_DISTANCE; i++)
	{
		for (j = 0; j < TRACK_SIZE; j++)
		{
			fprintf(f, "%d ", appends[i][j]);
		}
		
		fprintf(f, "\n");
	}
	
	fclose(f);
#endif
	
#ifdef USE_MMAP
	if (k->append->mmap != MAP_FAILED)
		if ((ret = munmap(
			k->append->mmap,
			round_mmap_up(k->append->eof))) != 0)
			out = ret;
#endif
	
	return out;
}

int kap_append_open(Kap k, const char* dir, const char* prefix)
{
	int	init;
	char	buf[200];
	int	out;
	int	block_device;
	
	unsigned char	header[LIBKAP_APPEND_HEAD_LEN];
	unsigned char	ptr[sizeof(off_t)];
	
#ifdef HAVE_SYS_STAT_H
	struct stat sbuf;
#endif
	
	if (!k->append) return 0;
	if (k->append->fd != -1)
	{
		out = KAP_ALREADY_OPEN;
		goto kap_append_open_error0;
	}
	
	snprintf(&buf[0], sizeof(buf),
		"%s/%s.append", dir, prefix);
	
	k->append->fd = open(&buf[0], 
		O_BINARY | O_RDWR | O_CREAT,
		S_IREAD | S_IWRITE);
	if (k->append->fd == -1)
	{
		if (errno)	out = errno;
		else		out = ENOENT;
		goto kap_append_open_error0;
	}
	
	if (kap_lock(k->append->fd))
	{
		if (errno)	out = errno;
		else		out = EBUSY;
		goto kap_append_open_error1;
	}
	
	block_device = 0;
#ifdef HAVE_SYS_STAT_H
	if (fstat(k->append->fd, &sbuf) != 0)
	{
		if (errno)	out = errno;
		else		out = ENOENT;
		goto kap_append_open_error2;
	}
	
	block_device = S_ISBLK(sbuf.st_mode);
#endif
	
	init = 0;

	if (block_device)
	{
		assert(0);
	}
	else
	{
		k->append->grow_dir = GROW_UP;
		lseek(k->append->fd, 0, SEEK_SET);
		k->append->eof = LIBKAP_APPEND_HEAD_LEN+sizeof(off_t);
		
		if (kap_read_full(k->append->fd, &header[0], LIBKAP_APPEND_HEAD_LEN) != 0)
		{
			init = 1;
			lseek(k->append->fd, 0, SEEK_SET);
		}
		
		if (!init && memcmp(&header[0], LIBKAP_APPEND_HEAD, LIBKAP_APPEND_HEAD_LEN))
		{
			out = KAP_APPEND_CORRUPT;
			goto kap_append_open_error2;
		}
	}
	
	if (init)
	{
		if (kap_write_full(
			k->append->fd, 
			(const unsigned char*)LIBKAP_APPEND_HEAD, 
			LIBKAP_APPEND_HEAD_LEN) != 0)
		{
			out = KAP_APPEND_CORRUPT;
			goto kap_append_open_error2;
		}
		
		kap_encode_offset(&ptr[0], k->append->eof, sizeof(off_t));
		if (kap_write_full(k->append->fd, &ptr[0], sizeof(off_t)) != 0)
		{
			out = KAP_APPEND_CORRUPT;
			goto kap_append_open_error2;
		}
	}
	else
	{
		if (kap_read_full(k->append->fd, &ptr[0], sizeof(off_t)) != 0)
		{
			out = KAP_APPEND_CORRUPT;
			goto kap_append_open_error2;
		}
		
		kap_decode_offset(&ptr[0], &k->append->eof, sizeof(off_t));
	}
	
	k->append->prealloc = k->append->eof;
	
#ifdef USE_MMAP
	k->append->mmap = mmap(
		0,
		round_mmap_up(k->append->eof),
		PROT_READ|PROT_WRITE,
		MAP_SHARED,
		k->append->fd,
		0);
	/* If we cannot mmap for any other reason than no memory, abort */
	if (k->append->mmap == MAP_FAILED && errno != ENOMEM)
	{
		if (errno)	out = errno;
		else		out = ENOMEM;
		goto kap_append_open_error2;
	}
#endif
	
	if (k->append->tail_size)
	{
		k->append->tail_cache = calloc(k->append->tail_size, k->append->record_size);
		if (!k->append->tail_cache)
		{
			if (errno)	out = errno;
			else		out = ENOMEM;
			goto kap_append_open_error3;
		}
		
		k->append->cache_off = k->append->eof & BLOCK_MASK;
		
		if (lseek(k->append->fd, k->append->cache_off, SEEK_SET)
			!= k->append->cache_off)
		{
			if (errno == 0) errno = EINTR;
			return -1;
		}
		
		if (kap_write_full(
			k->append->fd,
			k->append->tail_cache, 
			k->append->eof - k->append->cache_off) != 0)
		{
			if (errno == 0) errno = EINTR;
			return -1;
		}
		
	}
	
	return 0;

kap_append_open_error3:
#ifdef USE_MMAP
	if (k->append->mmap != MAP_FAILED)
		munmap(k->append->mmap, round_mmap_up(k->append->eof));
#endif
	
kap_append_open_error2:
	kap_unlock(k->append->fd);

kap_append_open_error1:
	close(k->append->fd);
	k->append->fd = -1;
	
kap_append_open_error0:
	return out;
}
