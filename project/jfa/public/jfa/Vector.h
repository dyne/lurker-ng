/*  $Id: Vector.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  Vector.h - User visible Vector implementation using RAW block-device
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

#ifndef __JFA_VECTOR_H__
#define __JFA_VECTOR_H__

#include <jfa/ABI/VectorI.h>

namespace JFA
{

template <class _RecordTraits>
class VectorDB;

namespace detail
{

template <class _RecordTraits>
class VectorSession
{
 protected:
 	SmartPtr<VectorI>	m_vector;
 	SmartPtr<TransactionI>	m_transaction;
 	
 	// cache this since we use it in many operations and it is slow to get
 	// THIS IS NOT THE REAL SECTOR SIZE!
 	// It is the number of _RecordTraits::len that fit.
 	// All addressing is done this way for VectorSession::*
 	sectorlen_t		m_sector_size;
 	recordptr_t		m_size_cache; // cache those nasty size() calls
 	MemHandle<data_t>	m_root;       // cache the root entry
 
 protected:
 	VectorSession(const SmartPtr<VectorI>& db, const SmartPtr<TransactionI>& tran)
 	 : m_vector(db), m_transaction(tran), 
 	   m_sector_size(m_vector->get_sector_size()), m_size_cache(-1)
 	{
 		m_sector_size /= _RecordTraits::len;
 	}
 	
 	class reference_;
 	class const_reference_
 	{
 	 public:
 		typedef const typename _RecordTraits::T	value_type;
 	 	typedef MemHandle<const data_t>		Handle;
 	 
 	 protected:
 	 	Handle		m_buffer;
 	 	sectorlen_t	m_position;
 	 
 	 public:
 	 	const_reference_(const Handle& buffer, sectorlen_t position)
 	 	 : m_buffer(buffer), m_position(position)
 	 	{ }
 	 	
 	 	operator value_type () const
 	 	{
 	 		return _RecordTraits::deserialize(
 	 			m_buffer.buf + (m_position * _RecordTraits::len));
 	 	}
 	 	
 	 	static Handle pull(VectorI* vect, TransactionI* tran, recordptr_t sect)
 	 	{
 	 		return vect->get_ro(tran, sect);
 	 	}
 	 
 	 // Allow an optimization to dodge serialization work
 	 friend class JFA::detail::VectorSession<_RecordTraits>::reference_;
 	};
 	
 	class reference_
 	{
 	 public:
 		typedef typename _RecordTraits::T	value_type;
 	 	typedef MemHandle<data_t>		Handle;
 	 
 	 protected:
 	 	Handle		m_buffer;
 	 	sectorlen_t	m_position;
 	 
 	 public:
 	 	reference_(const Handle& buffer, sectorlen_t position)
 	 	 : m_buffer(buffer), m_position(position)
 	 	{ }
 	 	
 	 	operator value_type () const
 	 	{
 	 		return _RecordTraits::deserialize(
 	 			m_buffer.buf + (m_position * _RecordTraits::len));
 	 	}
 	 	
 	 	reference_& operator = (const value_type& val)
 	 	{
 	 		_RecordTraits::serialize(
 	 			m_buffer.buf + (m_position * _RecordTraits::len),
 	 			val);
 	 		return *this;
 	 	}
 	 	
 	 	// We can accelerate this operation
 	 	reference_& operator = (const reference_& val)
 	 	{
 	 		memcpy(    m_buffer.buf + (    m_position * _RecordTraits::len),
 	 		       val.m_buffer.buf + (val.m_position * _RecordTraits::len),
 	 		       _RecordTraits::len);
 	 		return *this;
 	 	}
 	 	
 	 	// We can accelerate this operation
 	 	reference_& operator = (const const_reference_& val)
 	 	{
 	 		memcpy(    m_buffer.buf + (    m_position * _RecordTraits::len),
 	 		       val.m_buffer.buf + (val.m_position * _RecordTraits::len),
 	 		       _RecordTraits::len);
 	 		return *this;
 	 	}
 	 	
 	 	static Handle pull(VectorI* vect, TransactionI* tran, recordptr_t sect)
 	 	{
 	 		return vect->get_rw(tran, sect);
 	 	}
 	};
 	
 	template <class _Reference>
 	class iterator_
 	{
 	 public:
 	 	// This annoying detail is for compatability
 	 	typedef std::random_access_iterator_tag	iterator_category;
 	 	
 	 	// Only these two types are actually required by the stl
 		typedef typename _Reference::value_type	value_type;
 	 	typedef recordptr_t			difference_type;
 	 	
 	 	// A common extension which I can support too
 	 	typedef _Reference			reference;
 	 	
 	 	// Ok, I can't actually make this work, but I can at least
 	 	// make templates which specialize this class happy.
 	 	typedef void*				pointer;
 	 
 	 //protected:
 	 public:	// If you can get the friendship right, go ahead
 	 	SmartPtr<VectorI>	m_vector;
 	 	SmartPtr<TransactionI>	m_transaction;
 	 	
 	 	sectorlen_t		m_sector_size; // see above
 	 	recordptr_t		m_sector;
 	 	
 	 	sectorlen_t		m_position;    // see above
 	 	
 	 	// Ok, hate me if you will, but it is way better to allow
 	 	// this buffer to be pulled in lazily on const reads than
 	 	// to lock the database all over the place just to statisfy
 	 	// the C++ compiler.
 	 	mutable typename _Reference::Handle m_buffer;
 	 	
 	 protected:
 	 	iterator_(const VectorSession<_RecordTraits>& session,
 	 		recordptr_t sect, sectorlen_t pos)
 	 	 : m_vector     (session.m_vector),
 	 	   m_transaction(session.m_transaction),
 	 	   m_sector_size(session.m_sector_size),
 	 	   m_sector     (sect),
 	 	   m_position   (pos),
 	 	   m_buffer     (0, 0, 0)
 	 	{
 	 	}
 	 
 	 public:
 	 	// default construction is required by the STL.
 	 	// Just don't even THINK about trying to use this object
 	 	iterator_()
 	 	 : m_vector(0), m_transaction(0), m_sector_size(0),
 	 	   m_sector(0), m_position(0), m_buffer(0, 0, 0)
 	 	{
 	 	}
 	 	
 	 	// We override the default constructor so that we support
 	 	// conversion from non-const to const
 	 	template <class _OReference>
 	 	iterator_(const iterator_<_OReference>& i)
 	 	 : m_vector     (i.m_vector),
 	 	   m_transaction(i.m_transaction),
 	 	   m_sector_size(i.m_sector_size),
 	 	   m_sector     (i.m_sector),
 	 	   m_position   (i.m_position),
 	 	   m_buffer     (i.m_buffer)
 	 	{ }
 	 	
 	 	// Same for this as above
 	 	template <class _OReference>
 	 	iterator_& operator = (const iterator_<_OReference>& i)
 	 	{
 			m_vector      = i.m_vector;
 			m_transaction = i.m_transaction;
 			m_sector_size = i.m_sector_size;
 			m_sector      = i.m_sector;
 			m_position    = i.m_position;
 			m_buffer      = i.m_buffer;
 	 		
 	 		return *this;
 	 	}
 	 	
 	 	bool operator == (const iterator_& i) const
 	 	{
 	 		return m_sector   == i.m_sector &&
 	 		       m_position == i.m_position;
 	 	}
 	 	
 	 	bool operator != (const iterator_& i) const
 	 	{ return !(*this == i); }
 	 	
 	 	difference_type operator - (const iterator_& i) const
 	 	{
 	 		return (m_sector   - i.m_sector) * m_sector_size +
 	 		       (m_position - i.m_position);
 	 	}
 	 	
 	 	iterator_& operator += (difference_type x)
 	 	{
 	 		// new_position in a temp that is WIDER
 	 		recordptr_t new_position = m_position;
 	 		recordptr_t new_sector   = m_sector;
 	 		
 	 		new_position += x;
 	 		new_sector   += new_position / m_sector_size;
 	 		new_position %= m_sector_size;
 	 		
 	 		if (new_position < 0)
 	 		{
 	 			new_position += m_sector_size;
 	 			--new_sector;
 	 		}
 	 		
 	 		if (new_sector != m_sector)
 	 		{	// Discard buffer if we are no longer on it
 	 			m_buffer = typename _Reference::Handle(0, 0, 0);
 	 			m_sector = new_sector;
 	 		}
 	 		m_position = new_position;
 	 		
 	 		return *this;
 	 	}
 	 	
 	 	iterator_& operator -= (difference_type x)
 	 	{
 	 		return *this += -x;
 	 	}
 	 	
 	 	iterator_& operator-- () { return *this -= 1; }
 	 	iterator_& operator++ () { return *this += 1; }
 	 	
 	 	iterator_ operator - (difference_type x) const
 	 	{
 	 		iterator_ out = *this;
 	 		out -= x;
 	 		return out;
 	 	}
 	 	
 	 	iterator_ operator + (difference_type x) const
 	 	{
 	 		iterator_ out = *this;
 	 		out += x;
 	 		return out;
 	 	}
 	 	
 	 	iterator_ operator-- (int)
 	 	{
 	 		iterator_ tmp = *this;
 	 		--*this;
 	 		return tmp;
 	 	}
 	 	
 	 	iterator_ operator++ (int)
 	 	{
 	 		iterator_ tmp = *this;
 	 		++*this;
 	 		return tmp;
 	 	}
 	 	
 	 	_Reference operator * () const
 	 	{
 	 		if (!m_buffer.buf)
 	 		{
	 	 		// The zero-th sector is reserved by us.
 		 		assert (m_sector > 0);
 	 			
 	 			// Grab the sector
 	 			m_buffer = _Reference::pull(
 	 				m_vector.get(),
 	 				m_transaction.get(),
 	 				m_sector);
 	 		}
			
 	 		return _Reference(m_buffer, m_position);
 	 	}
 	 	
 	 	_Reference operator [] (difference_type x) const
 	 	{	// I could potentially make this faster (not require
 	 		// the temporary), but I think the clarity is better.
 	 		return *(*this + x);
 	 	}
 	 
 	 friend iterator_ operator - (difference_type x, const iterator_& i)
 	 { return i - x; }
 	 friend iterator_ operator + (difference_type x, const iterator_& i)
 	 { return i + x; }
 	 
 	 friend class JFA::detail::VectorSession<_RecordTraits>;
 	};
 	
 public:
 	typedef typename _RecordTraits::T		value_type;
 	typedef reference_				reference;
 	typedef const_reference_			const_reference;
 	typedef recordptr_t				size_type;
 	typedef recordptr_t				difference_type;
 	typedef iterator_<reference_>			iterator;
	typedef iterator_<const_reference_>		const_iterator;
	typedef std::reverse_iterator<iterator>		reverse_iterator;
	typedef std::reverse_iterator<const_iterator>	const_reverse_iterator;
 	// pointer; crazy! - forget it!
 	typedef void*					pointer;
 	
 protected:
 	void _set_size(size_type sz)
 	{
 		if (m_root.buf == 0)
			m_root = m_vector->get_rw(m_transaction.get(), 0);
	 	
	 	// It is likely that our type is not 8 bytes wide. :-)
	 	// I am just being cautious for future format changes.
		m_size_cache = sz;
		for (int i = 7; i >= 0; --i)
		{
			m_root.buf[i] = sz & 0xFF;
			sz >>= 8;
		}
 	}
 	
 public:
 	      iterator begin()       { return       iterator(*this, 1, 0); }
 	const_iterator begin() const { return const_iterator(*this, 1, 0); }
 	
 	// This function is very expensive; cache the result!
 	// Also, be advised that it will lock the database from appends.
 	const_iterator end() const { return begin() += size(); }
 	      iterator end()       { return begin() += size(); }
 	
 	      reverse_iterator rbegin()       { return       reverse_iterator(end());   }
 	const_reverse_iterator rbegin() const { return const_reverse_iterator(end());   }
 	      reverse_iterator rend  ()       { return       reverse_iterator(begin()); }
 	const_reverse_iterator rend  () const { return const_reverse_iterator(begin()); }
 	
 	reference       front()       { return *begin();   }
 	const_reference front() const { return *begin();   }
 	reference       back ()       { return *(end()-1); }
 	const_reference back () const { return *(end()-1); }
 	
 	const_reference operator [] (difference_type x) const
 	{
 		// using += means we only use one temporary
 		return *(begin() += x);
 	}
 	
 	// I have not decided whether or not to allow this as
 	// vect[4] = x;  -- it would mean x = vect[4]; would lock the record.
 	reference mutable_reference_at(difference_type x)
 	{
 		return *(begin() += x);
 	}
 	
 	size_type max_size()
 	{
 		return recordptr_max;
 	}
 	
 	bool empty()
 	{
 		return size() == 0;
 	}
 	
 	size_type size()
 	{
	 	if (m_size_cache != -1) return m_size_cache;
	 	
 		MemHandle<const data_t> h =
	 		m_vector->get_ro(m_transaction.get(), 0);
	 	
	 	// If we read past the end to read this counter, then the
	 	// database is empty.
	 	if (h.buf == 0) return 0;
	 	
	 	// It is likely that our type is not 8 bytes wide. :-)
	 	// I am just being cautious for future format changes.
	 	m_size_cache = 0;
	 	for (int i = 0; i < 8; ++i)
	 	{
	 		m_size_cache <<= 8;
	 		m_size_cache |= h.buf[i];
	 	}
	 	
	 	return m_size_cache;
 	}
 	
 	void resize(size_type record)
 	{
 		_set_size(record);
 		// note: will not default construct as standard requires
 		return m_vector->truncate(m_transaction.get(), 
 			(record+m_sector_size-1)/m_sector_size + 1);
 	}
 	
 	void clear()
 	{
 		return resize(0);
 	}
 	
 	void push_back(const value_type& v)
 	{
 		size_type sz = size();
 		_set_size(sz+1);
 		*(begin() += sz) = v;
 	}
 	
 	void pop_back()
 	{
 		size_type sz = size();
 		assert (sz);
 		return resize(sz-1);
 	}
	
 	// reserve; maybe
 	// capacity - if reserve then this too
 	
 	// swap - the contents of two vectors; crazy! no.
 	// insert - by position and shift the database; crazy! no.
 	// erase - something not at the end; crazy! no.
 	
 	// The standard requires that containers be assignable.
 	// Don't call this or you will suffer!!
 	VectorSession& operator = (const VectorSession& o)
 	{
 		if (o.m_vector == m_vector) // Please god!!
 		{
	 		resize(o.size());
	 		std::copy(o.begin(), o.end(), begin());
	 	}
 		return *this;
 	}
 
 // This class can construct us
 friend class JFA::VectorDB<_RecordTraits>;
 
 // Satisfy compiler
 friend class JFA::detail::VectorSession<_RecordTraits>::iterator_
 	<JFA::detail::VectorSession<_RecordTraits>::reference_>;
 friend class JFA::detail::VectorSession<_RecordTraits>::iterator_
 	<JFA::detail::VectorSession<_RecordTraits>::const_reference_>;
 
 // All those ridiculous comparators the standard permits on containers...
 // PS. Don't even think about actually using these!
 
 friend bool operator <  (const VectorSession& x, const VectorSession& y)
 { return std::lexicographical_compare(x.begin(), x.end(), y.begin(), y.end()); }
 friend bool operator == (const VectorSession& x, const VectorSession& y)
 { return x.size() == y.size() && std::equal(x.begin(), x.end(), y.begin()); }
 
 // All the other ones
 friend bool operator >  (const VectorSession& x, const VectorSession& y) { return   y < x;  }
 friend bool operator >= (const VectorSession& x, const VectorSession& y) { return !(x < y); }
 friend bool operator <= (const VectorSession& x, const VectorSession& y) { return !(y < x); }
 friend bool operator != (const VectorSession& x, const VectorSession& y) { return   y != x; }
};

} // detail

template <class _RecordTraits>
class VectorDB
{
 public:
 	typedef detail::VectorSession<_RecordTraits> Session;
 	
 protected:
 	detail::SmartPtr<detail::VectorI> interface;
 	
 	Session create_session(const detail::SmartPtr<detail::TransactionI>& trans) const
 	{
 		return Session(interface, trans);
 	}
 
 public:
 	VectorDB(const Environment& env, const std::string& name)
 	 : interface(detail::VectorFactory(env._get_internal(), name))
 	{ }
 
 friend class Transaction;
};

} // JFA

#endif
