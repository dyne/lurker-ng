#ifndef __JFA_N_WAY_FRONT_DECODER_H__
#define __JFA_N_WAY_FRONT_DECODER_H__

#include "ForestIterator.h"
#include <algorithm>

/* This forms the heart of the Forest. The merge operation must be as fast
 * as possible since this is the bottle-neck for sequential reading and for
 * insertion. (Who ever heard of a CPU-bound database? ME!)
 */

namespace JFA
{
namespace detail
{

ForestIterator::~ForestIterator()
{
	delete [] m_elements;
}

/** Merge a set of sorted queues for our user (in linear time)
 *  Preconditions:
 *    The size of each queue roughly doubles for each queue.
 *    The record format is
 *     (<keylen> <repeatlen> [non-repeated-key-part] <datalen> [data])+ <null>
 */
int ForestIterator::advance()
	throw (trans_exception)
{
	// !!! Consider making a lot of these things local variables
	// and compare timing data
		
	// Note that m_position points at the queue which we last consumed
	
	// Test if we last consumed a non-existant queue.
	// If so, we are at end.
	if (m_position == m_end)
		return -1;
	
	// Why is the test for hitting sector end at the top of the function?
	// Because we don't want to modify state and then throw.
	if (m_position->record[0] == 0)
	{	// in here we can be slow; it is rare.
		
		// This is the only throwing function call we ever make:
		m_position->buffer = m_position->stream->get_next();
		m_position->record = m_position->buffer.buf;
 		
		if (m_position->record == 0)
		{	// even more rare! - end of queue
			--m_end;
			
			// Shift all the stuff over
			Element* x;
			for (x = m_position; x != m_end; ++x)
				*x = *(x+1);
			
			if (m_position == m_end)
			{
				// end of stream
				if (m_position == m_elements)
					return -1;
					
				// move back to a valid entry for processing
				// below. 
				// (harmless even though it was not consumed)
				--m_position;
			}
			else if (m_position == m_elements)
			{
				// Oops, we have to fix our base case now
				m_elements[0].min = &m_elements[0];
			}
		}
	}
	
	// Update the common counter for the last consumed queue now that
	// we have it's next entry.
	m_position->common = m_position->record[1];
	
	// Correct the compression for min
	// We have to do this because we guarantee optimal compression
	int commonm = m_position->min->common;
	int clen = std::min((data_t)m_key.len, *m_position->min->record);
	const data_t* minb = &m_position->min->record[
			commonm+2-m_position->min->record[1]];
	const data_t* keyb = &m_keybuf[commonm];
	while (*minb == *keyb && commonm < clen)
	{
		++commonm;
		++minb;
		++keyb;
	}
	m_position->min->common = commonm;
	
	// Are we on the first record? (base case)
	if (m_position == m_elements)
	{
		// No need to correct entry 0; it is always right.
		// (Not to mention we would crash if we used the
		//  non-base case code below)
		++m_position;
	}
	
	// Reposition m_position at the end
	while (m_position != m_end)
	{
		// Update the compression (the last key may have more
		// in common than the one before)
		int commonp = m_position->common;
		int clen = std::min((data_t)m_key.len, *m_position->record);
		const data_t* posb = &m_position->record[
			commonp+2-m_position->record[1]];
		const data_t* keyb = &m_keybuf[commonp];
		while (*posb == *keyb && commonp < clen)
		{
			++posb;
			++keyb;
			++commonp;
		}
		m_position->common = commonp;
		
		// Update the invariant
		// Logically, we are doing this:
		// 
		// if (m_position->key < (m_position-1)->min->key)
		//      m_position->min = m_position;
		// else m_position->min = (m_position-1)->min;
		                                                
		// So, we have to compare two strings, a helper method
		// would be nice, but I don't want the overhead
		//
		// To compare two keys, note that the one with the most
		// in common with the last key MUST be smallest. 
		// 
		// If they have the same in common, then we do a
		// string comparison from where they differ.
		//
		// If they are still equal, we prefer the leftmost.
		Element* min = (m_position-1)->min;
		if (m_position->common < min->common)
		{	// min has more in common, therefore it is less
			m_position->min = min;
		}
		else if (m_position->common > min->common)
		{ 	// position has more in common therefore it is less
			m_position->min = m_position;
		}
		else
		{	// string compare
			int x = min->common; // the commons are equal
			int len = std::min(*m_position->record, *min->record);
			// note: posb already points to the right place
			const data_t* minb = &min->record[x+2-min->record[1]];
			while (*posb == *minb && x < len)
			{
				++posb;
				++minb;
				++x;
			}
			
			if (*posb < *minb)
			{	// position is smaller
				m_position->min = m_position;
			}
			else if (*posb > *minb)
			{	// min is smaller
				m_position->min = min;
			}
			else if (len < *min->record)
			{	// min is longer therefore position is smaller
				m_position->min = m_position;
			}
			else if (len < *m_position->record)
			{	// min is shorter and therefore smaller
				m_position->min = min;
			}
			else
			{	// Christ! The strings are identical
				// Prefer smaller queue
				m_position->min = min;
			}
		}
		
		// Now that we fixed the invariant up, we can advance
		++m_position;
	}
	
	// m_position points to the end which means all the min
	// pointers are correct. Pop off the smallest.
	
	
	// Let's try and find the smallest by pointer comparison.
	while (1)
	{
		// Ok, move back one, note that m_position->min
		// is still valid even though we moved m_position
		// back a step (since we didn't change it).
		--m_position;
		
		// Is this the smallest?
		if (m_position->min == m_position)
		{
			// Consume this record. Note that this
			// invalidates all the min records for
			// m_position and higher. But, we will fix
			// those up when we hit the top of the loop
			// again.
			
			const data_t* record = m_position->record;
			
			// Extract the key length
			m_key.len = *record;
			++record;
			
			// Copy the non-common part into the keybuf
			
			// Skip over the common-part that is due
			// to merging and not the current queue
			record += 1 + (m_position->common - *record);
			
			// Do the copy (generally quite small)
			data_t x = m_position->common;
			while (x < m_key.len)
			{
				m_keybuf[x] = *record;
				++record;
				++x;
			}
			
			// Extract the data length
			m_data.len = *record;
			++record;
			
			// Why memcpy when we can simply point the
			// buffer in place. :-)
			m_data.buf = record;
			
			// Move the position past the data
			// Do we have to pull a new record?
			m_position->record = record + m_data.len;
			
			// We have this much in common
			return m_position->common;
		}
	}
}

ForestIterator::ForestIterator(const ForestIterator& o)
{
	long sz = o.m_end - o.m_elements;
	m_elements = new Element[sz]; // use a temporary to work around g++ bug
	m_position = m_elements + (o.m_position - o.m_elements);
	m_end      = m_elements + (o.m_end      - o.m_elements);
	
	::memcpy(&m_keybuf[0], &o.m_keybuf[0], sizeof(m_keybuf));
	
	Element* m;
	Element* om;
	for (m = m_elements, om = o.m_elements; m != m_end; ++m, ++om)
	{
		m->stream = om->stream->clone(); // seperate iterator state
		m->buffer = om->buffer; // double referencing is ok
		m->record = om->record; // we have a handle to the same place
		                        // so the buffer pointer is ok
		m->common = om->common; // same current position
		m->min    = m_elements + (om->min - o.m_elements);
	}
	
	// Since we have a shared buffer due to MemHandles, this is ok
	m_key  = o.m_key;
	m_data = o.m_data;
}

ForestIteratorI* ForestIterator::clone()
	throw (trans_exception)
{
	return new ForestIterator(*this);
}

Observer ForestIterator::reference() const
{
	// Note that m_position points at the queue which we last consumed
	
	// Test if we last consumed a non-existant queue.
	// If so, we are at end.
	if (m_position == m_end)
		return Observer(0);
	
	return m_position->buffer.get_observer();
}

} // detail
} // JFA

#endif
