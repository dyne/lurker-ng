/*  $Id: CharsetEscape.cpp,v 1.2 2003-04-21 18:25:31 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

#include <mimelib/string.h>
#include <mimelib/utility.h>

#include <sstream>
#include <cerrno>

#include "CharsetEscape.h"

CharsetEscape::CharsetEscape(const char* charset)
 : ic(iconv_open("utf-8", charset))
{
	if (ic == (iconv_t)-1)
		ic = iconv_open("utf-8", "iso-8859-1");
}

CharsetEscape::~CharsetEscape()
{
	iconv_close(ic);
}

void iconv_bug_kill_nulls(char* ob, size_t is)
{
	while (is != 0)
	{
		if (*ob == '\0') *ob = '?';
		++ob;
		--is;
	}
}

void CharsetEscape::write(ostream& o, const char* ib, size_t is)
{
	char buf[8096];
	
	char*		ob = &buf[0];
	size_t		os = sizeof(buf);
	
	size_t		ret;
	
	// We forcibly type-cast iconv b/c it has different types on some
	// platforms, but the difference is only in the const.
	while (((size_t (*)(iconv_t, const char **, size_t*, char**, size_t*))&iconv)
		(ic, &ib, &is, &ob, &os) == (size_t)-1)
	{
		if (errno == EILSEQ)
		{
			// Output some stuff
			iconv_bug_kill_nulls(buf, sizeof(buf) - os);
			o.write(buf, sizeof(buf) - os);
			
			ob = &buf[0];
			os = sizeof(buf);
			
			// skip a broken byte
			++ib;
			--is;
			o << "?";
		}
		else if (errno == EINVAL)
		{
			// Incomplete data
			break;
		}
		else
		{	// E2BIG
			iconv_bug_kill_nulls(buf, sizeof(buf) - os);
			o.write(buf, sizeof(buf) - os);
			
			ob = &buf[0];
			os = sizeof(buf);
		}
	}
	
	// success, write out tail.
	iconv_bug_kill_nulls(buf, sizeof(buf) - os);
	o.write(buf, sizeof(buf) - os);
}

// Transform any =?charset?encoding?str?= stuff in the string to utf-8
string decode_header(
	const string&	str,
	const char*	default_coding)
{
	std::stringstream out;
	
	CharsetEscape code(default_coding);
	
	string::size_type len = str.length();
	string::size_type b = 0, c, e, s, w;
	while ((c = str.find("=?", b)) != string::npos)
	{
		code.write(out, str.c_str() + b, c - b);
		
		if ((e = str.find('?',  c+2)) != string::npos &&
		    (s = str.find('?',  e+1)) != string::npos &&
		    s == e + 2 &&
		    (b = str.find("?=", s+1)) != string::npos &&
		    str.find_first_of(" \t", c) > b)
		{	// valid escape
			c += 2;
			string charset(str, c, e - c);
			char encoding = str[e+1];
			s += 1;
			DwString in(str.c_str() + s, b-s);
			DwString decode;
			b += 2;
			
			if (encoding == 'Q' || encoding == 'q')
			{
				// Convert also all '_' to ' '
				size_t x = 0;
				while ((x = in.find_first_of("_", x)) != DwString::npos)
				{
					in[x] = ' ';
					++x;
				}
				
				DwDecodeQuotedPrintable(in, decode);
			}
			if (encoding == 'B' || encoding == 'b')
			{
				DwDecodeBase64(in, decode);
			}
			
			CharsetEscape subcode(charset.c_str());
			subcode.write(out, decode.c_str(), decode.length());
		}
		else
		{	// not valid escape
			code.write(out, "=?", 2);
			b = c+2;
		}
	}
	
	code.write(out, str.c_str() + b, str.length() - b);
	return out.str();
}
