/*  $Id: SimplePromiseKeeper.cpp,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  SimplePromiseKeeper.cpp - A dirty-buffer-always-in-RAM implementation 
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

#include "SimplePromiseKeeper.h"
#include "SimpleLockHolder.h"
#include "../Exception.h"

#define A_LOT_OF_SECTORS	128

namespace JFA
{
namespace detail
{

/* There are various states that a Promise can be in.
 * A Promise's state consists of usage, memory, and referencing.
 *
 * In the below table we have (referenced / unreferenced) in each cell
 * A + sign indicates that it is keeping the PromiseKeeper alive
 *
 *         unused     |      promised      |    write-cache   |
 *    ----------------+--------------------+------------------+---------
 *     float / lru    |   float+  / lru+   |  float+ / float+ | has memory
 *     float / vapour | past-end+ / float+ | impossible / imp | no memory
 * 
 * float    means that the promise is not tracked
 * lru      means that the promise is on the lru to have its memory freed
 * vapour   means that the promise should be erased
 * past-end means that the promise is just that a sector did not exist
 * impossible means ... kaboom!
 *
 *
 * Promises are on the LRU iff:
 *  1: They have memory
 *  2: They are not referenced
 *  3: They are not write-cache
 * 
 * They can be pushed on (from floating) in two places:
 *  (one cannot gain memory without also gaining a reference)
 *  not_observed sees memory and not write-cache (became unreferenced)
 *  release_promise on write cache and sees no references (always has memory)
 *
 * They can be pulled off:
 *  free_some_sectors removes their memory
 *  when they are brought into a new promise (new reference and/or write-cache)
 * 
 * -----
 * 
 * Promises evaporate iff:
 *  1: They have NO memory
 *  2: They are unreferenced
 *  2: They are unused
 *
 * This can happen when:
 *  not_observed sees no memory and unused (became unreferenced)
 *  free_some_sectors removes memory and sees unused (must be unreferened)
 *  release_promise reduces the usage and notices no memory
 */

SimplePromiseKeeper::Promise::Promise()
 : Observable(), keep_alive(0)
{
}

void SimplePromiseKeeper::Promise::not_observed()
{
	if (memory.buf == 0)
	{	// No memory.
		// Should we vapourize?
		if (use == LockI::UNUSED)
		{	// Yes, let's kill ourselves
			// Note we were: unused, no mem, ref'd
			// our keep_alive variable was NOT set so we are ok here.
			weak->m_promises.erase(sector);
			// Now we are vapour
		}
	}
	else
	{	// We have memory
		// Should we move to the lru?
		if (use != LockI::WRITE_LOCKED)
		{	// Yes we should.
			// Note we were: unused/promised, mem, ref'd
			weak->m_lru.push_back(sector);
			lru_position = --weak->m_lru.end();
		}
	}
}

SimplePromiseKeeper::ExclusiveLock::ExclusiveLock()
 : Observable(), owner(0)
{
}

void SimplePromiseKeeper::ExclusiveLock::not_observed()
{
	owner->m_locked = false;
	owner = 0;
}

SimplePromiseKeeper::SimplePromiseKeeper(
		SimpleTransactionManager*	manager,
		const SmartPtr<PoolI>&		pool, 
		VolumeI*			volume)
	throw (trans_exception)
 : m_manager(manager), m_pool(pool), m_volume(volume), m_held_sectors(0), m_locked(false)
{
	m_volid = m_volume->get_id();
	m_eov   = m_volume->get_end();
	
	m_manager->borne(m_volid, this);
}

SimplePromiseKeeper::~SimplePromiseKeeper()
{
	// Do our best to make sure it is synced
	try { sync(); }
	catch (...) { }
	
	// Since we are dying, then there are no handles to us.
	// This means that all the promises are in column unused.
	
	// There is a bad possibility however.
	// Some punk may still hold Observers on our Promises.
	// This would lead to a crash when he later tried to decrement.
	// So, let us assert that this is not the case just to be safe.
	
	Promises::const_iterator i;
	for (i = m_promises.begin(); i != m_promises.end(); ++i)
	{
		assert(i->second.get_observer_count() == 0);
		assert(i->second.use == LockI::UNUSED);
	}
	
	// This should be impossible since the handle would keep us alive.
	assert(!m_locked);
	
	m_manager->died(m_volid);
}

