/*  $Id: Exceptions.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  Exceptions.h - User visible exception base class
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

#ifndef __JFA_ABI_EXCEPTIONS_H__
#define __JFA_ABI_EXCEPTIONS_H__

#include <stdexcept>
#include <string>

namespace JFA
{

/* I hereby promise that I will never throw an exception during destruction
 * of an object. (at least that any user of this library can see)
 *
 * Yes the jfa_ is redundant given JFA:: but, we don't want it to conflict
 * with the std:: namespace in case the user uses both.
 */
class jfa_exception : public std::exception
{
 public:
 	const char*	file;
 	int		line;
 	const char*	why;
 	
 	jfa_exception(const char* f, int l, const char* w)
 	 : file(f), line(l), why(w) { }
 	
 	std::string desc() const throw();
 	 	
 	virtual const char* detail() const throw() = 0;
};

} // JFA

#endif
