/*  $Id: Exception.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  Exception.cpp - Provide the user-visible thrown exceptions
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

#include <sstream>

#include <jfa/ABI/Exceptions.h>
#include <jfa/ABI/EnvironmentI.h>
#include <jfa/ABI/TransactionI.h>

namespace JFA
{

std::string jfa_exception::desc() const throw()
{
	std::ostringstream s;
	s << file << ":" << line << ":" << detail() << ": " << why;
	return s.str();
}

const char*       failed_exception::detail() const throw() { return "transaction failed"; }
const char*  read_locked_exception::detail() const throw() { return  "read_locked"; }
const char* write_locked_exception::detail() const throw() { return "write_locked"; }
const char*     io_retry_exception::detail() const throw() { return "io_retry"; }

const char*      corrupt_exception::detail() const throw() { return "corrupt environment"; }

}
