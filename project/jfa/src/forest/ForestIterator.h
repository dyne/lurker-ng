/*  $Id: ForestIterator.h,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  ForestIterator.h - A merging iterator
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

#ifndef __JFA_FOREST_ITERATOR_H__
#define __JFA_FOREST_ITERATOR_H__

#include <jfa/ABI/ForestI.h>

#include "../StreamI.h"

namespace JFA
{
namespace detail
{

class ForestIterator : public ForestIteratorI
{
 private:
 	// Don't event think about it!
 	ForestIterator& operator = (const ForestIterator& o)
	{ assert(0); return *this; }
	
 protected:
 	struct Element
 	{
 		SmartPtr<IStreamI>	stream;	// the stream which feeds us
 		MemHandle<const data_t>	buffer;	// current sector
 		const data_t*		record;	// the position in the sector
 		data_t			common; // how much in common with out
 		Element*		min;	// optimization for linear-time
 						// sort. hehehehehe :-)
 		
 		Element() { }
 	};
 	
 	/* Invariant:
 	 *   j->min->key = min { k->key: m_elements.begin() <= k <= j }
 	 *   for: m_elements.begin() <= j < m_position
 	 *   
 	 *   Where the comparison is lexical on the keys, favouring the
 	 *   smallest vector in equality. Note that there is no "key" field.
 	 *   position points to the record which holds the front-coded key.
 	 */
 	Element*		m_elements;
 	Element*		m_position;
 	Element*		m_end;
 	data_t			m_keybuf[256]; // for storing the last key
 
 public:
 	~ForestIterator();
 	ForestIterator(const ForestIterator& o);
 	
 	template <class _Iterator>
 	ForestIterator(_Iterator begin, _Iterator end)
 	{
 		m_buf.buf = &m_keybuf[0];
 		
 		int records = 0;
 		_Iterator count = begin;
 		while (count != end) ++records;
 		
 		assert (records);
 		m_elements = new Element[records];
 		try
 		{
	 		for (i = 0; i < records; ++i, begin++)
	 		{
	 			m_elements[i].stream = *begin;
	 			m_elements[i].buffer = m_elements[i].stream->get_next();
	 			m_elements[i].record = m_elements[i].buffer.buf;
	 			m_elements[i].common = m_elements[i].record[1];
	 			m_elements[i].min    = 0;
	 		}
	 	}
	 	catch (...)
	 	{
	 		delete [] m_elements;
	 		throw;
	 	}
 		
 		// Setup the invariant
 		m_position = m_elements[0].min = &m_elements[0];
 		m_end = &m_elements[records];
 	}
 	
 	ForestIteratorI* clone() throw (trans_exception);
 	
 	Observer reference() const;
 	
 	// return the amount in common with the last key, -1 on eos
 	// we guarantee to give the maximal compression.
 	// therefore, you can detect a repeated record
 	//   if advance() == get_key().len
 	// we promise to return the key/data pair from the smallest queue first
 	int advance() throw (trans_exception);
};

} // detail
} // JFA

#endif
