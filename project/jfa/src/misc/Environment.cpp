/*  $Id: Environment.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  Environment.cpp - Provide the user visible Environment constructors
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

#include <jfa/ABI/EnvironmentI.h>

#include "../simple/Simple.h"
#include "../pool/GuardedPool.h"
#include "../filesystem/FileSystem.h"

#define SECTOR_SIZE 8192

namespace JFA
{
namespace detail
{

SmartPtr<EnvironmentI> FileSystemFactory(const std::string& path, int mode)
	throw(env_exception)
{
	VolumeManagerI*	vm   = FileSystemVolumeManagerFactory(path, mode, SECTOR_SIZE);
	PoolI*          pool = GuardedPoolFactory(SECTOR_SIZE);
	
	return SimpleTransactionManagerFactory(vm, pool);
}

SmartPtr<EnvironmentI> GuessFactory(const std::string& path)
	throw(env_exception)
{
	return FileSystemFactory(path, 0644);
}

} //detail
} // JFA
