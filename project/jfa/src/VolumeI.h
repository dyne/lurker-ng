/*  $Id: VolumeI.h,v 1.1.1.1 2003-01-04 21:13:56 terpstra Exp $
 *  
 *  VolumeI.h - Interface for raw sector-based volume; no transaction safety
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

#ifndef __JFA_VOLUME_I_H__
#define __JFA_VOLUME_I_H__

#include <jfa/ABI/MemHandles.h>
#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/TransactionI.h>

namespace JFA
{
namespace detail
{

typedef unsigned short volume_t;
JFA_STATIC_ASSERT(sizeof(volume_t) > 1);

// Note that VolumeI is *NOT* MemoryManaged.
// It persists as long as the environment.
// If you keep a pointer to a Volume, keep a SmartPtr to the VolumeManager.
//
// This is the case with PromiseKeepers; they only hold on to a VolumeI*, but
// also a SmartPtr to a TransactionManager which holds a SmartPtr to a VM.
//
// NOTE: This behavoiur is desireable. It means we do not keep opening and
// closing disk representations (which might be costly) and that all the
// volumes are available to a TransactionManager for replay.
class VolumeI
{
 public:
 	// Not required, but makes the compiler happy
 	virtual ~VolumeI() { }
 	
 	// Read the sector. Should assert fail or free/non-existant sectors
 	virtual	void pull_disk(recordptr_t, MemBlock<data_t>)       throw (trans_exception) = 0;
 	
 	// Write the sector. Grow the on-disk representation if necessary
 	virtual	void push_disk(recordptr_t, MemBlock<const data_t>) throw (trans_exception) = 0;
 	
 	// Get the number of the first past end sector.
 	// Only called once by Volume to setup allocation.
 	virtual recordptr_t get_end() throw (trans_exception) = 0;
 	
 	// A method which will forcably free all sectors >= last.
 	// This should actually release the disk to the OS.
 	// This should only ever be called when the disk has no readers/writers.
 	virtual void truncate(recordptr_t last) throw(trans_exception) = 0;
 	
 	// Synchronize all the writes to hard storage
 	virtual void sync() throw (trans_exception) = 0;
 	
 	// Get the volume's unique ID
 	virtual volume_t get_id() const throw () = 0;
 	
 	// Get the sector size
 	virtual sectorlen_t get_sector_size() const throw () = 0;
};

} // detail
} // JFA

#endif
