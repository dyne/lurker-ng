/*  $Id: search.cpp,v 1.2 2003-04-21 18:26:21 terpstra Exp $
 *  
 *  sindex.cpp - Handle a search/ command
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
#include <cerrno>
#include <cstring>
#include <cassert>

#include <MessageId.h>
#include <XmlEscape.h>
#include <Keys.h>

#include "commands.h"
#include "Search.h"
#include "Cache.h"

using namespace std;

int search_format_error(const string& param)
{
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << error(_("Bad request"), param,
		_("The given parameter was not of the correct format. "
		  "A searc request must be formatted like: "
		  "search/YYYYMMDD.HHMMSS.hashcode@word,word,word.xml"));
	return 1;
}

inline int fromHex(char c)
{
	if (c >= 'A' && c <= 'Z') return c - 'A' + 10;
	if (c >= 'a' && c <= 'z') return c - 'a' + 10;
	return c - '0';
}
                        
string decipherHalf(const string& str)
{
	// cout << "deciper: " << str << endl;
	
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

Search::Search(ESort::Reader* db_, const MessageId& laterThan)
 : db(db_), start(laterThan.serialize_backward()), eof(false)
{
}

string Search::keyword(const string& key)
{
	// cout << "need: " << key << "\n";
	
	string prefix = LU_KEYWORD LU_BACKWARD + key + '\0';
	entries.push_back(KEntry());
	entries.back().prefix = prefix;
	entries.back().walker = db->seek(prefix + start);
	
	if (entries.back().walker == 0)
	{
		eof = true;
		if (errno != 0)
			return string("Reader::seek:") + strerror(errno);
	}
	
	if (entries.back().walker->key.substr(0, prefix.length()) != prefix)
		eof = true;
	
	return "";
}

MessageId Search::top(Entries::iterator i)
{
	assert (i->walker->key.length() == i->prefix.length() + 8);
	MessageId id(i->walker->key.c_str()+i->prefix.length(), false);
	return id;
}

string Search::advance(Entries::iterator i)
{
	int j = i->walker->advance();
	if (j == -1)
	{
		eof = true;
		return string("Walker::advance:") + strerror(errno);
	}
	
	if (j < i->prefix.length())
	{
		eof = true;
		return "";
	}
	
	if (i->walker->key.length() != i->prefix.length() + 8)
	{
		eof = true;
		return string("corrupt keyword entry");
	}
	
	return "";
}

string Search::pull(int n, vector<Summary>& o)
{
	string ok;
	
	while (!eof && n)
	{
		Entries::iterator i, 
			small = entries.end(), 
			big   = entries.end();
		
		for (i = entries.begin(); i != entries.end(); ++i)
		{
			if (small == entries.end() || 
			    top(i) < top(small))
				small = i;
			if (big == entries.end() || 
			    top(i) > top(big))
				big = i;
		}
		
		if (top(small) == top(big))
		{
			o.push_back(Summary(top(big)));
			--n;
		}
		
		if ((ok = advance(big)) != "") return ok;
	}
	
	return "";
}

int handle_search(const Config& cfg, ESort::Reader* db, const string& param)
{
	string::size_type o = param.find('@');
	if (o == string::npos || o != 24)
		return search_format_error(param);
	
	MessageId id(param.c_str());
	
	if (id.timestamp() == 0)
		return search_format_error(param);
	
	string::size_type e = param.rfind('.');
	if (e == string::npos)
		return search_format_error(param);
	
	vector<string> tokens;
	++o;
	string keys = decipherHalf(param.substr(o, e-o));
	tokenize(keys, tokens, ",");
	
	// !!! this is actually somewhat ok, find a good base-case
	if (tokens.empty())
		return search_format_error(param);
	
	// Right! Everything the user did is ok.
	string ok;
	
	Search s(db, id);
	for (vector<string>::iterator i = tokens.begin(); i != tokens.end(); ++i)
		if ((ok = s.keyword(*i)) != "")
			break;
	
	if (ok != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database search push failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	vector<Summary> queue;
	
	if ((ok = s.pull(21, queue)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database search isect failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	vector<Summary>::size_type i;
	for (i = 0; i < queue.size(); ++i)
		if ((ok = queue[i].load(db)) != "")
			break;
	
	if (ok != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database search pull failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	Cache cache(cfg, "search", param);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/render.xsl\"?>\n"
		<< "<search>\n"
		<< " " << cfg << "\n"
		<< " <query>" << xmlEscape << keys << "</query>\n";
	
	vector<Summary>::size_type eof;
	if (queue.size() == 21)
	{
		eof = 20;
		cache.o << " <next>" << queue[20] << "</next>\n";
	}
	else	eof = queue.size();
	
	for (i = 0; i < eof; ++i)
		cache.o << " <row>" << queue[i] << "</row>\n";
	
	cache.o << "</search>\n";
	
	return 1;
}
