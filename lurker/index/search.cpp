/*  $Id: search.cpp,v 1.1 2004-08-24 16:14:27 terpstra Exp $
 *  
 *  search.cpp - Search for messages in lurker database (optionally delete)
 *  
 *  Copyright (C) 2004 - Wesley W. Terpstra
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

#include <Search.h>
#include <Keys.h>

#include <iostream>
#include <assert.h>

using namespace std;

void help(const char* name)
{
	cerr << "Lurker-search (v" << VERSION << ") searches for messages in a lurker database.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> [ -d -f -v -q ] <terms>*\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-d               Delete matching messages\n";
	cerr << "\t-f               Don't prompt before deleting\n";
	cerr << "\t-v               Output message summaries\n";
	cerr << "\t-q               Don't output message ids or status\n";
	cerr << "\n";
	cerr << "Execute a keyword search to find messages.\n";
	cerr << "The following search terms are supported (all must match):\n";
	cerr << " -xxx   - select messages which do NOT match this term\n";
	cerr << " id:xxx - select a message by the Message-ID field\n";
	cerr << " rt:xxx - select messages which reply to the chosen Message-ID\n";
	cerr << " th:xxx - select messages included in this thread\n";
	cerr << " ml:xxx - select messages in this mailing list\n";
	cerr << " gr:xxx - select messages in this mailing list group\n";
	cerr << " au:xxx - select messages with this term in the author fields\n";
	cerr << " sb:xxx - select messages with this word in the subject\n";
	cerr << " lang:xx - select messages in this language\n";
	cerr << "\n";
}

int main(int argc, char** argv)
{
	int c;
	
	const char* config = 0;
	bool erase = false;
	bool force = false;
	bool verbose = false;
	bool quiet = false;
	
	while ((c = getopt(argc, (char*const*)argv, "c:dvfq?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'd':
			erase = true;
			break;
		case 'f':
			force = true;
			break;
		case 'v':
			verbose = true;
			break;
		case 'q':
			quiet = true;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	if (!config || optind >= argc)
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
	
	auto_ptr<ESort::Writer> db(ESort::Writer::opendb(cfg.dbdir + "/db"));
	Search s(cfg, db.get(), ESort::Forward);
	
	while (optind < argc) s.keyword(argv[optind++]);
	
	vector<Summary> result;
	vector<Summary>::size_type sz = 0;
	while (s.pull(1, result) && result.size() == sz+1)
	{
		sz = result.size();
		
		if (!quiet)
			cout << "id: " << result.back().id().serialize() << "\n";
		if (verbose)
		{
			string ok;
			if ((ok = result.back().load(db.get(), cfg)) != "")
			{
				cerr << "Failed to load: " << ok << "\n";
				return 1;
			}
			cout << "sb: " << result.back().subject() << "\n";
			cout << "au: \"" << result.back().author_name() << "\" <"
			     << result.back().author_email() << ">\n";
		}
	}
	
	if (result.empty()) return 0;
	
	if (erase && !force)
	{
		cout << flush;
		cerr << "Are you certain you want to delete these messages? (yes/no) [no] " << flush;
		string ok;
		std::getline(cin, ok);
		if (ok != "yes")
		{
			cerr << "aborted!\n";
			return 1;
		}
	}
	
	if (erase)
	{
		if (!quiet) cerr << "Marking messages as deleted" << endl;
		for (vector<Summary>::iterator i = result.begin(); 
		     i != result.end();
		     ++i)
		{
			if (db->insert(
				LU_KEYWORD +
				string(LU_KEYWORD_DELETED) +
				'\0' +
				i->id().raw()) != 0)
			{
				perror("insert");
				cerr << "Delete failed; operation aborted.\n";
				return 1; 
			}
		}
		if (!quiet) cerr << "Committing changes to disk" << endl;
		if (db->commit() != 0)
		{
			perror("commit");
			cerr << "Commit failed; operation aborted.\n";
			return 1;
		}
		
		if (!quiet)
		{
			cerr << "\n";
			cerr << "Message deleted -- cache is now invalid.\n";
			cerr << "Re-run lurker-prune with the '-p' option.\n";
		}
	}
	
	return 0;
}
