/*  $Id: LayoutI.h,v 1.1.1.1 2003-01-04 21:13:56 terpstra Exp $
 *  
 *  LayoutI.h - Interface describing a alloc/free layout manager
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

#ifndef __JFA_LAYOUT_I_H__
#define __JFA_LAYOUT_I_H__

#include <jfa/ABI/MemHandles.h>
#include <jfa/ABI/SmartPtr.h>
#include <jfa/ABI/TransactionI.h>

namespace JFA
{
namespace detail
{

/* The intention of this class is to provide a mechanism for management of 
 * sector-level alloc&free. A LayoutI always lives on top of a PromiseKeeperI.
 * The sector numbers returned should be indexed in the matching PromiseKeeper.
 */
class LayoutI : public MemoryManaged
{
 public:
 	// Simple storage management interface
 	virtual void        free (TransactionI* tran, recordptr_t record)
 		throw (trans_exception) = 0;
 	virtual recordptr_t alloc(TransactionI* tran)
 		throw (trans_exception) = 0;
 	
 	// Since we manage the storage, we must provide a way to find a
 	// "root" node from which a structure can operate.
 	// When the tree is empty, please use 0 for the root.
 	
 	virtual recordptr_t get_root(TransactionI* tran)
 		throw (trans_exception) = 0;
 	virtual void        set_root(TransactionI* tran, recordptr_t record)
 		throw (trans_exception) = 0;
};

} // detail
} // JFA

#endif
