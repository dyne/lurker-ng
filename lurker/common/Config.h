/*  $Id: Config.h,v 1.3 2003-04-22 13:22:05 terpstra Exp $
 *  
 *  Config.h - Knows how to load the config file
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
#include <sstream>
#include <map>
#include <iostream>

#include "Version.h"

using std::string;
using std::map;
using std::stringstream;
using std::ostream;

struct List
{
	string mbox;
	string title;
	string address;
	string description;
	string link;
};

class Config
{
 public:
 	typedef map<string, List> Lists;
 	
 	Lists lists;
 	List* list;
 	 	
 	string	dbdir;
 	string	host;
 	string	admin_name;
 	string	admin_address;
 	string	xslt;
 	
 	stringstream error;
 	
 	Config() : lists(), list(0) { }
 	
 	// Open the config from this file.
 	int load(const char* file);
 	int process_command(const string& key, const string& val);
};

ostream& operator << (ostream& o, const List& l);
ostream& operator << (ostream& o, const Config& c);

#endif
