/*  $Id: CharsetEscape.cpp,v 1.11 2003-06-23 15:28:10 terpstra Exp $
 *  
 *  CharsetEscape.cpp - A stream manipulator-like thing for charset conversion
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
#define _FILE_OFFSET_BITS 64

#include <mimelib/string.h>
#include <mimelib/utility.h>

#include <cerrno>
#if __GNUC__ == 2
#include <strstream>
#else
#include <sstream>
#endif

#include "CharsetEscape.h"

CharsetEscape::CharsetEscape(const char* charset)
 : ic(iconv_open("UTF-8", charset))
{
}

CharsetEscape::~CharsetEscape()
{
	if (valid()) iconv_close(ic);
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
	if (!valid())
	{	// when not valid, just keep ascii chars
	
		while (1)
		{
			const char* s;
			const char* e;
			
			for (s = ib, e = s + is; s != e; ++s)
			{	// if it moves, kill it!
				if ((*s < 0x20 || *s >= 0x7f) &&
				    (*s != '\n' && *s != '\t'))
				{
					break;
				}
			}
			
			// write out what we have
			if (s != ib) o.write(ib, long(s - ib));
			
			is -= long(s - ib);
			ib = s;
			
			if (!is) return;
			
			// skip the offensive byte
			++ib;
			--is;
			o << '?';
		}
	}
	
	char buf[8096];
	
	char*		ob = &buf[0];
	size_t		os = sizeof(buf);
	
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

string CharsetEscape::write(const char* ib, size_t is)
{
#if __GNUC__ == 2
	strstream out;
#else
	std::stringstream out;
#endif
	write(out, ib, is);
	
#if __GNUC__ == 2
	char* tmpstr = out.str();
	string ret(tmpstr, out.rdbuf()->pcount());
	free(tmpstr);
	return ret;
#else
	return out.str();
#endif
}

// Transform any =?charset?encoding?str?= stuff in the string to utf-8
string decode_header(
	const string&	str,
	const char*	default_coding)
{
#if __GNUC__ == 2
	strstream out;
#else
	std::stringstream out;
#endif
	
	CharsetEscape code(default_coding);
	
	string::size_type b = 0, c, e, s;
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
	
#if __GNUC__ == 2
	char* tmpstr = out.str();
	string ret(tmpstr, out.rdbuf()->pcount());
	free(tmpstr);
	return ret;
#else
	return out.str();
#endif
}
