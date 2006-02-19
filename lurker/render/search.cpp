/*  $Id: search.cpp,v 1.18 2006-02-19 01:17:22 terpstra Exp $
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
#include <cerrno>
#include <cstring>
#include <cassert>
#include <algorithm>

#include <MessageId.h>
#include <XmlEscape.h>
#include <Keys.h>

#include "commands.h"
#include "Search.h"
#include "Cache.h"
#include "ConfigFile.h"

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
	
	while ((e = str.find_first_of("%+", b)) 
		!= string::npos)
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

int handle_search(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	cfg.options = req.options;
	
	string::size_type o = req.options.find('@');
	if (o == string::npos || o != MessageId::full_len)
		return search_format_error(param);
	
	if (!MessageId::is_full(req.options.c_str()))
		return search_format_error(param);
	
	vector<string> tokens;
	++o;
	
	MessageId id(req.options.c_str());
	string raw = decipherHalf(req.options.substr(o, string::npos));
	string keys(raw);
	// we need to translate '!' to '/'
	for (string::size_type es = 0; es < keys.length(); ++es)
		if (keys[es] == '!') keys[es] = '/';
		
	tokenize(keys, tokens, ",");
	
	// Right! Everything the user did is ok.
	
	vector<Summary> forward, backward, queue;
	
	Search backwardk(cfg, db, Backward, id);
	Search forwardk (cfg, db, Forward,  id);
	
	for (vector<string>::iterator i = tokens.begin(); i != tokens.end(); ++i)
	{
		string& key = *i;
		
		backwardk.keyword(key);
		forwardk.keyword(key);
	}
	
	if (!forwardk.pull(35, forward) || !backwardk.pull(35, backward))
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database search seek failure"), strerror(errno),
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "furth assistence."));
		return 1;
	}
	
	vector<Summary>::size_type left, right, i;
	if (forward.size() + backward.size() < 20)
	{
		left = backward.size();
		right = forward.size();
	}
	else if (forward.size() < 10)
	{
		right = forward.size();
		left = 20 - right;
	}
	else if (backward.size() < 10)
	{
		left = backward.size();
		right = 20 - left;
	}
	else
	{
		left = right = 10;
	}
	
	assert (left  <= backward.size());
	assert (right <= forward .size());
	
	for (i = left; i > 0; --i)  queue.push_back(backward[i-1]);
	for (i = 0; i < right; ++i) queue.push_back(forward[i]);
	
	string ok;
	for (i = 0; i < queue.size(); ++i)
		if ((ok = queue[i].load(db, cfg)) != "")
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
	
	Cache cache(cfg, "search", 
		param.substr(0, o) + 
		raw + "." +	// this is transformed so the webserver can eat it
		req.language + "." +
		req.ext, req.ext);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/search.xsl\"?>\n"
		<< "<search xml:lang=\"" << req.language << "\">\n"
		<< " " << cfg(req.language) << "\n"
		<< " <query>" << xmlEscape << keys << "</query>\n";
	
	if (right < forward.size())
	{	// we need a next link
		i = std::min(right+9, forward.size()-1);
		MessageId nd(forward[i].id());
		nd.increment(); // hope that it doesn't exist (-> skips one)
		cache.o << " <next>" << nd.serialize() << "</next>\n";
	}
	
	if (left < backward.size())
	{	// we need a prev link
		i = std::min(left+10, backward.size()-1);
		MessageId pd(backward[i].id());
		pd.increment();
		cache.o << " <prev>" << pd.serialize() << "</prev>\n";
	}
	
	for (i = 0; i < queue.size(); ++i)
		cache.o << " <row>" << queue[i] << "</row>\n";
	
	cache.o << "</search>\n";
	
	return 1;
}
