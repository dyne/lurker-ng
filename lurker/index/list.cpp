/*  $Id$
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
	cerr << "Usage: " << name << " [-c <config-file>] [-f <locale>]\n";
	cerr << "                         [-i -g -t -a -l -d -s -o] [listid ...]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-f <locale>      Output the fields for this locale\n";
	cerr << "\t-i               Output only the list id\n";
	cerr << "\t-g               Output only the group\n";
	cerr << "\t-t               Output only the title\n";
	cerr << "\t-a               Output only the email address\n";
	cerr << "\t-l               Output only the url link\n";
	cerr << "\t-d               Output only the description\n";
	cerr << "\t-s               Output only the language\n";
	cerr << "\t-o               Output only the offline status\n";
	cerr << "\n";
	cerr << "Output various lurker settings from the config file for use in shell scripts.\n";
	cerr << "Do not use sed/grep/etc, instead use this as it respects include.\n";
	cerr << "\n";
}

int main(int argc, char** argv)
{
	int c;
	
	const char* config   = DEFAULT_CONFIG_FILE;
	int         fields   = 0;
	bool        ids      = false;
	bool        group    = false;
	bool        title    = false;
	bool        address  = false;
	bool        link     = false;
	bool        desc     = false;
	bool        language = false;
	bool        offline  = false;
	string lc;
	
	while ((c = getopt(argc, (char*const*)argv, "c:f:igtaldso?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'f':
			lc = optarg;
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
		case 's':
			++fields;
			language = true;
			break;
		case 'o':
			++fields;
			offline = true;
			break;
		default:
			help(argv[0]);
			return 1;
		}
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
		if (!argv[optind][0]) continue; // ignore empty arguments
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
	
	if (lc != "" && !lstring::locale_normalize(lc))
	{
		cerr << "'" << lc << "' is not a valid locale\n";
		return 1;
	}
	
	vector<List>::const_iterator o;
	for (o = output.begin(); o != output.end(); ++o)
	{
		if (!fields || ids)     cout << o->mbox            << "\n";
		if (!fields || group)   cout << o->group           << "\n";
		if (!fields || title)   cout << o->title(lc)       << "\n";
		if (!fields || address) cout << o->address         << "\n";
		if (!fields || link)    cout << o->link(lc)        << "\n";
		if (!fields || desc)    cout << o->description(lc) << "\n";
		if (!fields || language)
		{
			set<string>::const_iterator i, e;
			for (i = o->languages.begin(), e = o->languages.end(); i != e; )
			{
				cout << *i;
				++i;
				if (i == e)	cout << "\n";
				else		cout << ",";
			}
		}
		if (!fields || offline) cout << (o->offline?"true\n":"false\n");
	}
	
	return 0;
}
