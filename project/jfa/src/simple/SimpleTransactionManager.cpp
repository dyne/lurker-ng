/*  $Id: SimpleTransactionManager.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  SimpleTransactionManager.cpp - A dirty-buffer-always-in-RAM implementation 
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

#include "SimpleTransactionManager.h"
#include "SimpleLockHolder.h"
#include "SimplePromiseKeeper.h"

namespace JFA
{
namespace detail
{

SimpleTransactionManager::SimpleTransactionManager(
		VolumeManagerI*	vm,
		PoolI*		pool)
	throw (env_exception)
 : m_vm(vm), m_pool(pool)
{
	m_journal = m_vm->get_volume("journal");
}

void SimpleTransactionManager::abort(SimpleLockHolder* trans) 
	throw ()
{
	// Walk the list backwards. Why? This probably means
	// we will keep sector cache for earlier sectors in 
	// preference to later sectors. A good thing IMO.
	SimpleLockHolder::Contracts::reverse_iterator contract;
	for (contract  = trans->m_contracts.rbegin(); 
	     contract != trans->m_contracts.rend();
	     ++contract)
	{
		contract->first->release_promise(contract->second);
	}
	
	trans->m_contracts.clear();
	
	SimpleLockHolder::Locks::iterator lock;
	for (lock  = trans->m_locks.begin();
	     lock != trans->m_locks.end();
	     ++lock)
	{
		if ((*lock)->use > LockI::EXCLUSIVE_USE)
		{
			--(*lock)->use;
		}
		else
		{
			(*lock)->use = LockI::UNUSED;
			(*lock)->release();
		}
	}
	
	trans->m_locks.clear();
}

void SimpleTransactionManager::commit(SimpleLockHolder* trans)
	throw (trans_exception)
{
	try
	{
		//!!! Write to journal
		m_journal->sync();
		
		SimpleLockHolder::Contracts::iterator contract;
		for (contract  = trans->m_contracts.begin(); 
		     contract != trans->m_contracts.end();
		     ++contract)
		{
			contract->first->commit_promise(contract->second);
		}
		
		//!!! If journal is quite full, sync all volumes and truncate it
	}
	catch (...)
	{
		abort(trans);
		throw;
	}
	
	// Release internal locks
	SimpleLockHolder::Locks::iterator lock;
	for (lock  = trans->m_locks.begin();
	     lock != trans->m_locks.end();
	     ++lock)
	{
		if ((*lock)->use == LockI::WRITE_LOCKED)
		{
			// can never throw
			(*lock)->commit();
		}
	}
		
	// This is to release the resources on successful commit
	abort(trans);
}

void SimpleTransactionManager::borne(volume_t id, SimplePromiseKeeper* keeper)
{
	m_keepers.insert(Keepers::value_type(id, keeper));
}

void SimpleTransactionManager::died(volume_t id)
{
	m_keepers.erase(id);
}

SmartPtr<PromiseKeeperI> SimpleTransactionManager::open_volume(const std::string& name)
	throw (env_exception)
{
	VolumeI* vol = m_vm->get_volume(name);
	
	Keepers::iterator i = m_keepers.find(vol->get_id());
	if (i != m_keepers.end())
		return i->second;
	
	return new SimplePromiseKeeper(this, m_pool, vol);
}

LockHolderI* SimpleTransactionManager::open_transaction()
	throw (trans_exception)
{
	return new SimpleLockHolder(this);
}

TransactionManagerI* SimpleTransactionManagerFactory(
		VolumeManagerI*	vm,
		PoolI*		pool) 
	throw (env_exception)
{
	return new SimpleTransactionManager(vm, pool);
}

} // detail
} // JFA
