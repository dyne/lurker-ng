/*  $Id: kap.c,v 1.12 2002-07-17 10:30:50 terpstra Exp $
 *  
 *  kap.c - Implementation of the non-layer methods.
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

#include <string.h>
#include <stdlib.h>

#ifdef HAVE_ERRNO_H 
# include <errno.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_IO_H
#include <io.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#ifdef HAVE_SYS_FCNTL_H
#include <sys/fcntl.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_GETTEXT
#include <libintl.h>
#else
#include "../intl/libgnuintl.h"
#endif

#ifdef HAVE_ASSERT_H
# include <assert.h> 
#endif

#ifndef assert
# define assert(x) do { if (!x) { printf("\nASSERT FAILURE: %s:%i: '%s'\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
#endif

#define _(String) gettext (String)
#define gettext_noop(String) (String)
#define N_(String) gettext_noop (String)

#if defined(HAVE_FCNTL_H) && defined(HAVE_F_GETLK)
#define USE_LOCK_FCNTL
#elif defined(HAVE_FLOCK)
#define USE_LOCK_FLOCK
#elif defined(HAVE_LOCKF)
#define USE_LOCK_LOCKF
#else
#warning Not locking KAP databases on open
#endif

#ifdef DMALLOC
# include <dmalloc.h>
#endif

const char* kap_strerror(int error)
{
	switch (error)
	{
	case KAP_OPT_INVALID:		return _("KAP Option invalid");
	case KAP_UNALIGNED_DATA:	return _("KAP Unaligned data");
	case KAP_NO_SPACE:		return _("KAP Out of disk space");
	case KAP_NO_BTREE:		return _("KAP Btree subsystem not initialized");
	case KAP_NO_APPEND:		return _("KAP Append subsystem not initialized");
	case KAP_BTREE_CORRUPT:		return _("KAP Btree file is corrupt");
	case KAP_APPEND_CORRUPT:	return _("KAP Append file is corrupt");
	case KAP_WRONG_RECORD_SIZE:	return _("KAP Append DB has wrong record size");
	case KAP_WRONG_SECTOR_SIZE:	return _("KAP Btree DB has wrong sector size");
	case KAP_WRONG_MAX_KEY_SIZE:	return _("KAP Btree DB has wrong max key size");
	case KAP_WRONG_TREE_SIZE:	return _("KAP Btree DB has wrong tree size");
	case KAP_WRONG_LEAF_SIZE:	return _("KAP Btree DB has wrong leaf size");
	case KAP_KEY_EXIST:		return _("KAP Key already exists");
	case KAP_ALREADY_OPEN:		return _("KAP DB already open");
	case KAP_NOT_OPEN:		return _("KAP DB not open");
	case KAP_NOT_FOUND:		return _("KAP Key does not exist");
	default:			return strerror(error);
	}
}

int kap_create(Kap* k, int flags)
{
	*k = malloc(sizeof(struct Kap_T));
	if (!*k) goto kap_create_error0;
	
	if ((flags & (KAP_WBUFFER|KAP_APPEND)) == KAP_WBUFFER)
		return KAP_OPT_INVALID;
	if ((flags & (KAP_WBUFFER|KAP_BTREE))  == KAP_WBUFFER)
		return KAP_OPT_INVALID;
	
	(*k)->btree   = 0;
	(*k)->append  = 0;
	(*k)->wbuffer = 0;
	(*k)->rbuffer = 0;
	
	if ((flags & KAP_BTREE) == KAP_BTREE)
	{
		(*k)->btree = btree_init();
		if (!(*k)->btree) goto kap_create_error1;
	}
	
	if ((flags & KAP_APPEND) == KAP_APPEND)
	{
		(*k)->append = append_init();
		if (!(*k)->append) goto kap_create_error2;
	}
	
	if ((flags & KAP_WBUFFER) == KAP_WBUFFER)
	{
		(*k)->wbuffer = wbuffer_init();
		if (!(*k)->wbuffer) goto kap_create_error3;
	}
	
	return 0;
	
kap_create_error3:
	if ((*k)->append) free((*k)->append);
	
kap_create_error2:
	if ((*k)->btree) free((*k)->btree);
	
kap_create_error1:
	free(*k);
	
kap_create_error0:
	return ENOMEM;
}

int kap_destroy(Kap k)
{
	int out, ret;
	out = 0;
	
	if ((ret = kap_close(k)) != 0) out = ret;
	
	if (k->btree)   free(k->btree);
	if (k->append)  free(k->append);
	if (k->wbuffer) free(k->wbuffer);
	
	free(k);
	
	return out;
}

int kap_open(Kap k, const char* dir, const char* prefix)
{
	int out;
	
	if (k->btree)
	{
		out = kap_btree_open(k, dir, prefix);
		if (out) goto kap_open_error0;
	}
	if (k->append)
	{
		out = kap_append_open(k, dir, prefix);
		if (out) goto kap_open_error1;
	}
	if (k->wbuffer)
	{
		out = kap_wbuffer_open(k, dir, prefix);
		if (out) goto kap_open_error2;
	}
	
	return 0;

kap_open_error2:
	if (k->append) kap_append_close(k);
	
kap_open_error1:
	if (k->btree)  kap_btree_close (k);
	
kap_open_error0:
	return out;
}

int kap_sync(Kap k)
{
	int out, ret;
	out = 0;
	
	if (k->wbuffer) if ((ret = kap_wbuffer_sync(k)) != 0) out = ret;
	if (k->append)  if ((ret = kap_append_sync (k)) != 0) out = ret;
	if (k->btree)   if ((ret = kap_btree_sync  (k)) != 0) out = ret;
	
	return out;
}

int kap_close(Kap k)
{
	int out, ret;
	out = 0;
	
	if (k->wbuffer) if ((ret = kap_wbuffer_close(k)) != 0) out = ret;
	if (k->append)  if ((ret = kap_append_close (k)) != 0) out = ret;
	if (k->btree)   if ((ret = kap_btree_close  (k)) != 0) out = ret;
	
	return out;
}

int kap_lock(int fd)
{
#ifdef USE_LOCK_FCNTL
	struct flock lck;
#endif
	
#ifdef USE_LOCK_FCNTL	
	memset (&lck, 0, sizeof (struct flock));
	lck.l_type = F_WRLCK;
	lck.l_whence = SEEK_CUR;
	/* leave range as current to eof */
	
	if (fcntl(fd, F_SETLKW, &lck) == -1)
		return -1;
