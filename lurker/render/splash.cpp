/*  $Id: splash.cpp,v 1.7 2003-06-23 14:38:43 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _FILE_OFFSET_BITS 64

#include "commands.h"
#include "Cache.h"

int handle_splash(const Config& cfg, ESort::Reader* db, const string& param)
{
	Cache cache(cfg, "splash", param);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/splash.xsl\"?>\n"
		<< "<splash>\n"
		<< " " << cfg << "\n";
	
	Config::Groups::const_iterator group;
	for (group = cfg.groups.begin(); group != cfg.groups.end(); ++group)
	{
		cache.o << " <group>\n"
			<< "  <id>" << group->first << "</id>\n";
		
		if (group->second.heading != "")
			cache.o << "  <heading>" << group->second.heading << "</heading>\n";
		
		Config::Members::const_iterator member;
		for (member = group->second.members.begin(); member != group->second.members.end(); ++member)
			cache.o << "  " << *member->second << "\n";
		
		cache.o << " </group>\n";
	}
	
	cache.o << "</splash>\n";
	
	return 0;
}
