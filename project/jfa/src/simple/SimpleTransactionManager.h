/*  $Id: SimpleTransactionManager.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  SimpleTransactionManager.h - A dirty-buffer-always-in-RAM implementation 
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

#ifndef __JFA_SIMPLE_TRANSACTION_MANAGER_H__
#define __JFA_SIMPLE_TRANSACTION_MANAGER_H__

#include "Simple.h"
#include "../VolumeManagerI.h"

#include <map>

namespace JFA
{
namespace detail
{

class SimpleLockHolder;
class VolumeManagerI;
class SimplePromiseKeeper;

class SimpleTransactionManager : public TransactionManagerI
{
 private:
 	typedef std::map<volume_t, SimplePromiseKeeper*> Keepers;
 	
 	SmartPtr<VolumeManagerI>	m_vm;
 	SmartPtr<PoolI>			m_pool;
 	VolumeI*			m_journal;
 	Keepers				m_keepers;
 	
 public:
 	SimpleTransactionManager(VolumeManagerI* vm, PoolI* pool)
 		throw (env_exception);
 	
 	// Called by SimpleTransaction
 	void commit(SimpleLockHolder* trans) throw (trans_exception);
 	void abort (SimpleLockHolder* trans) throw ();
 	
 	// Called by SimplePromiseKeeper
 	void borne(volume_t id, SimplePromiseKeeper* keeper);
 	void died (volume_t id);
 	
 	SmartPtr<PromiseKeeperI> open_volume(const std::string& name)
 		throw (env_exception);
 	LockHolderI* open_transaction()
 		throw (trans_exception);
 	
 	MemHandle<data_t>	get_a_sector_buffer();
};

} // detail
} // JFA

#endif
