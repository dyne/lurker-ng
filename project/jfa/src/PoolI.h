/*  $Id: PoolI.h,v 1.1.1.1 2003-01-04 21:13:56 terpstra Exp $
 *  
 *  PoolI.h - Interface for automatic sector-sized memory management
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

#ifndef __JFA_POOL_I_H__
#define __JFA_POOL_I_H__

#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/MemHandles.h>

namespace JFA
{
namespace detail
{

class PoolI : public MemoryManaged
{
 public:
 	virtual MemHandle<data_t>	get_a_sector_buffer() = 0;
};

} // detail
} // JFA

#endif
