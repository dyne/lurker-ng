/*  $Id: SimpleLockHolder.cpp,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  SimpleLockHolder.cpp - A dirty-buffer-always-in-RAM implementation
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

#include "../Exception.h"
#include "SimpleLockHolder.h"

namespace JFA
{
namespace detail
{

SimpleLockHolder::SimpleLockHolder(
	SimpleTransactionManager*	tm)
 : m_tm(tm), m_transaction_complete(false)
{ }
 
SimpleLockHolder::~SimpleLockHolder()
{
	// This never throws
	if (!m_transaction_complete) abort();
	
	assert (m_transaction_complete); // long so we give a good error message
	assert (m_contracts.empty());
	assert (m_locks.empty());
}

void SimpleLockHolder::abort() throw ()
{
	assert (!m_transaction_complete);
	m_transaction_complete = true;
	return m_tm->abort(this);
}

void SimpleLockHolder::commit() throw (trans_exception)
{
	assert (!m_transaction_complete);
	m_transaction_complete = true;
	return m_tm->commit(this);
}

void SimpleLockHolder::read_lock(LockI* lock) throw (trans_exception)
{
	// From what I can tell, hint insertion is O(1) if the hint points
	// immediately to the right of the eventual target
	
	// find first key not less than lock
	Locks::iterator i = m_locks.lower_bound(lock);
	
	if (i != m_locks.end() && *i == lock)
	{
		// We already have this puppy locked - noop
	}
	else
	{
		if (lock->use == LockI::WRITE_LOCKED)
		{
			THROW(write_locked_exception,
				"exclusive lock held by another transaction");
		}
		else
		{
			// We now lock this lock
			++lock->use;
			// Insert the lock into our map (using the hint)
			m_locks.insert(i, lock);
		}
	}
}

void SimpleLockHolder::write_lock(LockI* lock) throw (trans_exception)
{
	// From what I can tell, hint insertion is O(1) if the hint points
	// immediately to the right of the eventual target
	
	// find first key not less than lock
	Locks::iterator i = m_locks.lower_bound(lock);
	
	if (i != m_locks.end() && *i == lock)
	{
		// We already have this puppy locked
		if (lock->use > LockI::EXCLUSIVE_USE)
		{
			lock->use = LockI::WRITE_LOCKED;
		}
		else
		{
			THROW(read_locked_exception,
				"lock is not exclusively available");
		}
	}
	else
	{	// we do not have this locked
		if (lock->use == LockI::UNUSED)
		{
			m_locks.insert(i, lock);
			lock->use = LockI::WRITE_LOCKED;
		}
		else
		{
			THROW(read_locked_exception,
				"lock in not exclusively available");
		}
	}
}


} // detail
} // JFA
