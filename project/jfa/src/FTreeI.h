/*  $Id: FTreeI.h,v 1.1.1.1 2003-01-04 21:13:56 terpstra Exp $
 *  
 *  FTreeI.h - A write-once read-many-times sorted array
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

#ifndef __JFA_FTREE_I_H__
#define __JFA_FTREE_I_H__

#include <string>
#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/EnvironmentI.h>

#include "StreamI.h"

namespace JFA
{
namespace detail
{

class FTreeI : public MemoryManaged
{
 public:
 	// Return an IStreamI* whose first output will be the last sector
 	// which contains the last key < key. (or the first sector)
 	// Something >= key is NOT guaranteed.
 	// The reason for this behaviour is because of multiple identical keys
 	virtual SmartPtr<IStreamI>	find(TransactionI* t, MemBlock<const data_t> key) 
 		throw (trans_exception) = 0;
 	
 	// Toasts the database (truncates to nothing), then begins
 	// writing the database with the given ostream.
 	// Note: will fail if there are any open readers.
 	// Note: only one such stream can exist at any time.
 	// Note: you MUST promise to write sectors in sorted order.
 	// The sectors MUST start with <keylen> <anybyte> [full key...] [whatever...]
 	virtual SmartPtr<OStreamI>	rewrite() 
 		throw (trans_exception) = 0;
};

SmartPtr<FTreeI> FTreeFactory(EnvironmentI* env, const std::string& name)
	throw (env_exception);

} // detail
} // JFA

#endif
