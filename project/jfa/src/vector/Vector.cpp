/*  $Id: Vector.cpp,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  Vector.cpp - A trivial raw block-device for use in simple wrappers
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

#include <jfa/ABI/VectorI.h>

#include "../TransactionManagerI.h"

namespace JFA
{
namespace detail
{

class Vector : public VectorI
{
 protected:
 	SmartPtr<PromiseKeeperI> m_keeper;
 	
 public:
 	Vector(TransactionManagerI* tm, const std::string& name)
 		throw (env_exception);
 	
 	MemHandle<data_t>       get_rw(TransactionI* t, recordptr_t sect) throw (trans_exception);
 	MemHandle<const data_t> get_ro(TransactionI* t, recordptr_t sect) throw (trans_exception);
 	void                  truncate(TransactionI* t, recordptr_t sect) throw (trans_exception);
 	
 	sectorlen_t		get_sector_size() const throw ();
};

Vector::Vector(TransactionManagerI* tm, const std::string& name)
	throw (env_exception)
 : m_keeper(tm->open_volume(name))
{
}

MemHandle<data_t> Vector::get_rw(TransactionI* t, recordptr_t sect)
	throw (trans_exception)
{
	return m_keeper->get_rw(t, sect);
}

MemHandle<const data_t> Vector::get_ro(TransactionI* t, recordptr_t sect)
	throw (trans_exception)
{
	return m_keeper->get_ro(t, sect);
}

void Vector::truncate(TransactionI* t, recordptr_t sect)
	throw (trans_exception)
{
	return m_keeper->truncate(t, sect);
}

sectorlen_t Vector::get_sector_size() const throw ()
{
	return m_keeper->get_sector_size();
}

SmartPtr<VectorI> VectorFactory(EnvironmentI* env, const std::string& name)
	throw (env_exception)
{
	TransactionManagerI* tm = dynamic_cast<TransactionManagerI*>(env);
	assert(tm != 0);
	
	return new Vector(tm, name);
}

} // detail
} // JFA
