/*  $Id: parse.cpp,v 1.2 2003-04-21 18:26:21 terpstra Exp $
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

#include <iostream>

#include "parse.h"

inline int fromHex(char c)
{
	if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
	if (c >= 'a' && c <= 'z') return c - 'a' + 10;
	return c - '0';
}

string decipherHalf(const string& str)
{
//	cout << "deciper: " << str << endl;
	
	string out;
	
	string::size_type b = 0, e;
	while ((e = str.find_first_of("%+", b)) != string::npos)
	{
		out.append(str, b, e - b);
		if (str[e] == '+') out.append(" ");
		else if (str.length() > e+2)
		{
			int ch = fromHex(str[e+1]) << 4 | fromHex(str[e+2]);
			out += ((char)ch);
			e += 2;
		}
		
		b = e+1;
	}
	
	out.append(str, b, str.length() - b);
	
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
	
	char* x = getenv("QUERY_STRING");
	
	string str = x?x:"";
	
//	cout << "parse: " << str << endl;
	
	string::size_type b = str.find_first_not_of('&', 0), e;
	if (b == string::npos) return out;
	
	while ((e = str.find_first_of('&', b)) != string::npos)
	{
		out.insert(splitParam(str.substr(b, e - b)));
		b = str.find_first_not_of('&', e+1);
	}
	out.insert(splitParam(str.substr(b, str.length() - b)));
	
	return out;
}

int redirectUrl(const string& url)
{
	char* u = getenv("REQUEST_URI");
	string uri = u?u:"";
	
	char* n = getenv("SERVER_NAME");
	char* p = getenv("SERVER_PORT");
	
	if (n)
	{
		string host = "http://" + string(n);
		if (p) host += ":" + string(p);
		uri = host + uri;
	}
	
	// Prune off first '?'
	string::size_type x = uri.find('?');
	if (x != string::npos) uri.resize(x);
	
	// Prune off last '/'
	string::size_type y = uri.rfind('/');
	if (y != string::npos) uri.resize(y);
	
	// It is now relative to the request
	
	cout << "Status: 303 Moved Permanently\r\n"
	     << "Location: " << uri << "/" << url << "\r\n"
	     << "Content-type: text/html\r\n\r\n"
	     << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
	     << "<html><head>\r\n"
	     << "<title>301 Moved Permanently</title>\r\n"
	     << "</head><body>\r\n"
	     << "<h1>Moved Permanently</h1>\r\n"
	     << "The document has moved <a href=\"" << uri << "/" << url << "\">here</a>.\r\n"
	     << "<p><hr>\r\n"
	     << "</body></html>\r\n";
	
	return 0;
}
