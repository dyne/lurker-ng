/*  $Id: SimpleLockHolder.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  SimpleLockHolder.h - A dirty-buffer-always-in-RAM implementation
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

#ifndef __JFA_SIMPLE_TRANSACTION_H__
#define __JFA_SIMPLE_TRANSACTION_H__

#include "SimpleTransactionManager.h"
#include "../VolumeI.h"
#include "../LockHolderI.h"
#include "../LockI.h"

#include <set>

namespace JFA
{
namespace detail
{

class SimplePromiseKeeper;

class SimpleLockHolder : public LockHolderI
{
 protected:
 	// We do not need to keep a smart pointer because if we have a promise,
 	// the promise itself has a smart pointer to the promisekeeper
 	typedef std::set<std::pair<SimplePromiseKeeper*, recordptr_t> > Contracts;
 	typedef std::set<LockI*>					Locks;
 	
 	SmartPtr<SimpleTransactionManager>	m_tm;
 	bool					m_transaction_complete;
 	Contracts				m_contracts;
 	Locks					m_locks;
 	
 public:
 	SimpleLockHolder(SimpleTransactionManager* tm);
 	~SimpleLockHolder();
 	
 	void abort()  throw ();
 	void commit() throw (trans_exception);
 	
 	void read_lock (LockI* lock) throw (trans_exception);
 	void write_lock(LockI* lock) throw (trans_exception);
 
 friend class SimplePromiseKeeper;
 friend class SimpleTransactionManager;
};

} // detail
} // JFA

#endif
