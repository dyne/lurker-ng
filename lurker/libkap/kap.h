/*  $Id: kap.h,v 1.5 2002-07-04 18:35:35 terpstra Exp $
 *  
 *  kap.h - Public interface to the kap database
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

#ifndef _LIBKAP_H
#define _LIBKAP_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define	KAP_FORMAT_MAJOR	1
#define	KAP_FORMAT_MINOR	0

#define	KAP_TOOLS		1

/* The opaque kap handle */
struct Kap_T;
typedef struct Kap_T* Kap;

/* A handle to a specific record */
typedef struct KRecord_T
{
	size_t	records;
	
	off_t	jumps[32];	/* Internal kap use only please */
} KRecord;

/* Flags which may be used to open kap */
#define	KAP_BTREE	1
#define KAP_APPEND	2
#define KAP_WBUFFER	4
#define KAP_RBUFFER	8
#define KAP_SLOPPY_SYNC	16

/* General use cases for kap */
#define KAP_ATOMIC	KAP_BTREE|KAP_APPEND|KAP_RBUFFER
#define KAP_FAST	KAP_BTREE|KAP_APPEND|KAP_WBUFFER|KAP_RBUFFER|KAP_SLOPPY_SYNC
#define KAP_SMALL	KAP_BTREE|KAP_APPEND

/* kap specific errno values */
#define KAP_OPT_INVALID		-1
#define KAP_UNALIGNED_DATA	-2
#define KAP_NO_SPACE		-3

#define KAP_NO_BTREE		-50
#define KAP_NO_APPEND		-51

#define KAP_BTREE_CORRUPT	-100
#define KAP_APPEND_CORRUPT	-101
#define KAP_WRONG_RECORD_SIZE	-102
#define KAP_WRONG_SECTOR_SIZE	-103
#define KAP_WRONG_MAX_KEY_SIZE	-104
#define KAP_WRONG_TREE_SIZE	-105
#define KAP_WRONG_LEAF_SIZE	-106
#define KAP_KEY_EXIST		-107

#define KAP_ALREADY_OPEN	-200
#define KAP_NOT_OPEN		-201
#define KAP_NOT_FOUND		-202



/****************************************** Gerenal KAP interace */



/* turn a kap error code into a human readable string */
const char* kap_strerror(int error);

/** Create a new kap subsystem.
 *  flags is the desired layer combinations and options of kap.
 *  Errors: ENOMEM, KAP_OPT_INVALID
 */
int	kap_create(Kap* k, int flags);

/** Destroy a kap subsystem.
 *  All resources are freed.
 *  Errors: all kap_sync() and close() errors
 */
int	kap_destroy(Kap k);

/** Open a kap subsystem.
 *  The files created will be dir/prefix.btree
 *                        and dir/prefix.append
 *  Errors: all open() errors, mmap errors(), ENOMEM, KAP_*_CORRUPT, 
 *          KAP_WRONG_RECORD_SIZE, KAP_WRONG_MAX_KEY_SIZE, 
 *          and KAP_ALREADY_OPEN
 */
int	kap_open(Kap k, const char* dir, const char* prefix);

/** Synchronize the kap subsystem.
 *  All write buffers are flushed to disk and mmaps synced.
 *  Even without KAP_SLOPPY_SYNC this may do work.
 *  Errors: all kap_append(), msync(), and fsync() errors
 */
int	kap_sync(Kap k);

/** Close the kap subsystem.
 *  Errors: all kap_sync()
 */
int	kap_close(Kap k);



/****************************************** Tune KAP parameters */



/** Set the desired sector size for layer btree.
 *  Precondition: called prior to open_kap.
 *  Default: 8k
 *  Errors: ERANGE, KAP_ALREADY_OPEN, KAP_NO_BTREE
 */
int	kap_btree_set_sectorsize(Kap k, ssize_t size);

/** Set the desired maximum key size for layer btree.
 *  Precondition: called prior to open_kap.
 *                should be << 10% of the sectorsize
 *  Default: 100 bytes
 *  Errors: ERANGE, KAP_ALREADY_OPEN, KAP_NO_BTREE
 */
int	kap_btree_set_maxkeysize(Kap k, ssize_t size);

/** Set the desired maximum number size a btree may obtain.
 *  Precondition: called prior to open_kap.
 *  The maximum size will be sectorsize*256^bytes
 *  Default: 3
 *  
 *  Note: If you want to exceed 2Gb tree size (~16Gb of email), 
 *    you will also need to define DISABLE_MMAP
 *  
 *  Errors: ERANGE, KAP_ALREADY_OPEN, KAP_NO_BTREE
 */
int	kap_btree_set_treesize(Kap k, short bytes);

