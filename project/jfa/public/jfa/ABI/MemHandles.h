/*  $Id: MemHandles.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  MemHandles.h - Inlined memory handle implementation
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

#ifndef __JFA_ABI_MEM_HANDLES_H__
#define __JFA_ABI_MEM_HANDLES_H__

#include <jfa/ABI/Observers.h>
#include <jfa/ABI/types.h>

namespace JFA
{

namespace detail
{

/* Use this as a PARAMETER to functions which WILL NOT KEEP IT only please.
 * NEVER use this as a local variable to receive a MemHandle.
 * You can use it as a local variable to memory on the stack however.
 * Prefer to pass this by value.
 * 
 * This is a template so we can enforce const memory buffers.
 */
template <class _Char>
struct MemBlock
{
 	_Char*		buf;
 	sectorlen_t	len;
 	
 	MemBlock(_Char* pBuf = 0, sectorlen_t pLen = 0)
 	 : buf(pBuf), len(pLen) { }
 	
 	// This is so that we can convert non-const to const
 	template <class _OChar>
 	MemBlock(const MemBlock<_OChar>& o)
 	 : buf(o.buf), len(o.len) { }
 	 
 	// No automatic conversion from std::string for fear of taking
 	// a reference to a temporary by accident somewhere
};

/* If you want to return some memory, or accept memory to keep, use this.
 * 
 * The trick is to hook the observer to something which will cleanup when
 * everyone is done with it. This could be an objects destructor, or some
 * memory broker, or something else entirely.
 * 
 * Prefer to pass this by reference to reduce ++observer--
 */
template <class _Char>
struct MemHandle : public MemBlock<_Char>
{
 protected:
	Observer observer;
	
 public:
 	MemHandle(_Char* pBuf, sectorlen_t pLen, const Observer& o)
 	 : MemBlock<_Char>(pBuf, pLen), observer(o) { }
 	
 	MemHandle(_Char* pBuf = 0, sectorlen_t pLen = 0, Observable* o = 0)
 	 : MemBlock<_Char>(pBuf, pLen), observer(o) { }
 	
 	template <class _OChar>
 	MemHandle(const MemHandle<_OChar>& o)
 	 : MemBlock<_Char>(o), observer(o.get_observer()) { }
 	
 	Observer get_observer() const { return observer; }
};


} // detail
} // JFA

#endif