SimplePromiseKeeper::Promises::iterator
SimplePromiseKeeper::promise_builder(recordptr_t sect) throw (trans_exception)
{
	if (m_locked)
		THROW(write_locked_exception, "volume exclusively write locked");
	
	// If we are holding a lot of sectors in RAM, we should try and
	// free some up before grabbing more.
	if (m_held_sectors >= A_LOT_OF_SECTORS)
		free_some_sectors();
	
	// From what I can tell, hint insertion is O(1) if the hint points
	// immediately to the right of the eventual target
	
	// find first key not less than sect
	Promises::iterator i = m_promises.lower_bound(sect);
	
	if (i == m_promises.end() || i->first != sect)
	{	// the record is not in there, let's load it
		MemHandle<data_t> h;
		
		if (sect < m_eov)
		{
			// Note: the throwing operation happens before
			// any potential state changes
			h = m_pool->get_a_sector_buffer();
			m_volume->pull_disk(sect, h);
			++m_held_sectors;
		}
		
		i = m_promises.insert(i, Promises::value_type(
			sect, Promise()));
		
		// setup the promise
		i->second.memory = h;
		i->second.use    = LockI::UNUSED; // will be changed
		i->second.weak   = this;
		i->second.sector = sect;
		
		i->second.lru_position = m_lru.end();
	}
	else if (i->second.memory.buf == 0 && sect < m_eov)
	{	// needs memory pulled in
		// Note: the throwing operation happens before
		// any potential state changes
		MemHandle<data_t> h = m_pool->get_a_sector_buffer();
		m_volume->pull_disk(sect, h);
		++m_held_sectors;
		i->second.memory = h;
	}
	
	// Make sure the promise will keep us alive
	i->second.keep_alive = this;
	
	if (i->second.lru_position != m_lru.end())
	{	// Get it off the lru
		m_lru.erase(i->second.lru_position);
		i->second.lru_position = m_lru.end();
	}
	
	return i;
}

void SimplePromiseKeeper::release_promise(recordptr_t sect) throw ()
{
	Promises::iterator i = m_promises.find(sect);
	if (i->second.use > LockI::EXCLUSIVE_USE)
	{
		--i->second.use;
	}
	else
	{
		i->second.use = LockI::UNUSED;
		
		// This promise just moved in the table...
		SmartPtr<SimplePromiseKeeper> self = this; // keep-alive
		i->second.keep_alive = 0; // no longer keep us alive
		
		if (i->second.memory.buf == 0)
		{	// No memory eh?
			// Was either past-end or a floating lock
			if (i->second.get_observer_count() == 0)
			{	// was a floating lock
				m_promises.erase(i);
			}
			// else we have to let it hang around till unref'd
		}
		else
		{	// We had memory, and were not unused.
			// This means we were either float+ or lru+
			// We are to become float or lru
			
			if (i->second.get_observer_count() == 0 &&
			    i->second.lru_position == m_lru.end())
			{	// Only to LRU if no observers and not on it
				m_lru.push_back(i->second.sector);
				i->second.lru_position = --m_lru.end();
			}
		}
	}
}

void SimplePromiseKeeper::commit_promise(recordptr_t sect)
	throw (trans_exception)
{
	Promises::iterator i = m_promises.find(sect);
	if (i->second.use == LockI::WRITE_LOCKED)
	{
		if (sect >= m_eov) m_eov = sect+1;
		m_volume->push_disk(sect, i->second.memory);
	}
}

void SimplePromiseKeeper::free_some_sectors()
{	// We are using an awful lot of memory on loaded blocks, so maybe
	// we should try releasing some RAM!
	
	// Anything on the lru is unref'd for sure
	// It may or may not be used.
	// It definitely has RAM
	
	if  (!m_lru.empty())
	{
		recordptr_t r = m_lru.front();
		m_lru.pop_front();
		
		Promises::iterator i = m_promises.find(r);
		// assert (i != m_promises.end());
		// assert (i->second.memory.buf != 0);
		// assert (i->second.get_observer_count() == 0);
		
		// We just popped it out of the lru, so kill the position.
		i->second.lru_position = m_lru.end();
		
		// Free it's memory
		i->second.memory = MemHandle<data_t>(0, 0, 0);
		--m_held_sectors;
		
		if (i->second.use == LockI::UNUSED)
		{
			// This was a promise sitting around holding on to
			// RAM in the hopes that someone would read it again.
			
			// It did not have a keep_alive reference (unused)
			// so this is safe.
			m_promises.erase(i);
		}
	}
}

