/*  $Id: EnvironmentI.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  EnvironmentI.h - Interface into the library for Environments
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

#ifndef __JFA_ABI_ENVIRONMENT_I_H__
#define __JFA_ABI_ENVIRONMENT_I_H__

#include <jfa/ABI/TransactionI.h>

namespace JFA
{

/* If you ever get a corrupt_exception, then you must cease all database
 * use, report the error to the user, and only then recreate the environment
 * and start over.
 */
class env_exception : public jfa_exception
{
 public:
 	env_exception(const char* f, int l, const char* w)
 	 : jfa_exception(f, l, w) { }
};

class corrupt_exception : public env_exception
{
 public:
 	corrupt_exception(const char* f, int l, const char* w)
 	 : env_exception(f, l, w) { }
 	 
 	const char* detail() const throw();
};

namespace detail
{

class EnvironmentI : public MemoryManaged
{
 public:
 	// This should be wrapped immediately in a smart ptr
 	virtual TransactionI* open_transaction() = 0;
};

SmartPtr<EnvironmentI> FileSystemFactory(const std::string& path, int mode) throw(env_exception);
// future direction:
//SmartPtr<EnvironmentI>  RawDeviceFactory(const std::string& device) throw(env_exception);
//SmartPtr<EnvironmentI>       LVM1Factory(const std::string& device) throw(env_exception);
//SmartPtr<EnvironmentI>       LVM2Factory(const std::string& device) throw(env_exception);

// Magically guess what the path refers to
SmartPtr<EnvironmentI>      GuessFactory(const std::string& something) throw(env_exception);

} // detail
} // JFA

#endif
