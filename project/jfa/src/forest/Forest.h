/*  $Id: Forest.h,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  Forest.h - Implementation of the ForestI
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

#ifndef __JFA_FOREST_H__
#define __JFA_FOREST_H__

namespace JFA
{
namespace detail
{

class Forest : public ForestI
{
 public:
 	void insert(TransactionI* trans,
 	            MemBlock<const data_t> key,
 	            MemBlock<const data_t> data)
 		throw (trans_exception);
 	
 	ForestIteratorI* find(TransactionI* trans, MemBlock<const data_t> key)
 		throw (trans_exception);
};

} // detail
} // JFA

#endif
