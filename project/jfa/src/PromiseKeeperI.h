/*  $Id: PromiseKeeperI.h,v 1.1.1.1 2003-01-04 21:13:56 terpstra Exp $
 *  
 *  PromiseKeeperI.h - Interface for safely-locked volume usage
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

#ifndef __JFA_PROMISE_KEEPER_I_H__
#define __JFA_PROMISE_KEEPER_I_H__

#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/MemHandles.h>
#include <jfa/ABI/TransactionI.h>

namespace JFA
{
namespace detail
{

class VolumeI;

// Note, the observable inside a VolumeHandle must guarantee to keep the
// PromiseKeeper alive.
struct VolumeHandle : protected Observer
{
	VolumeI*	volume;
	
	VolumeHandle(Observable* o, VolumeI* v)
	 : Observer(o), volume(v) { }
};

// Note that although these functions take any TransactionI*, you must
// only pass those which you obtained from the same TransactionManagerI.
//
// Also, any PromiseKeeper will keep alive it's TransactionManager.
// Any operations done on a PromiseKeeper by a Transaction will keep the
// PromiseKeeper alive till the Transaction completes.
class PromiseKeeperI : public MemoryManaged
{
 public:
 	// Unlike a RAW volume, reading here will succeed even for a
 	// non-existant sector. However, the buffer will be NULL!
 	// The intent of this behaviour is to enable locking past end regions
 	virtual MemHandle<const data_t> get_ro(TransactionI* trans, recordptr_t sect) throw (trans_exception) = 0;
 	
 	// Calling this on a non-existant sector will succeed and give you
 	// an unitialized buffer. When the transaction commits, the device
 	// will be extended.
 	virtual MemHandle<data_t>       get_rw(TransactionI* trans, recordptr_t sect) throw (trans_exception) = 0;
 	
 	// Destroy all sectors >= sect. This will require a write lock on
 	// every sector past the end. When the transaction commits, the
 	// device will be resized to the maximum of this parameter and all
 	// future get_rw's by the transaction prior to commit.
 	// Note: It is also possible to grow the database with this call which
 	// is simply equivalent to write-locking all sectors from end->sect.
 	// Also note: VolumeI does NOT have this behaviour.
 	virtual void truncate(TransactionI* trans, recordptr_t sect) throw (trans_exception) = 0;
 	
 	// To access the RAW device, call this method.
 	// This will only succeed if there are no readers of the device.
 	// As long as you hold the raw device handle, all of the above calls
 	// to the promise keeper will fail. -- Only one VolumeHandle may
 	// exist at any point in time.
 	virtual VolumeHandle get_raw_volume() throw (trans_exception) = 0;
 	
 	// A handy way to get the sector size (just calls the volume)
 	virtual sectorlen_t get_sector_size() const throw() = 0;
 	
 	// Some people need a raw sector buffer now and again.
 	// Since we know how to allocate them (get_ro/get_rw) here is a good
 	// place to expose this method.
 	virtual MemHandle<data_t>	get_a_sector_buffer() = 0;
};

} // detail
} // JFA

#endif
