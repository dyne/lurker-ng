/*  $Id: LockI.h,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  LockI.h - Interface for locks which can be associated with transactions
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

#ifndef __JFA_LOCK_I_H__
#define __JFA_LOCK_I_H__

namespace JFA
{
namespace detail
{

class TransactionI;

/* Sometimes it is necessary to lock non-disk resources.
 * 
 * However, since we cannot record these operations in a journal, there are
 * certain constraints. Namely: commit & abort may never throw.
 * 
 * If a transaction holding a write lock commits, the commit() callback is
 * invoked while the lock is still retained, but after all operations that
 * might fail.
 * 
 * If a transaction holding a write lock aborts, the abort() callback is
 * invoked while the lock is still retained.
 * 
 * After the lock is released, if the Lock is UNUSED release() is called.
 * 
 * Also, this interface is not MemoryManaged; whatever structure is using the
 * LockIs is responsible for it's lifetime. The structure must never release
 * a lock which is locked since this lock is probably pointed to elsewhere.
 */
class LockI
{
 public:
	static const short WRITE_LOCKED	= -1;
	static const short UNUSED	= 0;
	static const short EXCLUSIVE_USE= 1;
	
	short use;
	
	// These call-backs may not throw.
	virtual void commit () throw () = 0;
	virtual void release() throw () = 0;
};

} // detail
} // JFA

#endif
