/*  $Id: Streamer.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  Streamer.h - Basic serialization hooks; can plug into another package
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

#ifndef __JFA_STREAMER_H__
#define __JFA_STREAMER_H__

#include "JFA.h"

namespace
{

template <class T>
struct Streamer;
// enum { size = fixed_length_of_serialized_data }; -1 if variable
//
// // Serialize the object to target
// sectorlen_t serialize(MemBlock<data_t> target, const T& object);
//
// // Deserialize the source into the return type
// T deserialize(MemBlock(const data_t> source);

struct Streamer<bool>
{
	enum { size = 1 };
	
	sectorlen_t serialize(MemBlock<data_t> target)
};

}

#endif
