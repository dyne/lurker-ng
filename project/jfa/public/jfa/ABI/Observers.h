/*  $Id: Observers.h,v 1.1.1.1 2003-01-04 21:13:59 terpstra Exp $
 *  
 *  Observers.h - Inlined Observer implementation
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

#ifndef __JFA_ABI_OBSERVERS_H__
#define __JFA_ABI_OBSERVERS_H__

namespace JFA
{

namespace detail
{

class Observer;

/* You probably wonder why I have an observable class.
 * Well, it is because it is sometimes important to know when no one is looking.
 * At those times you can delete memory, change representation, or other things.
 */
class Observable
{
 private:
 	int observers;
 	
 	virtual void not_observed() = 0; // called when no longer observed
 
 public:
 	Observable() { observers = 0; }
 	
 	int get_observer_count() const { return observers; }
 
 friend class Observer;
};

/* An observer is just a handle that says you are observing something.
 */
class Observer
{
 protected:
 	Observable*	m_obj;
 	
 public:
 	Observer(Observable* o) : m_obj(o)
 	{ if (m_obj) ++m_obj->observers; }
 	
 	Observer(const Observer& o) : m_obj(o.m_obj)
 	{ if (m_obj) ++m_obj->observers; }
 	
 	~Observer()
 	{ if (m_obj && !--m_obj->observers) m_obj->not_observed(); }
 	
 	Observer& operator = (const Observer& o)
 	{
 		Observable* tmp = m_obj; // for exception-safety
 		m_obj = o.m_obj;
 		if (m_obj) ++m_obj->observers;
 		if (tmp && !--tmp->observers) tmp->not_observed();
 		return *this;
 	}
 	
 	bool is_only_observer() const
 	{ return m_obj->get_observer_count() == 1; }
};

} // detail
} // JFA

#endif
