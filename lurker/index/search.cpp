/*  $Id: search.cpp,v 1.11 2006-02-25 00:32:25 terpstra Exp $
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

#define _FILE_OFFSET_BITS 64

#include <Search.h>
#include <Keys.h>

#include <iostream>
#include <assert.h>
#include <stdio.h>

using namespace std;

void help(const char* name)
{
	cerr << "Lurker-search (v" << VERSION << ") searches for messages in a lurker database.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " [-c <config-file>] [-k <keyword>] [ -d -f -v -q ] <terms>*\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-k <keyword>     Add the specified keyword tag to hits\n";
	cerr << "\t-d               Delete matching messages\n";
	cerr << "\t-f               Don't prompt before deleting\n";
	cerr << "\t-v               Output message summaries\n";
	cerr << "\t-i               Take lurker message ids instead of keywords\n";
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
	
	const char* config = DEFAULT_CONFIG_FILE;
	bool erase = false;
	bool force = false;
	bool verbose = false;
	bool quiet = false;
	bool ids = false;
	string keyword;
	
	while ((c = getopt(argc, (char*const*)argv, "c:k:dvfiq?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'k':
			keyword = optarg;
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
		case 'i':
			ids = true;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	if (optind >= argc)
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
	
	ESort::Writer* db;
	auto_ptr<ESort::Reader> dbr;
	
	if (erase || keyword != "")
	{
		if (verbose) cerr << "opening " << cfg.dbdir << "/db read-write" << endl;
		// Work around g++ 2.95 bug
		auto_ptr<ESort::Writer> w
			(ESort::Writer::opendb(cfg.dbdir + "/db"));
		db = w.get();
		dbr = w;
	}
	else
	{
		if (verbose) cerr << "opening " << cfg.dbdir << "/db read-only" << endl;
		auto_ptr<ESort::Reader> r
			(ESort::Reader::opendb(cfg.dbdir + "/db"));
		dbr = r;
	}

	if (!dbr.get())
	{
		perror("opening database");
		return 1;
	}
	
	vector<Summary> result;
	
	if (ids)
	{
		for (; optind < argc; ++optind) 
		{
			if (!argv[optind][0]) continue; // skip empty
			MessageId id(argv[optind]);
			if (id.serialize() != argv[optind])
			{
				cerr << "'" << argv[optind] << "' is not a message-id\n";
				return 1;
			}
			result.push_back(Summary(id));
		}
	}
	else
	{
		Search s(cfg, dbr.get(), ESort::Forward);
		
		for (; optind < argc; ++optind) 
		{
			if (!argv[optind][0]) continue; // skip empty
			s.keyword(argv[optind]);
		}
		
		vector<Summary>::size_type sz = 0;
		while (s.pull(1, result) && result.size() == sz+1)
		{
			sz = result.size();
		}
	}
	
	vector<Summary>::iterator i, e = result.end();
	for (i = result.begin(); i != e; ++i)
	{
		if (!quiet)
			cout << "id: " << i->id().serialize() << "\n";
		if (verbose)
		{
			string ok;
			if ((ok = i->load(dbr.get(), cfg)) != "")
			{
				cerr << "Failed to load: " << ok << "\n";
				return 1;
			}
			cout << "sb: " << i->subject() << "\n";
			cout << "au: \"" << i->author_name() << "\" <"
			     << i->author_email() << ">\n";
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
	
	if (keyword != "")
	{
		if (!quiet) cerr << "Tagging messages with keyword" << endl;
		for (i = result.begin(); i != e; ++i)
		{
			if (db->insert(
				LU_KEYWORD +
				keyword +
				'\0' +
				i->id().raw()) != 0)
			{
				perror("insert");
				cerr << "Tagging with keyword failed; operation aborted.\n";
				return 1; 
			}
		}
	}
	
	if (erase)
	{
		if (!quiet) cerr << "Marking messages as deleted" << endl;
		
		// The idea is that lurker-prune already kills cache which
		// refers to a newly imported message (regardless of time).
		// Therefore, report the deleted message as new.
		MessageId importStamp(time(0));
		
		for (i = result.begin(); i != e; ++i)
		{
			if (db->insert(
				LU_KEYWORD +
				string(LU_KEYWORD_DELETED) +
				'\0' +
				i->id().raw()) != 0)
			{
				perror("insert");
				cerr << "Delete keyword failed; operation aborted.\n";
				return 1; 
			}
			
			if (db->insert(
				LU_SUMMARY +
				i->id().raw() + 
				LU_MESSAGE_DELETED) != 0)
			{
				perror("insert");
				cerr << "Delete summary failed; operation aborted.\n";
				return 1;
			}
			if (db->insert(
				LU_CACHE + 
				importStamp.raw().substr(0, 4) + 
				i->id().raw()) != 0)
			{
				perror("insert");
				cerr << "Delete cache eviction failed; operation aborted.\n";
				return 1;
			}
		}
	}
	
	if (erase || keyword != "")
	{
		if (!quiet) cerr << "Committing changes to disk" << endl;
		if (db->commit() != 0)
		{
			perror("commit");
			cerr << "Commit failed; operation aborted.\n";
			return 1;
		}
		
		if (!quiet)
		{
			cerr << "Cache will be automatically corrected when the cronjob next runs.\n";
		}
	}
	
	return 0;
}
