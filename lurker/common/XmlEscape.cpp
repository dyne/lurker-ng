/*  $Id: XmlEscape.cpp,v 1.4 2003-04-22 13:52:39 terpstra Exp $
 *  
 *  XmlEscape.cpp - A stream manipulator-like thing for escaping XML
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

#include "XmlEscape.h"

#include <cstring>

XmlEscape xmlEscape;

ostream& XmlOstream::operator << (char c)
{
	if (c >= 0 && c <= 0x1f)
	{
		switch (c)
		{
		case '\n':	return o << "<br/>\n";
		case '\t':	return o << " ";
		default:	return o; // drop the char
		}
	}
	else
	{
		switch (c)
		{
		case '\'':	return o << "&apos;";
		case '<':	return o << "&lt;";
		case '>':	return o << "&gt;";
		case '"':	return o << "&quot;";
		case '&':	return o << "&amp;";
		default:	return o << c; // leave it alone
		}
	}
}

ostream& XmlOstream::operator << (const string& s)
{
	string::size_type b = 0, e;
	while ((e = s.find_first_of("'<>\"&\n", b)) != string::npos)
	{
		o.write(s.c_str() + b, e - b);
		*this << s[e];
		b = e+1;
	}
	
	return o.write(s.c_str() + b, s.length() - b);
}

ostream& XmlOstream::operator << (const char* s)
{
	while (1)
	{
		size_t bad = strcspn(s, "'<>\"&\n");
		o.write(s, bad);
		s += bad;
		
		if (!*s) return o;
		*this << *s;
		++s;
	}
}
