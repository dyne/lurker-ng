/*  $Id: list.cpp,v 1.6 2003-07-01 15:49:04 terpstra Exp $
 *  
 *  list.cpp - Parse the config file for helper scripts
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

#include <ConfigFile.h>

#include <iostream>
#include <vector>

#include <unistd.h>

using namespace std;

void help(const char* name)
{
	cerr << "Lurker-list (v" << VERSION << ") parses lists from the config file.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> [-i -g -t -a -l -d] [listid ...]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-i               Output only the list id\n";
	cerr << "\t-g               Output only the group\n";
	cerr << "\t-t               Output only the title\n";
	cerr << "\t-a               Output only the email address\n";
	cerr << "\t-l               Output only the url link\n";
	cerr << "\t-d               Output only the description\n";
	cerr << "\n";
	cerr << "Output various lurker settings from the config file for use in shell scripts.\n";
	cerr << "Do not use sed/grep/etc, instead use this as it respects include.\n";
	cerr << "\n";
}

int main(int argc, char** argv)
{
	int c;
	
	const char* config  = 0;
	int         fields  = 0;
	bool        ids     = false;
	bool        group   = false;
	bool        title   = false;
	bool        address = false;
	bool        link    = false;
	bool        desc    = false;
	
	while ((c = getopt(argc, (char*const*)argv, "c:igtald?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'i':
			++fields;
			ids = true;
			break;
		case 'g':
			++fields;
			group = true;
			break;
		case 't':
			++fields;
			title = true;
			break;
		case 'a':
			++fields;
			address = true;
			break;
		case 'l':
			++fields;
			link = true;
			break;
		case 'd':
			++fields;
			desc = true;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	if (!config)
	{
		help(argv[0]);
		return 1;
	}
	
	if (fields >= 2)
	{
		cerr << "Please either retrieval all fields or exactly one.\n";
		return 1;
	}
	
	Config cfg;
	if (cfg.load(config) != 0)
	{
		cerr << cfg.getError() << flush;
		return 1;
	}
	
	vector<List> output;
	for (; optind < argc; ++optind)
	{
		Config::Lists::const_iterator l = cfg.lists.find(argv[optind]);
		if (l == cfg.lists.end())
		{
			cerr << "'" << argv[optind] << "' is not a mailing list id.\n";
			return 1;
		}
		output.push_back(l->second);
	}
	
	if (output.empty())
	{
		Config::Lists::const_iterator l;
		for (l = cfg.lists.begin(); l != cfg.lists.end(); ++l)
			output.push_back(l->second);
	}
	
	vector<List>::const_iterator o;
	for (o = output.begin(); o != output.end(); ++o)
	{
		if (!fields || ids)     cout << o->mbox        << "\n";
		if (!fields || group)   cout << o->group       << "\n";
		if (!fields || title)   cout << o->title       << "\n";
		if (!fields || address) cout << o->address     << "\n";
		if (!fields || link)    cout << o->link        << "\n";
		if (!fields || desc)    cout << o->description << "\n";
	}
	
	return 0;
}
