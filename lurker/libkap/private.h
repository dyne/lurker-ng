/*  $Id: private.h,v 1.2 2002-07-01 12:54:09 terpstra Exp $
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

struct Kap_Btree* btree_init(void);

int kap_btree_open(Kap k, const char* dir, const char* prefix);
int kap_btree_sync(Kap k);
int kap_btree_close(Kap k);
