/*  $Id: splash.cpp,v 1.13 2006-02-21 12:49:36 terpstra Exp $
 *  
 *  splash.cpp - Handle a splash/ command
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

#include "commands.h"
#include "Cache.h"

int handle_splash(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	cfg.options = req.options;
	
	if (req.options != "index")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Bad request"), param,
			_("The given parameter was not of the correct format. "
			  "A splash request must be formatted like: "
			  "splash/index.lc.xml"));
		return 1;
	}
	
	Cache cache(cfg, "splash", param, req.ext);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../ui/splash.xsl\"?>\n"
		<< "<splash xml:lang=\"" << req.language << "\">\n"
		<< " " << cfg(req.language) << "\n";
	
	Config::Groups::const_iterator group;
	for (group = cfg.groups.begin(); group != cfg.groups.end(); ++group)
	{
		cache.o << " <group>\n"
			<< "  <id>" << group->first << "</id>\n";
		
		if (group->second.heading.is_set())
			cache.o << "  <heading>" << group->second.heading(req.language) << "</heading>\n";
		
		Config::Members::const_iterator member;
		for (member = group->second.members.begin(); member != group->second.members.end(); ++member)
		{
			Config::Lists::const_iterator i = cfg.lists.find(*member);
			if (i == cfg.lists.end()) continue;
			cache.o << "  " << i->second(req.language) << "\n";
		}
		
		cache.o << " </group>\n";
	}
	
	cache.o << "</splash>\n";
	
	return 0;
}
