/*  $Id: FreeList.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  FreeList.cpp - Linked-list free blocks layout implementation
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "FreeList.h"
#include "../PromiseKeeperI.h"
#include "../Exception.h"
#include "../VolumeI.h"
#include "../misc/intDeserialize.h"
#include "../misc/intSerialize.h"

// Why not be conservative? We have an 8k sector to put this in!
#define BYTES_PER_PTR	8

namespace JFA
{
namespace detail
{

/* !!! Someday I will need to write a better storage manager. This one is
 * rather lame. If two transactions need to allocate space, this list makes
 * one block the other.
 */

/* A free-list which uses the first sector for recording the head of a singly
 * linked list. The last link in the chain refers to the first past-the-end
 * sector.
 * 
 * The root of the tree is stored in the second sector. I don't keep it in
 * the first because then anything reading a tree would lock the system
 * from being able to allocate a new sector.
 */
class FreeList : public LayoutI
{
 protected:
 	SmartPtr<PromiseKeeperI>	m_keeper;
 	
 public:
 	FreeList(PromiseKeeperI* keeper)
 		throw (trans_exception);
 	
 	void        free (TransactionI* t, recordptr_t record) throw (trans_exception);
 	recordptr_t alloc(TransactionI* t)                     throw (trans_exception);
 	
 	void        set_root(TransactionI* t, recordptr_t root)throw (trans_exception);
 	recordptr_t get_root(TransactionI* t)                  throw (trans_exception);
};

FreeList::FreeList(PromiseKeeperI* keeper)
	throw (trans_exception)
 : m_keeper(keeper)
{
	VolumeHandle h = m_keeper->get_raw_volume();
	
	if (h.volume->get_end() < 2)
	{	// a new thing to manage!
		MemHandle<data_t> mem = m_keeper->get_a_sector_buffer();
		
		// First free sector is past-end
		intSerializeFast<BYTES_PER_PTR>(mem.buf, 1);
		h.volume->push_disk(0, mem);
		
		// There is no root
		intSerializeFast<BYTES_PER_PTR>(mem.buf, 0);
		h.volume->push_disk(1, mem);
	}
}

void FreeList::free(TransactionI* t, recordptr_t record)
	throw (trans_exception)
{
	// The the list-head and the sector being added
	MemHandle<data_t> first  = m_keeper->get_rw(t, 0);
	MemHandle<data_t> sector = m_keeper->get_rw(t, record);
	
	// Push it on the list
	recordptr_t next;
	intDeserialize  <BYTES_PER_PTR>(first .buf, next);
	
	// Make the changes
	intSerializeFast<BYTES_PER_PTR>(sector.buf, next);
	intSerializeFast<BYTES_PER_PTR>(first .buf, record);
}

recordptr_t FreeList::alloc(TransactionI* t)
	throw (trans_exception)
{
	// Find the first item on the free-list
	MemHandle<data_t> first = m_keeper->get_rw(t, 0);
	
	recordptr_t out;
	intDeserialize  <BYTES_PER_PTR>(first.buf, out);
	
	// Decide whether or not it is past-end
	MemHandle<const data_t> test = m_keeper->get_ro(t, out);
	
	if (test.buf == 0)
	{	// was past-end
		
		// Make sure the caller is going to write it and actually
		// expand the eof; this helps prevent sparse-files.
		m_keeper->get_rw(t, out);
		
		// was past-end - next sector is first free
		intSerializeFast<BYTES_PER_PTR>(first.buf, out+1);
	}
	else
	{	// not past-end, -> has a next ptr
		recordptr_t next;
		intDeserialize  <BYTES_PER_PTR>(test .buf, next);
		intSerializeFast<BYTES_PER_PTR>(first.buf, next);
	}
	
	return out;
}

void FreeList::set_root(TransactionI* t, recordptr_t root)
	throw (trans_exception)
{
	MemHandle<data_t> second = m_keeper->get_rw(t, 1);
	intSerializeFast<BYTES_PER_PTR>(second.buf, root);
}

recordptr_t FreeList::get_root(TransactionI* t)
	throw (trans_exception)
{
	recordptr_t out;
	MemHandle<const data_t> second = m_keeper->get_ro(t, 1);
	intDeserialize<BYTES_PER_PTR>(second.buf, out);
	return out;
}

LayoutI* FreeListFactory(PromiseKeeperI* keeper)
	throw (env_exception)
{
	try
	{
		return new FreeList(keeper);
	}
	catch (const trans_exception& e)
	{
		THROW(corrupt_exception,
			"Unable to initialize FreeList for volume");
	}
}

} // detail
} // JFA
