/*  $Id: main.cpp,v 1.5 2003-04-25 20:08:50 terpstra Exp $
 *  
 *  main.cpp - Read the fed data into our database
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

#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstdlib>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>

#include "Index.h"

using namespace std;

auto_ptr<ESort::Writer> db;

string         append;
List*          list = 0;
int            mbox = -1;
off_t          length = 0;

void help(const char* name)
{
	cerr << "Lurker-index (v" << VERSION << ") imports messages to the archive.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> -l <list> ( -m | -b <count> )\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-l <list>        Import messages to the named list\n";
	cerr << "\t-b <count>       Import a batch of messages; flush every <count>.\n";
	cerr << "\t-m               Import a single message\n";
	cerr << "\t-v               Verbose operation\n";
	cerr << "\n";
	cerr << "Index messages from standard input and store them in the lurker database.\n";
	cerr << "Either import a single message, or import a batch of messages in mbox format.\n";
	cerr << "\n";
}

time_t convert_date_mbox(const char* d)
{	/* Format: www mmm dd hh:mm:ss yyyy */
	struct tm t;
	char mon[4];
	
	if (sscanf(d, "%*3s %3s %2d %02d:%02d:%02d %d",
		&mon[0], &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year)
		!= 6)
	{
		return 0;
	}
	
	t.tm_year -= 1900;
	
	if      (!strcmp(&mon[0], "Jan")) t.tm_mon = 0;
	else if (!strcmp(&mon[0], "Feb")) t.tm_mon = 1;
	else if (!strcmp(&mon[0], "Mar")) t.tm_mon = 2;
	else if (!strcmp(&mon[0], "Apr")) t.tm_mon = 3;
	else if (!strcmp(&mon[0], "May")) t.tm_mon = 4;
	else if (!strcmp(&mon[0], "Jun")) t.tm_mon = 5;
	else if (!strcmp(&mon[0], "Jul")) t.tm_mon = 6;
	else if (!strcmp(&mon[0], "Aug")) t.tm_mon = 7;
	else if (!strcmp(&mon[0], "Sep")) t.tm_mon = 8;
	else if (!strcmp(&mon[0], "Oct")) t.tm_mon = 9;
	else if (!strcmp(&mon[0], "Nov")) t.tm_mon = 10;
	else if (!strcmp(&mon[0], "Dec")) t.tm_mon = 11;
	else return 0;
	
	/** Mailbox arrival times are always local time... Sadly.
	 */
	return mktime(&t);
}

int commit()
{
	unsigned long done = 0;
	while (done < append.length())
	{
		long did = write(mbox, append.c_str()+done, append.length()-done);
		if (did <= 0)
		{
			perror("write");
			ftruncate(mbox, length);
			cerr << "Mbox write failed; aborting..." << endl;
			return -1;
		}
		done += did;
	}
	
	if (db->commit() != 0)
	{
		perror("commit");
		ftruncate(mbox, length);
		cerr << "Commit failed; aborting..." << endl;
		return -1;
	}
	
	length += append.length();
	append = "";
	
	return 0;
}

int index(const DwString& msg, long batch)
{
//	cout << msg.c_str() << endl;
	static int count = 0;
	if (++count == batch)
	{
		count = 0;
		if (commit() != 0) return -1;
	}
	
	off_t start = length + append.length();
	
	time_t arrival;
	const char* d = msg.c_str();
	if (d[0] == 'F' && d[1] == 'r' && d[2] == 'o' && d[3] == 'm' && d[4] == ' ')
	{
		while (*d && *d != ' ' && *d != '\t') ++d;
		while (*d && (*d == ' ' || *d == '\t')) ++d;
		while (*d && *d != ' ' && *d != '\t') ++d;
		while (*d && (*d == ' ' || *d == '\t')) ++d;
		
		arrival = convert_date_mbox(d);
	}
	else
	{
		arrival = time(0);
		
		/** Make a local-timezone timestamp for the message envelope.
		 */
		char buf[100];
		strftime(buf, 100,
		  "From lurker-index@localhost %a %b %2d %02H:%02M:%02S %Y\n",
		  localtime(&arrival));
		  
		append.append(buf);
	}
	
	append.append(msg.c_str(), msg.length());
	Index i(msg, db.get(), *list, start, msg.length());
	
	if (i.index(arrival) != 0)
	{
		cerr << "Import failed; aborting..." << endl;
		return -1;
	}
	
	return 0;
}

void status(off_t did, long messages, time_t start)
{
	time_t amt = time(0) - start;
	if (amt == 0) amt = 1;
	cout << "Imported " << messages << " messages (" << (did/(1024*1024)) << "Mb)"
	     << " at " << (messages / amt) << " messages (" << (did/(1024*amt)) << "Kb) / second" << endl;
}

int main(int argc, char** argv)
{
	int c;
	
	const char* config  = 0;
	const char* listn   = 0;
	long        batch   = 0;
	int         verbose = 0;
	
	srandom(time(0));
	
	while ((c = getopt(argc, (char*const*)argv, "c:l:b:mv?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'l':
			listn = optarg;
			break;
		case 'b':
			batch = atol(optarg);
			break;
		case 'm':
			batch = -1;
			break;
		case 'v':
			verbose = 1;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	if (!config || !listn || batch == 0 || optind < argc)
	{
		help(argv[0]);
		return 1;
	}
	
	Config cfg;
	if (cfg.load(config) != 0)
	{
		cerr << cfg.error.str() << flush;
		return 1;
	}
	
	Config::Lists::iterator i = cfg.lists.find(listn);
	if (i == cfg.lists.end())
	{
		cerr << "No such list '" << listn << "' defined; cannot import to it." << endl;
		return 1;
	}
	list = &i->second;
	
	db = ESort::Writer::open(cfg.dbdir + "/db");
	if (!db.get())
	{
		perror("opening database");
		return 1;
	}
	
	string mboxf = cfg.dbdir + "/" + listn;
	mbox = open(mboxf.c_str(), O_RDWR|O_CREAT, 0666);
	if (mbox == -1)
	{
		perror(mboxf.c_str());
		return 1;
	}
	length = lseek(mbox, 0, SEEK_END);
	
	/** Begin processing standard input.
	 */
	string buf;
	char sect[8192];
	
	off_t did = 0;
	time_t start = time(0);
	long messages = 0;
	
	while (1)
	{
		int got = read(0, sect, sizeof(sect));
		if (got == -1)
		{
			perror("read");
			cerr << "Standard input failed!" << endl;
			return 1;
		}
		did += got;
		
		string::size_type was = buf.length();
		buf.append(sect, got);
		
		if (was >= 6)
			was -= 6;
		else	was = 0;
		
		string::size_type eos;
		while (batch != -1 && (eos = buf.find("\nFrom ", was)) != string::npos)
		{
			DwString msg(buf.c_str(), eos+1);
			buf = buf.substr(eos+1, string::npos);
			if (index(msg, batch) != 0) return 1;
			++messages;
			was = 0; 
			
			if (verbose && messages % 1000 == 0)
				status(did, messages, start);
		}
		
		if (got == 0)
		{
			DwString msg(buf.c_str(), buf.length());
			buf = "";
			if (index(msg, batch) != 0) return 1;
			break;
		}
	}
	
	if (commit() != 0) return 1;
	
	if (verbose) status(did, messages, start);
	
	return 0;
}
