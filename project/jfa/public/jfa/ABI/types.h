/*  $Id: types.h,v 1.1.1.1 2003-01-04 21:13:59 terpstra Exp $
 *  
 *  types.h - Those typedefs we use everywhere
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: GPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; version 2.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __JFA_ABI_TYPES_H__
#define __JFA_ABI_TYPES_H__

#include <jfa/ABI/StaticAssert.h>

namespace JFA
{
namespace detail
{

typedef signed   long recordptr_t; // this bounds the number of sectors
typedef unsigned int  sectorlen_t; // hehe - sectors > 64k? damn straight!
typedef unsigned char data_t;

static const recordptr_t recordptr_max = 0x7FFFFFFFUL;

JFA_STATIC_ASSERT(sizeof(recordptr_t) >= 4);
JFA_STATIC_ASSERT(sizeof(sectorlen_t) > 2);
JFA_STATIC_ASSERT(sizeof(data_t) == 1);

} // detail
} // JFA

#endif
