/*  $Id: XmlEscape.cpp,v 1.5 2003-05-03 19:29:17 terpstra Exp $
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
		default:	return o << "?"; // drop the char
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
				// this case includes high-ascii utf-8
		}
	}
}

string::size_type find_first_offensive_byte(const char* s, const char* e)
{
	const char* b = s;
	while (s != e)
	{
		char x = *s;
		if (x >= 0 && x <= 0x1f)
		{	// control char in utf-8 eh?
			return s - b;
		}
		else
		{
			switch (x)
			{
			case '\'':
			case '<':
			case '>':
			case '"':
			case '&':
				// xml doesn't like these dudes
				return s - b;
			}
		}
		
		++s;
	}
	
	return s - b; // the eos
}

ostream& XmlOstream::operator << (const string& s)
{
	string::size_type b = 0, e = s.length();
	while (1)
	{
		string::size_type x = find_first_offensive_byte(
			s.c_str() + b, s.c_str() + e) + b;
		
		o.write(s.c_str() + b, x - b);
		if (x == e) return o;
		
		*this << s[x];
		b = x+1;
	}
}

ostream& XmlOstream::operator << (const char* s)
{
	while (1)
	{
		string::size_type x = find_first_offensive_byte(s, 0);
		
		o.write(s, x);
		s += x;
		if (!*s) return o;
		
		*this << *s;
		++s;
	}
}
