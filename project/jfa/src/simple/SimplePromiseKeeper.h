/*  $Id: SimplePromiseKeeper.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  SimplePromiseKeeper.h - A dirty-buffer-always-in-RAM implementation 
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

#ifndef __JFA_SIMPLE_PROMISE_KEEPER_H__
#define __JFA_SIMPLE_PROMISE_KEEPER_H__

#include "SimpleTransactionManager.h"
#include "../PoolI.h"
#include "../VolumeI.h"

#include <map>
#include <list>

namespace JFA
{
namespace detail
{

class SimplePromiseKeeper : public PromiseKeeperI
{
 private:
 	/* So, what are we planning on doing?
 	 * 
 	 * Well, since we are an atomic database, we need to keep promises.
 	 * 
 	 * One of the promises is that only one person can be touching a
 	 * given sector at once if it is going to write to it.
 	 * 
 	 * The other promise is that a sector never changes from when a 
 	 * transaction first touches it till the transaction commits/aborts.
 	 * 
 	 * Finally, we are also charged with the mundane task of making sure
 	 * that the memory for the sectors is valid as long as people are
 	 * holding it.
 	 * 
 	 * Oh, and if some wacko does a read of a 12Gb database in one
 	 * transaction we can't just keep all the sectors in RAM. If he
 	 * tries to write the whole thing though; God have mercy, etc, etc.
 	 * 
 	 * So, how will we accomplish these feats? 
 	 * 
 	 * Well, fortunately we have this nice concept of observers. That
 	 * will let us know when no one is looking at the memory sector
 	 * anymore. We still can't forget about the sector until the
 	 * transaction ends because that would break our promise to make
 	 * sure it is unchanging. We can reclaim the sector memory however.
 	 * 
 	 * As for the promises we make to transactions, well we look after
 	 * that with the 'use' counter in our PromiseKeepers.
 	 * 
 	 * Note, any outstanding transactions against us will keep us alive.
 	 */
	struct Promise : public Observable
	{
		Observer			keep_alive;
		SimplePromiseKeeper*		weak; // a weak reference
		recordptr_t			sector;
		
		MemHandle<data_t>		memory;
		short				use;
		
		std::list<recordptr_t>::iterator lru_position;
		
		void not_observed();
		Promise();
		
		// keep compiler happy
		virtual ~Promise() { }
	};
	
	struct ExclusiveLock : public Observable
	{
		SmartPtr<SimplePromiseKeeper>	owner;
		
		void not_observed();
		ExclusiveLock();
		
		// keep compiler happy
		virtual ~ExclusiveLock() { }
	};

 	typedef std::map<recordptr_t, Promise>	Promises;
 	typedef std::list<recordptr_t>		LRU;
 	
 	SmartPtr<SimpleTransactionManager>	m_manager;
 	SmartPtr<PoolI>	m_pool;
 	VolumeI*	m_volume;
 	int		m_held_sectors;
 	bool		m_locked;
 	
 	recordptr_t	m_eov; // update on actual write
 	volume_t	m_volid;
 	
 	ExclusiveLock	m_exclusive;
 	Promises	m_promises;
 	LRU		m_lru;
 	
 	// Build a new empty promise
 	Promises::iterator promise_builder(recordptr_t) throw (trans_exception);
 	
 	// Look for sectors we could drop without violating our promises
 	void free_some_sectors();
 	
 public:
 	SimplePromiseKeeper(
 			SimpleTransactionManager*	manager,
	 		const SmartPtr<PoolI>&		pool, 
 			VolumeI*			volume)
 		throw (trans_exception);
 	~SimplePromiseKeeper();
 	
 	MemHandle<const data_t> get_ro(TransactionI* trans, recordptr_t sect) throw (trans_exception);
 	MemHandle<data_t>       get_rw(TransactionI* trans, recordptr_t sect) throw (trans_exception);
 	
 	void                    truncate(TransactionI* trans, recordptr_t sect) throw (trans_exception);
 	VolumeHandle            get_raw_volume() throw (trans_exception);
 	
 	sectorlen_t		get_sector_size() const throw();
 	MemHandle<data_t>	get_a_sector_buffer();
 
 	// Called by SimpleTransactionManager to release us from a promise
 	// release will release us from the promise without writing
 	void release_promise(recordptr_t r) throw ();
 	// commit will write the promise to disk (BUT NOT RELEASE IT)
 	void commit_promise(recordptr_t r) throw (trans_exception);
 	
 	// Synchronize the underlying disk
 	void sync() throw (trans_exception);
 	
 // Make older compilers happy
 friend class JFA::detail::SimplePromiseKeeper::Promise;
 friend class JFA::detail::SimplePromiseKeeper::ExclusiveLock;
};

} // detail
} // JFA

#endif
