/*  $Id: prune.cpp,v 1.1 2003-05-10 19:28:09 terpstra Exp $
 *  
 *  prune.cpp - Prune obsolete / stale cache files
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

#include <Config.h>
#include <esort.h>

#include <unistd.h>

void help(const char* name)
{
	cerr << "Lurker-prune (v" << VERSION << ") prunes the web-server cache.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> -d <docroot> [-v]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-d <docroot>     The base directory of the lurker cache\n";
	cerr << "\t-v               Verbose operation\n";
	cerr << "\n";
	cerr << "Prune obsolete or stale html/xml from the web-server accessible cache.\n";
	cerr << "This command should be run at regular intervals from cron for each site.\n";
	cerr << "\n";
}

int main(int argc, char** argv)
{
	int c;
	
	const char* config  = 0;
	const char* docroot = 0;
	int         verbose = 0;
	
	srandom(time(0));
	
	while ((c = getopt(argc, (char*const*)argv, "c:d:v?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'd':
			docroot = optarg;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	if (!config || !docroot || optind < argc)
	{
		help(argv[0]);
		return 1;
	}
	
	Config cfg;
	if (cfg.load(config) != 0)
	{
		cerr << cfg.getError() << flush;
		return 1;
	}
	
	return 0;
}
