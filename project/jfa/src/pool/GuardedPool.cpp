/*  $Id: GuardedPool.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  GuardedPool.cpp - A sector-sized memory pool with reference counting
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
#include "GuardedPool.h"

#define MAX_SECTOR_QUEUE 128

namespace JFA
{
namespace detail
{

class GuardedPool : public PoolI
{
 private:
 	/* So, what's the plan?
 	 * 
 	 * The plan is that we want to give out memory to the caller which is
 	 * reference counted. When all the MemHandles to the memory are gone,
 	 * we take it back.
 	 * 
 	 * Now, MemHandles have a nice Observer.  We can point this a memory
 	 * guardian which reclaims the memory after everyone is done with it.
 	 * 
 	 * To save on calls to new/delete, we won't actually free the memory
 	 * since it is exactly the right size for the next caller.
 	 * 
 	 * We don't want to be super gready however, so if we have more than
 	 * a fixed number of unused memory blocks sitting idle, we free them
 	 * (and the associated guardian).
 	 * 
 	 * This whole scheme would be pointless if we had to allocate the
 	 * guardians... So, we don't! We keep two nice spliceable lists.
 	 * 
 	 * One of the lists holds all in-use guardians, the other all
 	 * the guardians who are hold free memory.
 	 * 
 	 * One final detail: an in-use guardian must hold an owning reference
 	 * to the environment so that we only cleanup after all the memory is
 	 * free. A free guardian does NOT have this property.
 	 */
 	struct MemoryGuardian : public Observable
 	{
 		SmartPtr<GuardedPool>	owner;
 		MemBlock<data_t>	memory;
 		
 		// This nasty detail is so we can move ourselves around
 		std::list<MemoryGuardian>::iterator	self;
 		
 		void init(GuardedPool* o,
 			std::list<MemoryGuardian>::iterator me);
 		void not_observed();
 		
 		MemoryGuardian();
 		
 		// this doesn't need to be virtual, but it keeps the
 		// older compilers happy
 		virtual ~MemoryGuardian();
 	};
 	
 	sectorlen_t			m_sector_size;
 	int				m_sectors_free;
 	
 	typedef std::list<MemoryGuardian> GuardianList;
 	
 	GuardianList	m_free_guardians;
 	GuardianList	m_used_guardians;
 	
 public:
 	GuardedPool(sectorlen_t sector_size);
 	~GuardedPool();
 	
 	MemHandle<data_t> get_a_sector_buffer();
 
 // This keeps older compilers happy also (even though it is unneeded!)
 friend class JFA::detail::GuardedPool::MemoryGuardian;
};

GuardedPool::MemoryGuardian::MemoryGuardian()
 : Observable(),
   owner(0),
   memory(0, 0)
{ }

GuardedPool::MemoryGuardian::~MemoryGuardian()
{
	if (memory.buf) delete [] memory.buf;
}

void GuardedPool::MemoryGuardian::init(
	GuardedPool* o,
	std::list<GuardedPool::MemoryGuardian>::iterator me)
{
	owner = o;
	
	memory.buf = new unsigned char [owner->m_sector_size];
	memory.len = owner->m_sector_size;
	
	self = me;
}

void GuardedPool::MemoryGuardian::not_observed()
{
	// There is a slight annoyance here.
	// We need to make sure our owner sticks around during this call;
	// that is, that we are not the last person to see him.
	
	SmartPtr<GuardedPool> boss = owner;
	
	// After the end of this function, our outstanding memory is no longer
	// a reason to keep the environment alive.
	owner = static_cast<GuardedPool*>(0);
	
	if (boss->m_sectors_free == MAX_SECTOR_QUEUE)
	{
		// don't add us to the list of free people if too big
		// This will destroy us so we cannot access member variables
		// beyond this function call
		boss->m_used_guardians.erase(self);
	}
	else
	{
		// move us to the free list
		++boss->m_sectors_free;
		boss->m_free_guardians.splice(
			boss->m_free_guardians.begin(),
			boss->m_used_guardians,
			self);
	}
}

GuardedPool::GuardedPool(sectorlen_t sector_size)
 : m_sector_size(sector_size), m_sectors_free(0)
{
}

GuardedPool::~GuardedPool()
{
	// If we had outstanding guardians, we couldn't get called!
	assert (m_used_guardians.empty());
	
	// Allow the normal stl destructor for the list
	// This will call the guardian destructor which will free the memory
}

MemHandle<data_t> GuardedPool::get_a_sector_buffer()
{
	if (m_free_guardians.empty())
	{
		m_used_guardians.push_front(MemoryGuardian());
		GuardianList::iterator i = m_used_guardians.begin();
		
		i->init(this, i);
		return MemHandle<data_t>(i->memory.buf, i->memory.len, &(*i));
	}
	else
	{
		GuardianList::iterator i = m_free_guardians.begin();
		--m_sectors_free;
		m_used_guardians.splice(
			m_used_guardians.begin(),
			m_free_guardians,
			i);
		
		// This memory needs to keep us alive as long as it is alive
		i->owner = this;
		return MemHandle<data_t>(i->memory.buf, i->memory.len, &(*i));
	}
}

PoolI* GuardedPoolFactory(sectorlen_t sector_size)
{
	return new GuardedPool(sector_size);
}

} // detail
} // JFA
