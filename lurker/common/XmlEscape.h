/*  $Id: XmlEscape.h,v 1.2 2003-04-21 18:25:31 terpstra Exp $
 *  
 *  XmlEscape.h - A stream manipulator-like thing for escaping XML
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: GPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; version 2.1.
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

#ifndef XML_ESCAPE_H
#define XML_ESCAPE_H

#include <iostream>
#include <string>

using std::ostream;
using std::string;

// The empty type used to trigger our code.
extern struct XmlEscape { } xmlEscape;

class XmlOstream
{
 protected:
 	ostream& o;
 
 public:
 	XmlOstream(ostream& o_) : o(o_) { }
 	
 	ostream& operator << (const string& s);
 	ostream& operator << (const char* s);
 	ostream& operator << (char c);
};

inline XmlOstream operator << (ostream& o, const XmlEscape& e)
{
	return XmlOstream(o);
}

#endif
