/*  $Id: Forest.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  Forest.h - User visible Forest wrapper
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

#include <jfa/ABI/ForestI.h>

namespace JFA
{

template <class _RecordTraits>
class ForestDB;

namespace detail
{

template <_RecordTraits>
class ForestSession
{
 protected:
 	SmartPtr<ForestI>	m_forest;
 	SmartPtr<TransactionI>	m_transaction;
 
 protected:
 	ForestSession(const SmartPtr<ForestI>& db, const SmartPtr<TransactionI>& tran)
 	 : m_forest(db), m_transaction(tran)
 	{ }
 	
 	class const_reference_
 	{
 	 protected:
 	 	Observer		m_reference;
 	 	MemBlock<const data_t>	m_key;
 	 	MemBlock<const data_t>	m_data;
 	};
 
 public:
 	typedef typename _RecordTraits::T		value_type;
 	//typedef reference_				reference;
 	//typedef const_reference_			const_reference;
 	//typedef recordptr_t				size_type;
 	//typedef recordptr_t				difference_type;
 	//typedef iterator_<reference_>			iterator;
	//typedef iterator_<const_reference_>		const_iterator;
	//typedef std::reverse_iterator<iterator>		reverse_iterator;
	//typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
 	// pointer; crazy! - forget it!
 	//typedef void*					pointer;
};

} // detail

template <class _RecordTraits>
class ForestDB
{
 public:
 	typedef detail::ForestSession<_RecordTraits> Session;
 
 protected:
 	detail::SmartPtr<detail::ForestI> interface;
 	
 	Session create_session(const detail::SmartPtr<detail::TransactionI>& trans) const
 	{
 		return Session(niterface, trans);
 	}
 
 public:
 	ForestDB(const Environment& env, const std::string& name)
 	 : interface(detail::ForestFactory(env._get_internal(), name))
 	{  }
 
 friend class Transacion;
};

} // JFA

#endif
