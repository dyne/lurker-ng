/*  $Id: private.h,v 1.11 2002-08-25 15:59:12 terpstra Exp $
 *  
 *  private.h - Private internal interfaces for libkap
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

#include "kap.h"

struct Kap_Btree;
struct Kap_Append;
struct Kap_Wbuffer;
struct Kap_Rbuffer;

struct Kap_T
{
	struct Kap_Btree*	btree;
	struct Kap_Append*	append;
	struct Kap_Wbuffer*	wbuffer;
	struct Kap_Rbuffer*	rbuffer;
};

struct Kap_Btree* 	  btree_init(Kap k);
struct Kap_Append*	 append_init(Kap k);
struct Kap_Wbuffer*	wbuffer_init(Kap k);
struct Kap_Rbuffer*	rbuffer_init(Kap k);

int kap_btree_open(Kap k, const char* dir, const char* prefix);
int kap_btree_sync(Kap k);
int kap_btree_close(Kap k);

int kap_append_open(Kap k, const char* dir, const char* prefix);
int kap_append_sync(Kap k);
int kap_append_close(Kap k);

int kap_wbuffer_open(Kap k, const char* dir, const char* prefix);
int kap_wbuffer_sync(Kap k);
int kap_wbuffer_close(Kap k);

int kap_rbuffer_open(Kap k, const char* dir, const char* prefix);
int kap_rbuffer_sync(Kap k);
int kap_rbuffer_close(Kap k);

/* Convenient methods */
int kap_write_full(int fd, const unsigned char* buf, size_t dat);
int kap_read_full (int fd,       unsigned char* buf, size_t dat);

/* Don't use write buffers, and also report record count */
int kap_append_real(Kap k, const char* key, void* data, size_t len, size_t* out);

/* Protected methods for use by kap */
int kap_wbuffer_push (Kap k, const char* key, unsigned char* buf);
int kap_wbuffer_flush(Kap k, const char* key);

int kap_rbuffer_kopen (Kap k, const KRecord* kr);
int kap_rbuffer_kclose(Kap k, const KRecord* kr);

void kap_rbuffer_write(Kap k, const KRecord* kr, size_t offset, const void* out, size_t amount);
int kap_rbuffer_read(Kap k, const KRecord* kr, size_t offset, void* out, size_t amount);
int kap_rbuffer_find(Kap k, KRecord* kr,
	int (*testfn)(const void* arg, const void* rec), const void* arg,
	ssize_t* offset, void* rec);

/* Kap databases only support a single reader/writer */
int kap_lock(int fd);
int kap_unlock(int fd);