#endif

#if defined(USE_LOCK_FLOCK)
	if (flock(fd, LOCK_EX) == -1)
		return -1;
#elif defined(USE_LOCK_LOCKF)
	if (lockf(fd, F_LOCK, 0) == -1)
		return -1;
#endif
	
	return 0;
}

int kap_unlock(int fd)
{
#ifdef USE_LOCK_FCNTL
	struct flock lck;
#endif
	
#ifdef USE_LOCK_FCNTL
	memset (&lck, 0, sizeof (struct flock));
	lck.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lck);
#endif

#if defined(USE_LOCK_FLOCK)
	flock(fd, LOCK_UN);
#elif defined(USE_LOCK_LOCKF)
	lockf(fd, F_ULOCK, 0);
#endif
	
	return 0;
}

int kap_write_full(int fd, const unsigned char* buf, size_t dat)
{
	while (dat > 0)
	{
		ssize_t did = write(fd, buf, dat);
		if (did <= 0 && errno != EAGAIN && errno != EINTR) return -1;
		
		if (did > 0)
		{
			dat -= did;
			buf += did;
		}
	}
	
	return 0;
}

int kap_read_full(int fd, unsigned char* buf, size_t dat)
{
	while (dat > 0)
	{
		ssize_t did = read(fd, buf, dat);
		if (did <= 0 && errno != EAGAIN && errno != EINTR) return -1;
		
		if (did > 0)
		{
			dat -= did;
			buf += did;
		}
	}
	
	return 0;
}

