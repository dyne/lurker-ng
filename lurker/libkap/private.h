/*  $Id: private.h,v 1.3 2002-07-04 18:33:03 terpstra Exp $
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

struct Kap_Btree*  btree_init(void);
struct Kap_Append* append_init(void);

int kap_btree_open(Kap k, const char* dir, const char* prefix);
int kap_btree_sync(Kap k);
int kap_btree_close(Kap k);

int kap_append_open(Kap k, const char* dir, const char* prefix);
int kap_append_sync(Kap k);
int kap_append_close(Kap k);

/* Convenient methods */
int kap_write_full(int fd, const unsigned char* buf, size_t dat);
int kap_read_full (int fd,       unsigned char* buf, size_t dat);

/* Encode offsets in portable manner */
void kap_decode_offset(const unsigned char* where, off_t* out, short len);
void kap_encode_offset(      unsigned char* where, off_t  rec, short len);

/* Kap databases only support a single reader/writer */
int kap_lock(int fd);
int kap_unlock(int fd);
