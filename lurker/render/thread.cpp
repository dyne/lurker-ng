/*  $Id: thread.cpp,v 1.16 2006-03-01 14:55:45 terpstra Exp $
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
		error(_("Bad request"), param,
		      _("The given parameter was not of the correct format. "
		        "A thread request must be formatted like: "
		         "thread/YYYYMMDD.HHMMSS.hashcode.xml"));
	
	MessageId id(req.options.c_str());
	string ok;
	
	Summary source(id);
	// Identical error for a not allowed message as non-existing (security)
	if ((ok = source.load(db, cfg)) != "" || !source.allowed())
	{
		if (ok == "") ok = "not in a mailbox"; // fake
		error(_("Database thread source pull failure"), ok,
		      _("The specified message does not exist."));
	}
	
	if (source.deleted())
		error(_("Database thread source pull failure"), "not found",
		      _("The specified message has been deleted."));
	
	Threading::Key spot;
	Threading thread;
	if ((ok = thread.load(db, source, spot)) != "" ||
	    (ok = thread.draw_tree(db)) != "")
		error(_("Database thread tree load failure"), ok,
		      _("Something internal to the database failed. "
		        "Please contact the lurker user mailing list for "
		        "further assistence."));
	
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
		error(_("Database thread sumary load failure"), ok,
		      _("Something internal to the database failed. "
		        "Please contact the lurker user mailing list for "
		        "further assistence."));
	
	Cache cache(cfg, "thread", param, req.ext);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../ui/thread.xsl\"?>\n"
		<< "<thread xml:lang=\"" << req.language << "\">\n"
		<< " <mode>" << req.ext << "</mode>\n"
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