/** Set the desired number of maximum bytes in the leaves. Must be < 255.
 *  Precondition: called prior to open_kap.
 *  Default: 164
 *  Errors: ERANGE, KAP_ALREADY_OPEN, KAP_NO_BTREE
 */
int	kap_btree_set_leafsize(Kap k, ssize_t bytes);

/** Set the desired data stored for each kap record.
 *  Precondition: called prior to open_kap.
 *  Default: 4 bytes
 *  Errors: ERANGE, KAP_ALREADY_OPEN, KAP_NO_APPEND
 */
int	kap_append_set_recordsize(Kap k, ssize_t size);



/****************************************** Btree specific DB calls */



/** Operate on a btree record.
 *  
 *  This function forms the heart of all our DB operations. It searches for
 *  the first key >= the search key. Then it passes this key, the associated
 *  data (len = -1 if end of tree), and the user data to the decide()
 *  callback. The decide callback may return 0, meaning no change, or 1,
 *  meaning create (or update) the record for which we originally searched
 *  with the new contents of record/len (respecting leaf_size).
 *  
 *  Note: the key, and record parameters to decide(...) are valid until
 *  the next call of kap_btree_op (or any wrapper). This means you CANNOT
 *  pass them to kap_btree_op for the next call.
 * 
 *  Be advised, that at this time, deletion is NOT possible.
 *  
 *  Errors: all write(), ERANGE, KAP_NO_BTREE, and KAP_NOT_OPEN
 */
int	kap_btree_op(Kap k, const char* key, 
	int (*decide)(void* arg, const char* key, unsigned char* record, ssize_t* len),
	void* arg);

/** Do a simple database read.
 *  Make sure that buf has _leafsize bytes available.
 *  This is a convenience method implemented on top of kap_btree_op.
 *  Errors: kap_btree_op(), KAP_NOT_FOUND
 */
int	kap_btree_read(Kap k, const char* key,
	unsigned char* buf, ssize_t* len);

/** Do a simple database write.
 *  This method will not overwrite an existing record.
 *  This is a convenience method implemented on top of kap_btree_op.
 *  Errors: kap_btree_op(), KAP_KEY_EXIST
 */
int	kap_btree_write(Kap k, const char* key,
	const unsigned char* buf, ssize_t len);

/** Do a database read for the first key which is > the search key.
 *  This will modifiy key with the new key name. Make sure that key has
 *  max_key_size storage available!
 *  This is useful for iterating through a btree.
 *  Errors: kap_btree_op(), KAP_NOT_FOUND (on end of tree)
 */
int	kap_btree_read_next(Kap k, char* key,
	unsigned char* buf, ssize_t* len);

/****************************************** Append specific DB calls */
/* Note: Obtaining a KRecord must be done somewhere else */



/** Write a block of records to a keyword.
 *  len and offset are in records, not bytes.
 *  Errors: all read(), write(), KAP_UNALIGNED_DATA, KAP_NO_APPEND,
 *  and KAP_NOT_OPEN
 */
int	kap_append_write(Kap k, KRecord* kr, 
	size_t where, void* buf, size_t amt);

/** Read a krecord.
 *  amt and where are in number of records; not bytes.
 *  Errors: read()
 */
int	kap_append_read(Kap k, const KRecord* kr, 
	size_t where, void* buf, size_t amt);

/** Write a block of records to an append record.
 *  len is the number of records, not bytes.
 *  Errors: all kap_append_write(...)
 */
int	kap_append_append(Kap k, KRecord* kr, 
	void* data, size_t len);

/** Return how many bytes of the KRecord need to be stored.
 */
int	kap_append_keyspace(const KRecord* kr);


/****************************************** Combined KAP calls */



/** Open a KRecord by keyword name.
 *  This requires both the keyword and append layers.
 *  Errors: all kap_btree_read(), and KAP_NO_APPEND
 */
int	kap_kopen(Kap k, KRecord* kr, const char* key);

/** Close a KRecord by keyword name.
 *  This requires only the append layer.
 */
int	kap_kclose(Kap k, KRecord* kr);

/** Read from a KRecord.
 *  This is simply an alias for kap_append_read. However, at some point this
 *  may change. So a stub function is provided.
 */
int	kap_kread(Kap k, const KRecord* kr, 
	off_t where, void* buf, size_t amt);

/** Write to a KRecord.
 *  This is simply an alias for kap_append_write. However, at some point this
 *  may change. So a stub function is provided.
 */
int	kap_kwrite(Kap k, KRecord* kr, 
	off_t where, void* buf, size_t amt);

/** Do a full-fledged kap append operation possibly using the write buffer.
 *  This is probably the interface most people will want to use.
 *  Errors: all kap_btree_op() and kap_append_append()
 */
int	kap_append(Kap k, const char* key, 
	void* data, size_t len);


#ifdef __cplusplus
}
#endif

#endif
