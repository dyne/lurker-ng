/*  $Id: keyword.cpp,v 1.4 2003-04-27 18:57:21 terpstra Exp $
 *  
 *  jump.cpp - Jump to a given date offset
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

#include <keyword.h>
#include "parse.h"

#include <set>
#include <vector>

using std::set;

vector<string> keys;
set<string> dups;

int push_key(const char* keyword, void* arg)
{
	if (dups.find(keyword) != dups.end())
	{
		dups.insert(keyword);
		keys.push_back(keyword);
	}
	
	return 0;
}

int main()
{
	map<string, string> args = getParams();
	
	struct tm tms;
	memset(&tms, 0, sizeof(tms));
	
	tms.tm_sec  = args.find("sec" ) != args.end() ? atol(args["sec" ].c_str()) : 0;
	tms.tm_min  = args.find("min" ) != args.end() ? atol(args["min" ].c_str()) : 0;
	tms.tm_hour = args.find("hour") != args.end() ? atol(args["hour"].c_str()) : 0;
	tms.tm_mday = args.find("mday") != args.end() ? atol(args["mday"].c_str()) : 1;
	tms.tm_mon  = args.find("mon" ) != args.end() ? atol(args["mon" ].c_str()) - 1 : 0;
	tms.tm_year = args.find("year") != args.end() ? atol(args["year"].c_str()) - 1900 : 138;
	
	char buf[26];
	strftime(buf, 25, "%Y%m%d.%H%M%S", &tms);
	
	my_keyword_digest_string(
		args["subject"].c_str(), args["subject"].length(), 
		LU_KEYWORD_SUBJECT, &push_key, 0, 0);
	my_keyword_digest_string(
		args["author"].c_str(), args["author"].length(), 
		LU_KEYWORD_AUTHOR, &push_key, 0, 0);
	my_keyword_digest_string(
		args["query"].c_str(), args["query"].length(), 
		LU_KEYWORD_WORD, &push_key, 0, 0);
	my_keyword_digest_string(
		args["list"].c_str(), args["list"].length(), 
		LU_KEYWORD_LIST, &push_key, 0, 0);
	
	string url = args["doc-url"] + "/search/" + string(buf) + ".00000000@";
	vector<string>::iterator i;
	for (i = keys.begin(); i != keys.end(); ++i)
	{
		if (i != keys.begin()) url += ",";
		url += *i;
	}
	url += "." + args["format"];
	
	return redirectUrl(url);
}
