/*  $Id$
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

int pull_allowed(const Config& cfg, ESort::Reader* db, vector<Summary>& v, Search& s)
{
	string ok;
	vector<Summary>::size_type i;
	
	while (v.size() < 35)
	{
		i = v.size();
		if (!s.pull(1, v))
			error(_("Database search seek failure"), strerror(errno),
			      _("Something internal to the database failed. "
			        "Please contact the lurker user mailing list for "
			        "furth assistence."));
		
		if (i == v.size()) break; // no more data
		if ((ok = v[i].load(db, cfg)) != "")
			error(_("Database search pull failure"), ok,
			      _("Something internal to the database failed. "
			        "Please contact the lurker user mailing list for "
			        "further assistence."));
		
		// trim forbidden fruit
		if (!v[i].allowed() || v[i].deleted()) v.resize(i);
	}
	return 0;
}

int handle_search(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	cfg.options = req.options;
	
	string::size_type o = req.options.find('@');
	if (o == string::npos || o != MessageId::full_len ||
	    !MessageId::is_full(req.options.c_str()))
		error(_("Bad request"), param,
		      _("The given parameter was not of the correct format. "
		        "A searc request must be formatted like: "
		        "search/YYYYMMDD.HHMMSS.hashcode@word,word,word.xml"));
	
	vector<string> tokens;
	++o;
	
	MessageId id(req.options.c_str());
	string keys = req.options.substr(o, string::npos);
	// we need to translate '!' to '/'
	for (string::size_type es = 0; es < keys.length(); ++es)
		if (keys[es] == '!') keys[es] = '/';
		
	tokenize(keys, tokens, ",");
	
	// Right! Everything the user did is ok.
	
	vector<Summary> forward, backward, queue;
	
	Search backwardk(cfg, db, Backward, id, false);
	Search forwardk (cfg, db, Forward,  id, false);
	
	for (vector<string>::iterator i = tokens.begin(); i != tokens.end(); ++i)
	{
		string& key = *i;
		
		backwardk.keyword(key);
		forwardk.keyword(key);
	}
	
	if (pull_allowed(cfg, db, forward,  forwardk)  != 0) return 1;
	if (pull_allowed(cfg, db, backward, backwardk) != 0) return 1;
	
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
	
	Cache cache(cfg, "search", param, req.ext);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../ui/search.xsl\"?>\n"
		<< "<search xml:lang=\"" << req.language << "\">\n"
		<< " <mode>" << req.ext << "</mode>\n"
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
