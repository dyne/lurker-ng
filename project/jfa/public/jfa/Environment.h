/*  $Id: Environment.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  Environment.h - User visible Environment wrapper
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

#ifndef __JFA_ENVIRONMENT_H__
#define __JFA_ENVIRONMENT_H__

#include <jfa/ABI/EnvironmentI.h>

namespace JFA
{

class Environment
{
 protected:
 	detail::SmartPtr<detail::EnvironmentI>	interface;
 
 public:
 	Environment(const detail::SmartPtr<detail::EnvironmentI>& env) : interface(env)
 	{ }
 	
 	// NOT FOR USERS!
 	detail::EnvironmentI* _get_internal() const { return interface.get(); }
};

Environment Guess(const std::string& something) throw (env_exception)
{ return detail::GuessFactory(something); }

Environment FileSystem(const std::string& path, int mode = 0644) throw (env_exception)
{ return detail::FileSystemFactory(path, mode); }

// future direction:
//Environment RawDevice(const std::string& device) throw (env_exception)
//{ return detail::RawDeviceFactory(device); }

//Environment LVM1(const std::string& device) throw (env_exception)
//{ return detail::LVM1Factory(device); }

//Environment LVM2(const std::string& device) throw (env_exception)
//{ return detail::LVM2Factory(device); }

}

#endif
