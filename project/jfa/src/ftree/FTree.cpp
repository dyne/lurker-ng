/*  $Id: FTree.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  FTree.cpp - A write-once read-many-times sorted array
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include <list>
#include <string.h> // good old memcpy & memcmp

#include "FTree.h"
#include "../VolumeI.h"
#include "../TransactionManagerI.h"
#include "../PromiseKeeperI.h"
#include "../Exception.h"
#include "../misc/intSerialize.h"
#include "../misc/intDeserialize.h"

// 4 -> 4 billion sectors * 8k? = 32 terrabytes. Hopefully enough...
// Maybe I should make this bigger?
// OTOH, I could use a variable scheme. 2 bytes for first level
// 4 for second, 6 for third, ... --- this sounds good. v1.1? !!!
#define BYTES_PER_SECTOR_CODING	4

/* Future work:
 *  Improve the sector density by only keeping enough that the keys differ. 
 *  Note: the LAST key of the preceding sector and the FIRST key of the
 *  succeeding sector are what must be considered.
 *
 *  Unroll comparison and copying algorithms.
 *
 * !!! compress higher-level nodes
 */

namespace JFA
{
namespace detail
{

/* The storage scheme is: user writes and reads to .data level.
 * We read/write to .level<x> 
 * For each level we keep a sector buffer that needs flushing.
 */
class FTree : public FTreeI
{
 protected:
 	typedef std::list<SmartPtr<PromiseKeeperI> >	Levels;
 	
 protected:
 	SmartPtr<TransactionManagerI>	m_tm;
 	std::string			m_name;
 	Levels				m_levels;
 	SmartPtr<PromiseKeeperI>	m_data;
 	
 	class Reader : public IStreamI
 	{
 	 protected:
 		TransactionI*			m_trans;
 		SmartPtr<PromiseKeeperI>	m_keeper;
 		recordptr_t			m_sector;
 	 	
 	 public:
 	 	Reader(	TransactionI*	trans, 
 	 		PromiseKeeperI*	keeper,
 	 		recordptr_t	sector);
 	 	
 		MemHandle<const data_t> get_next()
 			throw (trans_exception);
 		
 		IStreamI*	clone() const
 			throw (trans_exception);
 	};
 	
 	struct Writer : public OStreamI
 	{
 	 protected:
 	 	struct Level
 	 	{
	 		VolumeHandle		handle;
	 		MemHandle<data_t>	buffer;
	 		sectorlen_t		offset;
	 		recordptr_t		sector;
	 		
	 		Level(	const VolumeHandle&		_handle,
	 			const MemHandle<data_t>&	_buffer,
	 			sectorlen_t			_offset,
	 			recordptr_t			_sector)
	 		 : handle(_handle), buffer(_buffer),
	 		   offset(_offset), sector(_sector)
	 		{ }
	 	};
	 	
	 	typedef std::list<Level>	Levels;
	 	
 	 	SmartPtr<FTree>	m_tree;
 	 	recordptr_t	m_sector;
 	 	VolumeHandle	m_datahandle;
	 	Levels		m_levels;
 	 	
 	 	void write_level(
 	 		Levels::iterator	i,
 	 		MemBlock<const data_t>	key,
 	 		recordptr_t		sector)
 	 		throw (trans_exception);
 	 	
 	 public:
 	 	void put_next(MemBlock<const data_t> sect)
 			throw (trans_exception);
 		
 		Writer(FTree* tree)
 			throw (trans_exception);
 	};
 	
 public:
 	FTree(TransactionManagerI* tm, const std::string& name)
 		throw (env_exception);
 	
 	SmartPtr<IStreamI> find(TransactionI* t, MemBlock<const data_t> key)
 		throw (trans_exception);
 	SmartPtr<OStreamI> rewrite()
 		throw (trans_exception);
 	
 	std::string grow_level_name() const;
 	
