/*  $Id: thread.cpp,v 1.2 2003-04-21 18:26:21 terpstra Exp $
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

#include <Keys.h>

#include "commands.h"
#include "Threading.h"
#include "Cache.h"

int handle_thread(const Config& cfg, ESort::Reader* db, const string& param)
{
	MessageId id(param.c_str());
	
	if (id.timestamp() == 0)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Bad request"), param,
			_("The given parameter was not of the correct format. "
			  "A thread request must be formatted like: "
			  "thread/YYYYMMDD.HHMMSS.hashcode.xml"));
		return 1;
	}
	
	string ok;
	
	Summary source(id);
	if ((ok = source.load(db)) != "")
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
	
	for (Threading::Key j = 0; j < thread.size(); ++j)
	{
		Summary& sum = thread.getSummary(j);
		if (!sum.loaded() && (ok = sum.load(db)) != "")
			break;
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
	
	Cache cache(cfg, "thread", param);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/render.xsl\"?>\n"
		<< "<thread>\n"
		<< " " << cfg << "\n"
		<< " <hash>" << subject_hash(source.subject().c_str()) << "</hash>\n";
	
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