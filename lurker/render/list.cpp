/*  $Id: list.cpp,v 1.4 2003-06-23 14:38:43 terpstra Exp $
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
#include <Keys.h>
#include <esort.h>

#include "commands.h"
#include "Cache.h"
#include "KeyReader.h"

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
	Summary		newest;
	vector<int>	days;
};
	
int handle_list(const Config& cfg, ESort::Reader* db, const string& param)
{
	string::size_type o = param.find('.');
	if (o == string::npos)
		return list_format_error(param);
	
	string listn(param, 0, o);
	
	if (cfg.lists.find(listn) == cfg.lists.end())
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("No such list"), listn,
			_("The specified mailing list is not available in this "
			  "archive. Perhaps you misspelled it or went to the "
			  "wrong server?"));
		return 1;
	}
	
	const List& list = cfg.lists.find(listn)->second;
	
	// Right! Everything the user did is ok.
	
	map<string, NewTopic>	topics;
	vector<string>		order;
	
	auto_ptr<ESort::Walker> topicFinder(db->seek(
		LU_NEW_TOPICS + list.mbox + '\0',
		"\xFF\xFF\xFF\xFF",
		ESort::Backward));
	
	string::size_type skip = 1 + list.mbox.length() + 1 + 4;
	while (order.size() < NUM_TOPICS && topicFinder->advance() != -1)
	{	// check corrupt
		if (topicFinder->key.length() != skip + 8) break;
		string hash(topicFinder->key, skip, 8);
		
		if (topics.find(hash) == topics.end())
		{
			order.push_back(hash);
			topics[hash]; // prep it
		}
	}
	
	// Now that we have some topics, count the messages by day
	time_t now = time(0);
	
	vector<string>::iterator i;
	for (i = order.begin(); i != order.end(); ++i)
	{
		NewTopic& t = topics[*i];
		string prefix = LU_KEYWORD LU_KEYWORD_THREAD + *i + '\0';
		auto_ptr<ESort::Walker> dayWalker(db->seek(
			prefix, "\xFF\xFF\xFF\xFF",
			ESort::Backward));
		
		// set it up for num days
		t.days.resize(NUM_DAYS, 0);
		
		while (dayWalker->advance() != -1)
		{	// check corrupt
			if (dayWalker->key.length() != prefix.length() + 8) break;
			
			MessageId id(dayWalker->key.c_str() + prefix.length(), 8);
			
			// this is the newest if none has been set
			if (t.newest.id().timestamp() == 0)
			{
				t.newest = Summary(id);
				string ok = t.newest.load(db);
				if (ok != "") return list_load_error(ok);
			}
			
			int daygap = (now - id.timestamp()) / (60*60*24);
			if (daygap >= NUM_DAYS) break;
			
			++t.days[daygap];
		}
	}
	
	Cache cache(cfg, "list", param);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/list.xsl\"?>\n"
		<< "<list>\n"
		<< " " << cfg << "\n"
		<< " " << list << "\n";
	
	for (i = order.begin(); i != order.end(); ++i)
	{
		cache.o << " <row>\n";
		
		NewTopic& t = topics[*i];
		vector<int>::iterator j;
		cache.o << "  ";
		for (j = t.days.begin(); j != t.days.end(); ++j)
			cache.o << "<day>" << *j << "</day>";
		cache.o << "\n";
		
		cache.o << "  " << t.newest << "\n";
		cache.o << " </row>\n";
	}
	
	cache.o << "</list>\n";
	
	return 0;
}
