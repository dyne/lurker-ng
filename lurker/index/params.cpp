/*  $Id: params.cpp,v 1.11 2004-08-27 17:53:44 terpstra Exp $
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

#include <ConfigFile.h>

#include <iostream>
#include <vector>

#include <unistd.h>

using namespace std;

void help(const char* name)
{
	cerr << "Lurker-params (v" << VERSION << ") parses params from the config file.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> -f <locale>\n";
	cerr << "                         [-d -a -n -e -x -m -i -w -h -r -g]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-f <locale>      Output the fields for this locale\n";
	cerr << "\t-d               Output only the dbdir parameter\n";
	cerr << "\t-a               Output only the archive parameter\n";
	cerr << "\t-n               Output only the administrator name\n";
	cerr << "\t-e               Output only the administrator email address\n";
	cerr << "\t-x               Output only the xslt processing command\n";
	cerr << "\t-m               Output only the mime pgp verifying command\n";
	cerr << "\t-i               Output only the inline pgp verifying command\n";
	cerr << "\t-w               Output only the web_cache state\n";
	cerr << "\t-h               Output only the hide_email state\n";
	cerr << "\t-r               Output only the raw_email state\n";
	cerr << "\t-g               Output only the regroupable state\n";
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
	bool        pgpv_mime     = false;
	bool        pgpv_inline   = false;
	bool        web_cache     = false;
	bool        hide_email    = false;
	bool        raw_email     = false;
	bool        regroupable   = false;
	string lc;
	
	while ((c = getopt(argc, (char*const*)argv, "c:f:danexmiwhrg?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'f':
			lc = optarg;
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
		case 'm':
			++fields;
			pgpv_mime = true;
			break;
		case 'i':
			++fields;
			pgpv_inline = true;
			break;
		case 'w':
			++fields;
			web_cache = true;
			break;
		case 'h':
			++fields;
			hide_email = true;
			break;
		case 'r':
			++fields;
			raw_email = true;
			break;
		case 'g':
			++fields;
			regroupable = true;
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
	if (!fields || archive)       cout << cfg.archive(lc)   << "\n";
	if (!fields || admin_name)    cout << cfg.admin_name(lc)<< "\n";
	if (!fields || admin_address) cout << cfg.admin_address << "\n";
	if (!fields || xslt)          cout << cfg.xslt          << "\n";
	if (!fields || pgpv_mime)     cout << cfg.pgpv_mime     << "\n";
	if (!fields || pgpv_inline)   cout << cfg.pgpv_inline   << "\n";
	if (!fields || web_cache)     cout << (cfg.web_cache?"on":"off") << "\n";
	if (!fields || hide_email)    cout << (cfg.hide_email?"on":"off") << "\n";
	if (!fields || raw_email)     cout << (cfg.raw_email?"on":"off") << "\n";
	if (!fields || regroupable)   cout << (cfg.regroupable?"on":"off") << "\n";
	
	return 0;
}
