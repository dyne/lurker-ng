/*  $Id: ConfigFile.h,v 1.3 2004-08-19 14:52:29 terpstra Exp $
 *  
 *  ConfigFile.h - Knows how to load the config file
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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <iostream>

#if __GNUC__ == 2
#include <strstream>
#else
#include <sstream>
#endif

// get VERSION
#include "../config.h"

using std::string;
using std::map;
using std::ostream;

struct List
{
	string mbox;
	string title;
	string address;
	string description;
	string link;
	string group;
};

class Config
{
 private:
 	List* list;
 	string group;
#if __GNUC__ == 2
	strstream error;
#else
	std::stringstream error;
#endif
 	
 public:
 	typedef map<string, List>    Lists;
 	typedef map<string, List*>   Members;
 	
 	struct GroupData
 	{
 		string  heading;
 		Members members;
 	};
 	
 	typedef map<string, GroupData> Groups;
 	
 	Lists  lists;
 	Groups groups;
 	 	
 	string	dbdir;
 	string	archive;
 	string	admin_name;
 	string	admin_address;
 	string	xslt;
 	string	pgpv_mime;
 	string	pgpv_inline;
 	bool	web_cache;
 	bool	hide_email;
 	bool	raw_email;
 	time_t	modified; // the timestamp of modification for the config file
 	
 	// parameters specific for rendering
 	string	docUrl;
 	string	cgiUrl;
 	
 	// get the error string
 	string getError()
 	{
#if __GNUC__ == 2
		string out(error.str(), error.rdbuf()->pcount());
		return out;
#else
		return error.str();
#endif
 	}
 	
 	Config();
 	
 	// Open the config from this file.
 	int load(const string& file);
 	int process_command(const string& key, const string& val, const string& dir);
};

ostream& operator << (ostream& o, const List& l);
ostream& operator << (ostream& o, const Config& c);

#endif