void kap_decode_offset(const unsigned char* where, off_t* out, short len)
{
	*out = 0;
	while (len)
	{
		*out <<= 8;
		*out |= where[--len];
	}
}

void kap_encode_offset(unsigned char* where, off_t rec, short len)
{
	while (len--)
	{
		*where++ = rec;
		rec >>= 8;
	}
}

int kap_kopen(Kap k, KRecord* kr, const char* key)
{
	int	out;
	ssize_t len;
	size_t	klen;
	unsigned char buf[256];
	
	if (!k->append) return KAP_NO_APPEND;
	
	if (k->wbuffer)
	{
		out = kap_wbuffer_flush(k, key);
		if (out) return out;
	}
	
	out = kap_btree_read(k, key, &buf[0], &len);
	if (out == KAP_NOT_FOUND)
	{	/* We will rewrite this on calls to write */
		memset(kr, 0, sizeof(KRecord));
		return 0;
	}
	
	if (out == 0)
	{
		klen = kap_decode_krecord(&buf[0], kr);
		assert (len == klen);
	}
	
	return out;
}

int kap_kclose(Kap k, KRecord* kr,  const char* key)
{	/* do nothing (but don't tell! - people should still call this!) */
	return 0;
}

int kap_kread(Kap k, const KRecord* kr, const char* key,
	size_t where, void* buf, size_t amt)
{
	return kap_append_read(k, kr, where, buf, amt);
}

int kap_kwrite(Kap k, KRecord* kr, const char* key,
	size_t where, void* buf, size_t amt)
{
	size_t records = kr->records;
	int out = kap_append_write(k, kr, where, buf, amt);
	
	if (out) return out;
	if (kr->records != records)
	{
		unsigned char krb[256];
		size_t len = kap_encode_krecord(&krb[0], kr);
		
		out = kap_btree_write(k, key, &krb[0], len);
	}
	
	return out;
}

struct AppendBack
{
	Kap		k;
	const char*	key;
	void*		data;
	size_t		len;
	size_t*		recs;
	int		out;
};

static int append_back(void* arg, const char* key, unsigned char* record, ssize_t* len)
{
	struct AppendBack* nfo = arg;
	KRecord	kr;
	size_t	klen;
	
	if (!strcmp(key, nfo->key))
	{
		klen = kap_decode_krecord(record, &kr);
		assert (klen == *len);
	}
	else
	{
		memset(&kr, 0, sizeof(KRecord));
	}
	
	nfo->out = kap_append_append(nfo->k, &kr, nfo->data, nfo->len);
	
	*len = kap_encode_krecord(record, &kr);
	*nfo->recs = kr.records;
	
	return 1;
}

int kap_append_real(Kap k, const char* key, void* data, size_t len,
	size_t* nout)
{
	struct AppendBack nfo;
	int out;
	
	/* Silently ignore attempts to write nothing */
	if (len == 0) return 0;
	
	nfo.k		= k;
	nfo.key		= key;
	nfo.data	= data;
	nfo.len		= len;
	nfo.recs	= nout;
	nfo.out		= 0;
	
	out = kap_btree_op(k, key, &append_back, &nfo);
	
	if (nfo.out) return nfo.out;
	return out;
}

int kap_append(Kap k, const char* key, void* data, size_t len)
{
	size_t	nill;
	int	out;
	
	if (k->wbuffer)
	{
		if (len == 1)
		{
			return kap_wbuffer_push(k, key, data);
		}
		else
		{	/* User has done defragmentation or write combine
			 * already -- trust them.
			 */
			out = kap_wbuffer_flush(k, key);
			if (out) return out;
			
			return kap_append_real(k, key, data, len, &nill);
		}
	}
	else
	{
		return kap_append_real(k, key, data, len, &nill);
	}
}