 friend class JFA::detail::FTree::Writer;
};

static bool _ftree_less(const data_t* a, data_t al, 
                        const data_t* b, data_t bl)
{
	while (al && bl)
	{
		if (*a < *b) return true;
		
		--al;
		--bl;
	}
	
	return al == 0;
}

FTree::Reader::Reader(TransactionI* trans, PromiseKeeperI* keeper, recordptr_t sector)
 : m_trans(trans), m_keeper(keeper), m_sector(sector)
{
}

MemHandle<const data_t> FTree::Reader::get_next() throw (trans_exception)
{
	// We rely here on the keeper to give us a null buffer on reading
	// past the end.
	MemHandle<const data_t> out = m_keeper->get_ro(m_trans, m_sector);
	if (out.buf) ++m_sector;
	return out;
}

IStreamI* FTree::Reader::clone() const throw (trans_exception)
{
	// The only thing that matters here is that we have a seperate
	// m_sector to increment.
	return new Reader(m_trans, m_keeper.get(), m_sector);
}

void FTree::Writer::write_level(
	FTree::Writer::Levels::iterator	i, 
	MemBlock<const data_t>		key,
	recordptr_t			sector)
	throw (trans_exception)
{
	if (i == m_levels.end())
	{
		// oops; we need a new level
		
		// These might throw (but it should be ok)
		SmartPtr<PromiseKeeperI> newkeeper;
		try
		{
			newkeeper = m_tree->m_tm->open_volume(
				m_tree->grow_level_name());
		}
		catch (const env_exception& e)
		{
			THROW(io_retry_exception, "Failed to get subvolume");
		}
		VolumeHandle h = newkeeper->get_raw_volume();
		
		// these commands never throw
		m_tree->m_levels.push_back(newkeeper);
		m_levels.push_back(Level(h,
			newkeeper->get_a_sector_buffer(),
			0, 0));
		i = --m_levels.end();
		
		// Prep it. -- if we are a new level, for sure we point
		// to 0 on the left-end.
		i->offset = BYTES_PER_SECTOR_CODING;
		intSerializeFast<BYTES_PER_SECTOR_CODING>(i->buffer.buf, 0);
		
		// terminating it is unneccessary since we are about to
		// fill it in the else { clause below
		// i->buffer.buf[i->offset] = 0;
		
		// Ok, if we throw now, we are consistent
	}
	
	if (i->offset + 1 + key.len + BYTES_PER_SECTOR_CODING + 1 >=
	    i->buffer.len)
	{
		// this would overflow the current sector
		
		// Write out our sector (could throw)
		i->handle.volume->push_disk(i->sector, i->buffer);
		
		// Call the parent at this point while no state changed
		write_level(++Levels::iterator(i), key, i->sector + 1);
		// State of parent changed; no more throwing on the way down!
		
		// Move to the next sector
		++i->sector;
		
		// Prep it.
		i->offset = BYTES_PER_SECTOR_CODING;
		intSerializeFast<BYTES_PER_SECTOR_CODING>(i->buffer.buf, sector);
		
		// terminate it
		i->buffer.buf[i->offset] = 0;
	}
	else
	{
		// it fits in this sector
		i->buffer.buf[i->offset] = key.len;
		++i->offset;
		
		::memcpy(&i->buffer.buf[i->offset], key.buf, key.len);
		i->offset += key.len;
		
		intSerializeFast<BYTES_PER_SECTOR_CODING>(&i->buffer.buf[i->offset], sector);
		i->offset += BYTES_PER_SECTOR_CODING;
		
		// terminate the sector
		i->buffer.buf[i->offset] = 0;
	}
}

void FTree::Writer::put_next(MemBlock<const data_t> sect)
	throw (trans_exception)
{
	if (sect.buf == 0)
	{
		// Flush all the buffers
		
		Levels::iterator i;
		for (i  = m_levels.begin(); 
		     i != m_levels.end();
		     ++i)
		{
			// Write out our sector (could throw)
			i->handle.volume->push_disk(i->sector, i->buffer);
		}
		
		return;
	}
	
	m_datahandle.volume->push_disk(m_sector, sect);
	if (m_sector != 0)
	{
		MemBlock<const data_t> key(sect.buf+2, *sect.buf);
		// Call the parent write at this point with no state change
		// (so we can rewrite on failure)
		write_level(m_levels.begin(), key, m_sector);
	}
	
	++m_sector;
}

FTree::Writer::Writer(FTree* tree)
	throw (trans_exception)
 : m_tree(tree), m_sector(0),
   m_datahandle(m_tree->m_data->get_raw_volume())
{
	// If we can get a handle to the bottom (we just did) and to the
	// top (why we use back() instead of front() below), then
	//  no one is reading the tree, so blast it!
	
	while (!tree->m_levels.empty())
	{
		VolumeHandle h = m_tree->m_levels.back()->get_raw_volume();
		h.volume->truncate(0); // die!
		m_tree->m_levels.pop_back();
	}
	
	m_datahandle.volume->truncate(0); // die!
	
	// Whew. All that destruction was satisfying. Now, I guess we are done.
}

std::string FTree::grow_level_name() const
{
	char tmp[2];
	tmp[0] = '0' + m_levels.size(); // more than 4 levels is crazy!
	tmp[1] = 0;
	
	return m_name + ".level" + tmp;	
}

FTree::FTree(TransactionManagerI* tm, const std::string& name)
	throw (env_exception)
 : m_tm(tm), m_name(name)
{
	m_data = m_tm->open_volume(name + ".data");
	
	try
	{
		VolumeHandle h = m_data->get_raw_volume();
		while (h.volume->get_end() >= 2)
		{
			SmartPtr<PromiseKeeperI> keeper
				= m_tm->open_volume(grow_level_name());
			m_levels.push_back(keeper);
			h = keeper->get_raw_volume();
			
			if (h.volume->get_end() == 0)
				THROW(corrupt_exception,
					"Missing root level!");
		}
	}
	catch (const trans_exception& e)
	{
		THROW(corrupt_exception, 
			"Unable to obtain locks on opened FTree?");
	}
}

SmartPtr<IStreamI> FTree::find(TransactionI* t, MemBlock<const data_t> key)
	throw (trans_exception)
{
	recordptr_t sector = 0;
	Levels::reverse_iterator i;
	
	for (i  = m_levels.rbegin();
	     i != m_levels.rend();
	     ++i)
	{
		MemHandle<const data_t> h = (*i)->get_ro(t, sector);
		
		sectorlen_t off = BYTES_PER_SECTOR_CODING;
		intDeserialize<BYTES_PER_SECTOR_CODING>(h.buf, sector);
		
		while (h.buf[off] && // not last record
		      _ftree_less(	h.buf+off+1, h.buf[off],
					key.buf,     key.len))
		{
			off += 1+h.buf[off];
			intDeserialize<BYTES_PER_SECTOR_CODING>(h.buf + off, sector);
			off += BYTES_PER_SECTOR_CODING;
		}
	}
	
	return new Reader(t, m_data.get(), sector);
}

SmartPtr<OStreamI> FTree::rewrite()
	throw (trans_exception)
{
	return new Writer(this);
}

FTreeI* FTreeFactory(TransactionManagerI* tm, const std::string& name)
	throw (env_exception)
{
	assert(tm != 0);
	
	return new FTree(tm, name);
}

} // detail
} // JFA