MemHandle<const data_t> SimplePromiseKeeper::get_ro(TransactionI* t, recordptr_t sect) throw (trans_exception)
{
	SimpleLockHolder* trans = reinterpret_cast<SimpleLockHolder*>(t);
	assert (trans != 0);
	assert (!trans->m_transaction_complete);
	
	SimpleLockHolder::Contracts::value_type what(this, sect);
	Promises::iterator i = promise_builder(sect);
	
	// If the transaction already has it locked
 	if (trans->m_contracts.find(what) != trans->m_contracts.end())
		return MemHandle<const data_t>(
			i->second.memory.buf, i->second.memory.len, &i->second);
	
	// Is this record write-locked? And not by trans?
	if (i->second.use == LockI::WRITE_LOCKED)
	{
		THROW(write_locked_exception, 
			"sector in use by another transaction");
	}
	
	// Add another transaction reader to this sector
	++i->second.use;
	// Add this sector to the list of things locked by transaction
	trans->m_contracts.insert(what);
	
	return MemHandle<const data_t>(
		i->second.memory.buf, i->second.memory.len, &i->second);
}

MemHandle<data_t> SimplePromiseKeeper::get_rw(TransactionI* t, recordptr_t sect) throw (trans_exception)
{
	SimpleLockHolder* trans = reinterpret_cast<SimpleLockHolder*>(t);
	assert (trans != 0);
	assert (!trans->m_transaction_complete);
	
	SimpleLockHolder::Contracts::value_type what(this, sect);
	Promises::iterator i = promise_builder(sect);
	
	if (i->second.memory.buf == 0)
	{	// On rw past end, we will provide a buffer
		i->second.memory = m_pool->get_a_sector_buffer();
		++m_held_sectors;
	}
	
	// If unused, write lock it for the transaction
	if (i->second.use == LockI::UNUSED)
	{
		i->second.use = LockI::WRITE_LOCKED;
		trans->m_contracts.insert(what);
	}
	
	// If the transaction already holds a lock
	if (trans->m_contracts.find(what) != trans->m_contracts.end())
	{
		// Do they already have exclusive access?
		if (i->second.use == LockI::EXCLUSIVE_USE)
			i->second.use = LockI::WRITE_LOCKED; // write lock it
		
		// Do they already control the write lock?
		if (i->second.use == LockI::WRITE_LOCKED)
			return MemHandle<data_t>(
				i->second.memory.buf, i->second.memory.len, 
				&i->second);
		
		THROW(read_locked_exception, 
			"sector in use by another transaction");
	}
	
	// It is in use, and not by the transaction
	if (i->second.use == LockI::WRITE_LOCKED)
		THROW(write_locked_exception, "sector in use by another transaction");
	else
		THROW(read_locked_exception, "sector in use by another transaction");
}

void SimplePromiseKeeper::truncate(TransactionI* t, recordptr_t sector) throw (trans_exception)
{
	SimpleLockHolder* trans = reinterpret_cast<SimpleLockHolder*>(t);
	assert (trans != 0);
	assert (!trans->m_transaction_complete);
	
	// !!! ewwww... hard
	
	// do nothing for now; bad me.
}

VolumeHandle SimplePromiseKeeper::get_raw_volume() throw (trans_exception)
{
	if (!m_promises.empty())
		THROW(read_locked_exception, "volume read locked by a transaction");
	
	if (m_locked)
		THROW(write_locked_exception, "volume exclusively write locked");
	
	m_locked = true;
	m_exclusive.owner = this;
	
	return VolumeHandle(&m_exclusive, m_volume);
}

MemHandle<data_t> SimplePromiseKeeper::get_a_sector_buffer()
{
	return m_pool->get_a_sector_buffer();
}

sectorlen_t SimplePromiseKeeper::get_sector_size() const throw ()
{
	return m_volume->get_sector_size();
}

void SimplePromiseKeeper::sync() throw (trans_exception)
{
	return m_volume->sync();
}

} // detail
} // JFA
