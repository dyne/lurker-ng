/*  $Id: ForestI.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  ForestI.h - Interface into the library for Forest structures
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

#ifndef __JFA_ABI_FOREST_I_H__
#define __JFA_ABI_FOREST_I_H__

#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/MemHandles.h>
#include <jfa/ABI/TransactionI.h>
#include <jfa/ABI/EnvironmentI.h>

/* Considerations regarding the tree
 * 
 * Only two cases:
 *  map
 *  multimap
 * 
 * multiset = multimap with "" for data (converse is not true; half-mutability)
 * set      = map      with "" for data
 *
 * However, we can optimize set/multiset by having no datalen field
 *
 * map/multiset:
 *  reading:
 *   map:      merge and ignore larger trees.
 *   multimap: merge.
 *  inserting:
 *   map:      just add it.
 *   multimap: just add it.
 *  deleting:
 *   map      by key: set some deleted bit? (high bit of repeated?)
 *   multimap by ref: set some deleted bit? (high bit of repeated?)
 *  overwriting (value only):
 *   map      by key: just insert it.
 *   multiset by ref: delete+insert. (otherwise alignment gets affected)
 *  references:
 *   map      needs to worry about things added by key - Session proxy map?
 *   multiset needs to worry about things getting deleted
 */

namespace JFA
{
namespace detail
{

// This is not a pure interface since it partially specifies the
// implementation. However, It really sucks to have to make virtual method
// calls to read these things! 
class ForestIteratorI : public MemoryManaged
{
 protected:
 	MemBlock<const data_t>  m_data;     // the data for the last record
 	MemBlock<const data_t>  m_key;      // the key  for the last record  
 	
 public: 
 	// All memory is good as long as the iterator is held and not advanced.
 	// Alternately, getting an observer via reference() will also keep
 	// both buffers alive.
 	MemBlock<const data_t> get_key  () const { return m_key;  }
 	MemBlock<const data_t> get_value() const { return m_data; }
 	
 	virtual Observer reference() const = 0;
 	
 	// Clone the iterator.
 	// The iterator will point to the same location, but have an
 	// independent read position.
 	virtual ForestIteratorI* clone() throw (trans_exception) = 0;
 	
 	// Advance the iterator one position.
 	// Returned is the length of the largest common prefix between the 
 	// last key and the key key. -1 on end of stream.
 	virtual int advance() throw (trans_exception) = 0;
 	
 	// Sorry that this is a virtual method. :-(
 	// I tried really hard to get the guts out in the open in a meaningful
 	// way, but couldn't. !!! Maybe in version v1.1.
};

class ForestI : public MemoryManaged
{
 public:
 	// Insert a record into the database
 	virtual void insert(TransactionI* trans,
 	                    MemBlock<const data_t> key,
 	                    MemBlock<const data_t> data)
 		throw (trans_exception) = 0;
 	
 	// Get an iterator that is pointing at the first record with a
 	// key >= the parameter.
 	virtual ForestIteratorI* find(TransactionI* trans, MemBlock<const data_t> key)
 		throw (trans_exception) = 0;
};

SmartPtr<ForestI> ForestFactory(EnvironmentI* env, const std::string& name)
	throw (env_exception);

} // detail
} // JFA

#endif
