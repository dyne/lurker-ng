/*  $Id: thread.cpp,v 1.12 2006-02-21 12:49:36 terpstra Exp $
 *  
 *  thread.cpp - Handle a thread/ command
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

#include <Keys.h>

#include "commands.h"
#include "Threading.h"
#include "Cache.h"

int handle_thread(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	cfg.options = req.options;
	
	if (!MessageId::is_full(req.options.c_str()) ||
	    req.options.length() != MessageId::full_len)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Bad request"), param,
			_("The given parameter was not of the correct format. "
			  "A thread request must be formatted like: "
			  "thread/YYYYMMDD.HHMMSS.hashcode.xml"));
		return 1;
	}
	
	MessageId id(req.options.c_str());
	string ok;
	
	Summary source(id);
	if ((ok = source.load(db, cfg)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database thread source pull failure"), ok,
			_("The specified message does not exist."));
		return 1;
	}
	
	Threading::Key spot;
	Threading thread;
	if ((ok = thread.load(db, source, spot)) != "" ||
	    (ok = thread.draw_tree(db)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database thread tree load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	set<string> lists;
	for (Threading::Key j = 0; j < thread.size(); ++j)
	{
		Summary& sum = thread.getSummary(j);
		if (!sum.loaded() && (ok = sum.load(db, cfg)) != "")
			break;
		
		const set<string>& mboxs = sum.mboxs();
		set<string>::const_iterator i;
		for (i = mboxs.begin(); i != mboxs.end(); ++i)
			lists.insert(*i);
	}
	
	if (ok != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database thread sumary load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	Cache cache(cfg, "thread", param, req.ext);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../ui/thread.xsl\"?>\n"
		<< "<thread xml:lang=\"" << req.language << "\">\n"
		<< " " << cfg(req.language) << "\n"
		<< " <hash>" << subject_hash(source.subject().c_str()) << "</hash>\n";
	
	set<string>::const_iterator list;
	for (list = lists.begin(); list != lists.end(); ++list)
	{
		Config::Lists::const_iterator desc = cfg.lists.find(*list);
		if (desc == cfg.lists.end()) continue;
		cache.o << " " << desc->second(req.language) << "\n";
	}
	
	int head = -1;
	for (Threading::Key i = 0; i < thread.size(); ++i)
	{
		if (i == spot)
			cache.o << " <row selected=\"true\">\n";
		else	cache.o << " <row>\n";
		
		cache.o << "  <tree>";
		thread.draw_tree_row(cache.o, &head, i);
		cache.o << "</tree>\n  " << thread.getSummary(i) << "\n </row>\n";
	}
	
	cache.o << "</thread>\n";
	
	return 0;
}
