/*  $Id: append.c,v 1.10 2002-07-11 16:38:14 terpstra Exp $
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
 * device.
 */


#define	GROW_DOWN	1
#define	GROW_UP		2

#define LIBKAP_APPEND_HEAD	"libkap append v0.1\n\377"
#define LIBKAP_APPEND_HEAD_LEN	21

struct Kap_Append
{
	int	fd;
	int	grow_dir;
	size_t	record_size;
	off_t	eof;
};

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
	off_t	amt;
	off_t	where;
	off_t	remain;
	off_t	eof;
	
	unsigned char	buf[4096];
	unsigned char	out[sizeof(off_t)];
	
	amt = jump_to_length(jump);
	amt *= k->append->record_size;
	
	where = k->append->eof;
	if (k->append->grow_dir == GROW_DOWN)
	{
		/*!!! TEST FOR OVERLAP HERE */
		if (where < amt) return 0;
		where -= amt;
	}
	
	if (lseek(k->append->fd, where, SEEK_SET) != where)
		return 0;
	
	/* Write a whole whack of zeros to the file */
	memset(&buf[0], 0, sizeof(buf));
	
	remain = amt;
	while (remain > sizeof(buf))
	{
		if (kap_write_full(k->append->fd, &buf[0], sizeof(buf)) != 0)
			return 0;
		remain -= sizeof(buf);
	}
	
	if (kap_write_full(k->append->fd, &buf[0], remain) != 0)
		return 0;
	
	/* Okay, we seem to have the storage, update the eof marker */
	if (k->append->grow_dir == GROW_DOWN)
	{
		lseek(k->append->fd, sizeof(off_t), SEEK_END);
		eof = where;
	}
	else
	{
		lseek(k->append->fd, LIBKAP_APPEND_HEAD_LEN, SEEK_SET);
		eof = where + amt;
	}
	
	kap_encode_offset(&out[0], eof, sizeof(off_t));
	if (kap_write_full(k->append->fd, &out[0], sizeof(off_t)) != 0)
		return 0;
		
	k->append->eof = eof;
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
		
		if (lseek(k->append->fd, 
			kr->jumps[i]+(rstart*k->append->record_size), 
			SEEK_SET)
			!= kr->jumps[i]+(rstart*k->append->record_size))
			return errno;
		
		if (kap_read_full(
			k->append->fd, 
			((unsigned char*)data)+(start*k->append->record_size),
			amt*k->append->record_size)
			!= 0)
			return errno;
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
		
		if (lseek(k->append->fd, 
			kr->jumps[i]+(rstart*k->append->record_size), 
			SEEK_SET)
			!= kr->jumps[i]+(rstart*k->append->record_size))
			return errno;
		
		if (kap_write_full(
			k->append->fd, 
			((unsigned char*)data)+(start*k->append->record_size),
			amt*k->append->record_size)
			!= 0)
			return errno;
	}
	
	return 0;
}

int kap_append_append(Kap k, KRecord* kr, void* data, size_t len)
{
	return kap_append_write(k, kr,
		kr->records, data, len);
}

struct Kap_Append* append_init(void)
{
	struct Kap_Append* out = malloc(sizeof(struct Kap_Append));
	if (!out) return 0;
	
	out->fd          = -1;
	out->record_size = 4;
	
	return out;
}

int kap_append_sync(Kap k)
{
	if (fsync(k->append->fd) != 0)
	{
		return errno;
	}
	
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
	
	out = 0;
	
	if ((ret = kap_append_sync(k))        != 0) out = ret;
	if ((ret = kap_unlock(k->append->fd)) != 0) out = ret;
	if ((ret = close(k->append->fd))      != 0) out = ret;
	
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
		k->append->grow_dir = GROW_DOWN;
		k->append->eof = lseek(k->append->fd, LIBKAP_APPEND_HEAD_LEN+sizeof(off_t), SEEK_END);
		
		if (kap_read_full(k->append->fd, &header[0], LIBKAP_APPEND_HEAD_LEN) != 0)
		{
			out = KAP_APPEND_CORRUPT;
			goto kap_append_open_error2;
		}

		if (memcmp(&header[0], LIBKAP_APPEND_HEAD, LIBKAP_APPEND_HEAD_LEN))
		{
			init = 1;
			lseek(k->append->fd, LIBKAP_APPEND_HEAD_LEN+sizeof(off_t), SEEK_END);
		}
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
	
	return 0;

kap_append_open_error2:
	kap_unlock(k->append->fd);

kap_append_open_error1:
	close(k->append->fd);
	k->append->fd = -1;
	
kap_append_open_error0:
	return out;
}
