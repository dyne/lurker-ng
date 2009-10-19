/*  $Id$
 *  
 *  parse.cpp - Deal with CGI ugliness
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

#define _FILE_OFFSET_BITS 64

#include <iostream>
#include <cstdlib>

#include "parse.h"

inline int fromHex(char c)
{
	if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
	if (c >= 'a' && c <= 'z') return c - 'a' + 10;
	if (c >= '0' && c <= '9') return c - '0';
	return -1;
}

string decipherHalf(const string& str)
{
//	cout << "deciper: " << str << endl;
	
	string out;
	int high, low;
	
	string::size_type b = 0, e;
	while ((e = str.find_first_of("%+", b)) != string::npos)
	{
		out.append(str, b, e - b);
		if (str[e] == '+') out.append(" ");
		else if (str.length() > e+2 &&
		         (high = fromHex(str[e+1])) != -1 &&
		         (low = fromHex(str[e+2])) != -1)
		{
			int ch = high << 4 | low;
			out += ((char)ch);
			e += 2;
		}
		else
		{	// keep the broken escape char
			out.append("%");
		}
		
		b = e+1;
	}
	
	out.append(str, b, string::npos);
	
	return out;
}

pair<string, string> splitParam(const string& str)
{
//	cout << "split: " << str << endl;
	
	string::size_type split = str.find('=');
	if (split == string::npos)
		return pair<string, string>(decipherHalf(str), "");
	else	return pair<string, string>(
			decipherHalf(str.substr(0, split)),
			decipherHalf(str.substr(split+1, string::npos)));
}

map<string, string> getParams()
{
	map<string, string> out;
	
	const char* x = getenv("QUERY_STRING");
	
	string str = x?x:"";
	
//	cout << "parse: " << str << endl;
	
	string::size_type b = str.find_first_not_of('&', 0), e;
	if (b == string::npos) return out;
	
	while ((e = str.find_first_of('&', b)) != string::npos)
	{
		out.insert(splitParam(str.substr(b, e - b)));
		b = str.find_first_not_of('&', e+1);
		if (b == string::npos) break;
	}
	if (b != string::npos) 
		out.insert(splitParam(str.substr(b, str.length() - b)));
	
	return out;
}

map<string, string> getCookies()
{
	map<string, string> out;
	
	const char* x = getenv("HTTP_COOKIE");
	
	string str = x?x:"";
	
//	cout << "parse: " << str << endl;
	
	string::size_type b = str.find_first_not_of("; ", 0), e;
	if (b == string::npos) return out;
	
	while ((e = str.find_first_of("; ", b)) != string::npos)
	{
		out.insert(splitParam(str.substr(b, e - b)));
		b = str.find_first_not_of("; ", e+1);
		if (b == string::npos) break;
	}
	if (b != string::npos)
		out.insert(splitParam(str.substr(b, str.length() - b)));
	
	return out;
}

string uriEncode(const string& str)
{
	string out;
	static const char tbl[17] = "0123456789ABCDEF";
	
	string::size_type b = 0, e;
	while ((e = str.find_first_not_of(
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789~:/.,@-!", 
		b)) != string::npos)
	{
		out.append(str, b, e - b);
		
		out += '%';
		out += tbl[(str[e] >> 4) & 0xF];
		out += tbl[(str[e] >> 0) & 0xF];
		
		b = e+1;
	}
	
	out.append(str, b, str.length() - b);
	
	return out;
}

int redirectUrl(const string& url)
{
	string proto(url, 0, 11);
	for (string::size_type i = 0; i < proto.length(); ++i)
		if (proto[i] >= 'A' && proto[i] <= 'Z')
			proto[i] += 'a' - 'A';
	
	if (proto == "javascript:")
	{
		cout << "Status: 200 OK\r\n"
		     << "Content-type: text/plain\r\n\r\n"
		     << "Javascript go awaaayyyyyaaayyy! GO AWAY!\n";
	}
	else
	{
		cout << "Status: 303 Moved Permanently\r\n"
		     << "Location: " << uriEncode(url) << "\r\n"
		     << "Content-type: text/html\r\n\r\n"
		     << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		     << "<html><head>\r\n"
		     << "<title>301 Moved Permanently</title>\r\n"
		     << "</head><body>\r\n"
		     << "<h1>Moved Permanently</h1>\r\n"
		     << "The document has moved <a href=\"" << uriEncode(url) << "\">here</a>.\r\n"
		     << "<p><hr>\r\n"
		     << "</body></html>\r\n";
	}
	
	return 0;
}
