/*  $Id: SmartPtr.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  SmartPtr.h - Inlined intrusive smart pointer implementation
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

#ifndef __JFA_ABI_SMART_PTR_H__
#define __JFA_ABI_SMART_PTR_H__

#include <cassert>
#include <jfa/ABI/Observers.h>

namespace JFA
{
namespace detail
{

/* A typical application of the observer is for automatic memory cleanup.
 */
class MemoryManaged : public Observable
{
 protected:
 	void not_observed() { delete this; }
 	
 public:
 	virtual ~MemoryManaged() { assert (get_observer_count() == 0); }
};

template <typename T>
class SmartPtr : protected Observer
{
 public:
 	T* get() const { return reinterpret_cast<T*>(m_obj); }
 	
 	T* operator -> () const
 	{
 		assert (m_obj);
 		return get();
 	}
 	
 	T& operator* () const
 	{
 		assert (m_obj);
 		return *get();
 	}
 	
 	SmartPtr(T* o = 0)
 	 : Observer(static_cast<T*>(o))
 	{ }
 	
 	// This should work for the copy constructor
 	template <typename O>
 	SmartPtr(SmartPtr<O> o)
 	 : Observer(static_cast<T*>(o.get()))
 	{ }
 	
 	template <typename O>
 	SmartPtr& operator = (O* o)
 	{
 		Observer::operator = (static_cast<T*>(o));
 		return *this;
 	}
 	
 	bool operator == (const SmartPtr& o) const
 	{ return get() == o.get(); }
 	
 	// We have expressly omitted an automatic conversion to T* to prevent
 	// arcane automatic conversions by the compiler.
};

} // detail
} // JFA

#endif
