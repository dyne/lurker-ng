/*  $Id: params.cpp,v 1.5 2003-06-25 11:58:40 terpstra Exp $
 *  
 *  params.cpp - Parse the config file for helper scripts
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

#include <Config.h>

#include <iostream>
#include <vector>

#include <unistd.h>

using namespace std;

void help(const char* name)
{
	cerr << "Lurker-params (v" << VERSION << ") parses params from the config file.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> [-d -a -n -e -x]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-d               Output only the dbdir parameter\n";
	cerr << "\t-a               Output only the archive parameter\n";
	cerr << "\t-n               Output only the administrator name\n";
	cerr << "\t-e               Output only the administrator email address\n";
	cerr << "\t-x               Output only the xslt processing command\n";
	cerr << "\t-w               Output only the web_cache state\n";
	cerr << "\n";
	cerr << "Output various lurker settings from the config file for use in shell scripts.\n";
	cerr << "Do not use sed/grep/etc, instead use this as it respects include.\n";
	cerr << "\n";
}

int main(int argc, char** argv)
{
	int c;
	
	const char* config        = 0;
	int         fields        = 0;
	bool        dbdir         = false;
	bool        archive       = false;
	bool        admin_name    = false;
	bool        admin_address = false;
	bool        xslt          = false;
	bool        web_cache     = false;
	
	while ((c = getopt(argc, (char*const*)argv, "c:danexw?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'd':
			++fields;
			dbdir = true;
			break;
		case 'a':
			++fields;
			archive = true;
			break;
		case 'n':
			++fields;
			admin_name = true;
			break;
		case 'e':
			++fields;
			admin_address = true;
			break;
		case 'x':
			++fields;
			xslt = true;
			break;
		case 'w':
			++fields;
			web_cache = true;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	if (!config || optind < argc)
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
	
	if (!fields || dbdir)         cout << cfg.dbdir         << "\n";
	if (!fields || archive)       cout << cfg.archive       << "\n";
	if (!fields || admin_name)    cout << cfg.admin_name    << "\n";
	if (!fields || admin_address) cout << cfg.admin_address << "\n";
	if (!fields || xslt)          cout << cfg.xslt          << "\n";
	if (!fields || web_cache)     cout << (cfg.web_cache?"on":"off") << "\n";
	
	return 0;
}
