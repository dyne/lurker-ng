/*  $Id: StreamI.h,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  StreamI.h - Interface for sector-by-sector iterators (not always contiguous)
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

#ifndef __JFA_STREAM_I_H__
#define __JFA_STREAM_I_H__

#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/MemHandles.h>

namespace JFA
{
namespace detail
{

class IStreamI : public MemoryManaged
{
 public:
 	// Returns a null buffer on eos.
 	virtual MemHandle<const data_t> get_next() throw (trans_exception) = 0;
 	
 	// Get another copy of the stream with the same position, but that
 	// will move independently.
 	virtual IStreamI* clone() const throw (trans_exception) = 0;
};

class OStreamI : public MemoryManaged
{
 public:
 	// Terminate the stream (required!) with a null buffer
 	virtual void put_next(MemBlock<const data_t> sect) throw (trans_exception) = 0;
};

} // detail
} // JFA

#endif
