/*  $Id: CharsetEscape.h,v 1.2 2003-04-21 18:25:31 terpstra Exp $
 *  
 *  CharsetEscape.h - A stream manipulator-like thing for charset conversion
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

#ifndef CHARSET_ESCAPE_H
#define CHARSET_ESCAPE_H

#include <iostream>
#include <string>

#include <iconv.h>
#include <cstring>

using std::ostream;
using std::string;

/** Escape a charset into utf-8
 */
class CharsetEscape
{
 protected:
 	iconv_t	ic;
 	
 public:
 	// If the charset is invalid, iso-8859-1 is used instead
 	CharsetEscape(const char* charset);
 	~CharsetEscape();
 	
 	void write(ostream& o, const char* s, size_t amt);
 	
 	void write(ostream& o, const string& s)
 	{
 		write(o, s.c_str(), s.length());
 	}
 	
 	void write(ostream& o, const char* s)
 	{
 		write(o, s, strlen(s));
 	}
 	
 	void write(ostream& o, char c)
 	{
 		write(o, &c, 1);
 	}
};

class CharsetOstream
{
 protected:
 	ostream&	o;
 	CharsetEscape&	e;
 
 public:
 	CharsetOstream(ostream& o_, CharsetEscape& e_) : o(o_), e(e_) { }
 	
 	ostream& operator << (const string& s) { e.write(o, s); return o; }
 	ostream& operator << (const char* s)   { e.write(o, s); return o; }
 	ostream& operator << (char c)          { e.write(o, c); return o; }
};

inline CharsetOstream operator << (ostream& o, CharsetEscape& e)
{
	return CharsetOstream(o, e);
}

// Transform any =?charset?encoding?str?= stuff in the string to utf-8
string decode_header(
	const string&	str,
	const char*	default_coding);

#endif
