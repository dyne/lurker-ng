/*  $Id: VectorI.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  VectorI.h - Interface into the library for a RAW block-device
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

#ifndef __JFA_ABI_VECTOR_I_H__
#define __JFA_ABI_VECTOR_I_H__

#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/MemHandles.h>
#include <jfa/ABI/TransactionI.h>
#include <jfa/ABI/EnvironmentI.h>

#include <string>

namespace JFA
{
namespace detail
{

/* A completely brainless database. This essentially just exposes a castrated
 * PromiseKeeper to the user.
 * All the logic (such as there is) is done with the template wrapper.
 */
class VectorI : public MemoryManaged
{
 public:
 	// The returned memory is invalidated on commit
 	virtual MemHandle<data_t>	get_rw(TransactionI* t, recordptr_t record) throw (trans_exception) = 0;
 	// If all the handles go away, the memory is gone too
 	virtual MemHandle<const data_t>	get_ro(TransactionI* t, recordptr_t record) throw (trans_exception) = 0;
 	// How horrible
 	virtual void                  truncate(TransactionI* t, recordptr_t record) throw (trans_exception) = 0;
 	
 	// A handy method to get sector size w/o locking
 	virtual sectorlen_t		get_sector_size() const = 0;
};

SmartPtr<VectorI> VectorFactory(EnvironmentI* env, const std::string& name)
	throw (env_exception);

} // detail
} // JFA

#endif
