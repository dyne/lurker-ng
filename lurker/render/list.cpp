/*  $Id: list.cpp,v 1.10 2004-08-25 14:30:40 terpstra Exp $
 *  
 *  list.cpp - Handle a list/ command
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

#include <iostream>
#include <cerrno>
#include <cassert>
#include <ctime>

#include <map>
#include <vector>

#include <MessageId.h>
#include <XmlEscape.h>
#include <Keys.h>
#include <esort.h>

#include "commands.h"
#include "Cache.h"
#include "Summary.h"

using namespace std;

#define NUM_TOPICS	20
#define NUM_DAYS	14

int list_format_error(const string& param)
{
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << error(_("Bad request"), param,
		_("The given parameter was not of the correct format. "
		  "A list request must be formatted like: "
		  "list/listid.xml where listid "
		  "is the id of an indexed mailing list."));
	return 1;
}

int list_load_error(const string& ok)
{
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << error(_("Database list pull failure"), ok,
		_("Something internal to the database failed. "
		  "Please contact the lurker user mailing list for "
		  "further assistence."));
	return 1;
}

// nested types break g++ 2.95
struct NewTopic
{
	bool		pushed;
	Summary		newest;
	vector<int>	days;
};

static time_t now;

int load_topic(const Config& cfg, ESort::Reader* db, const string& hash, NewTopic& t)
{
	string prefix = LU_KEYWORD LU_KEYWORD_THREAD + hash + '\0';
	auto_ptr<ESort::Walker> dayWalker(db->seek(
		prefix, "\xFF\xFF\xFF\xFF",
		ESort::Backward));
	
	// set it up for num days
	t.days.resize(NUM_DAYS, 0);
	t.pushed = false;
	
	while (dayWalker->advance() != -1)
	{	// check corrupt
		if (dayWalker->key.length() != prefix.length() + 8) break;
		
		MessageId id(dayWalker->key.c_str() + prefix.length(), 8);
		
		// this is the newest if none has been set
		if (!t.newest.loaded())
		{
			Summary sum(id);
			string ok = sum.load(db, cfg);
			if (ok != "") return list_load_error(ok);
			if (!sum.deleted()) t.newest = sum;
		}
		
		int daygap = (now - id.timestamp()) / (60*60*24);
		if (daygap >= NUM_DAYS)
		{
			if (t.newest.loaded()) break;
		}
		else
			++t.days[daygap];
	}
	
	return 0;
}
	
int handle_list(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	cfg.options = req.options;
	
	if (cfg.lists.find(req.options) == cfg.lists.end())
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("No such list"), req.options,
			_("The specified mailing list is not available in this "
			  "archive. Perhaps you misspelled it or went to the "
			  "wrong server?"));
		return 1;
	}
	
	const List& list = cfg.lists.find(req.options)->second;
	
	// Right! Everything the user did is ok.
	
	map<string, NewTopic>	topics;
	vector<string>		order;
	
	auto_ptr<ESort::Walker> topicFinder(db->seek(
		LU_NEW_TOPICS + list.mbox + '\0',
		"\xFF\xFF\xFF\xFF",
		ESort::Backward));
	
	string::size_type skip = 1 + list.mbox.length() + 1;
	
	now = time(0);
	
	while (order.size() < NUM_TOPICS && topicFinder->advance() != -1)
	{	// check corrupt
		if (topicFinder->key.length() != skip + 4 + 8) break;
		
		const unsigned char* tss = 
			reinterpret_cast<const unsigned char*>
			(topicFinder->key.c_str() + skip);
		time_t when =	(time_t)tss[0] << 24 | 
				(time_t)tss[1] << 16 |
				(time_t)tss[2] << 8 |
				(time_t)tss[3];
		
		string hash(topicFinder->key, skip + 4, 8);
		
		// not already loaded?
		if (topics.find(hash) == topics.end() &&
		    load_topic(cfg, db, hash, topics[hash]) != 0)
			return 1;
		
		// Is this point in time the first (non-deleted) hit?
		NewTopic& t = topics[hash];
		if (t.newest.loaded() && // does the thread have any hits?
		    t.newest.id().timestamp() == when &&
		    !t.pushed)
		{
			t.pushed = true;
			order.push_back(hash);
		}
	}
	
	Cache cache(cfg, "list", param, req.ext);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/list.xsl\"?>\n"
		<< "<list xml:lang=\"" << req.language << "\">\n"
		<< " " << cfg << "\n"
		<< " " << list << "\n";
	
	for (vector<string>::iterator i = order.begin(); i != order.end(); ++i)
	{
		cache.o << " <row>\n";
		
		NewTopic& t = topics[*i];
		vector<int>::iterator j;
		cache.o << "  <title>" << xmlEscape << skipSubjectStart(t.newest.subject().c_str()) << "</title>\n"
		        << "  ";
		for (j = t.days.begin(); j != t.days.end(); ++j)
			cache.o << "<day>" << *j << "</day>";
		cache.o << "\n";
		
		cache.o << "  " << t.newest << "\n";
		cache.o << " </row>\n";
	}
	
	cache.o << "</list>\n";
	
	return 0;
}
