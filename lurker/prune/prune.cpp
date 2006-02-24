/*  $Id: prune.cpp,v 1.17 2006-02-24 14:12:04 terpstra Exp $
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
#include <esort.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>

#include <unistd.h>
#include <fcntl.h>
#include <utime.h>

#include <iostream>

#include <cstring>
#include <cerrno>
#include <ctime>

#include "PTable.h"

using namespace std;

bool        verbose = false;
bool        purge = false;
time_t      modifyTime = 60*60*24*7;
time_t      accessTime = 60*60*24*1;

void help(const char* name)
{
	cerr << "Lurker-prune (v" << VERSION << ") prunes the web-server cache.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " [-c <config-file>] [-f <frontend>] [-m <d> -a <d> -p -v]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-f <frontend>    Only clear cache from the named frontend        [all]\n";
	cerr << "\t-m <days>        Keep cached files for at most this many days      [7]\n";
	cerr << "\t-a <days>        Kill cached files not accessed for this many days [1]\n";
	cerr << "\t-p               Purge everything even if it appears to not be expired\n";
	cerr << "\t-v               Verbose operation\n";
	cerr << "\n";
	cerr << "Prune obsolete or stale html/xml from the web-server accessible cache.\n";
	cerr << "This command should be run at regular intervals from cron for each site.\n";
	cerr << "\n";
}

int execute(const Config& cfg, const string& docroot)
{
	if (verbose) cout << "Cleaning document root " << docroot << endl;
	
	string docfile = docroot + "/lurker.docroot";
	int fd = open(docfile.c_str(), O_RDWR | O_CREAT, 0666);
	if (fd == -1)
	{
		cerr << "open()ing " << docfile << ": " << strerror(errno) << endl;
		return 1;
	}
	
	struct stat dbuf;
	if (fstat(fd, &dbuf) < 0)
	{
		cerr << "stat()ing " << docfile << ": " << strerror(errno) << endl;
		return 1;
	}
	
	enum LockState { GOT, FAIL, USED } state = GOT;
	
#ifdef	LOCK_EX
	if (flock(fd, LOCK_EX|LOCK_NB) != 0)
	{
		if (errno == EWOULDBLOCK)
			state = USED;
		else	state = FAIL;
	}
#else
#ifdef F_SETLK
	struct flock lock;
	memset(&lock, 0, sizeof(lock));
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	if (fcntl(fd, F_SETLK, &lock) != 0)
	{
		if (errno == EACCES || errno == EAGAIN)
			state = USED;
		else	state = FAIL;
	}
#endif
#endif
	
	if (state == USED)
	{
		if (verbose) cout << "Already pruning docroot " << docroot << endl;
		return 0;
	}
	
	if (state == FAIL)
	{
		cerr << "Locking " << docfile << " failed: " << strerror(errno) << endl;
		return 1;
	}
	
	std::auto_ptr<ESort::Reader> db(ESort::Reader::opendb(cfg.dbdir + "/db"));
	if (!db.get())
	{
		cerr << "Opening database: " << strerror(errno) << endl;
		return 1;
	}
	
	time_t beginfix = time(0);
	
	if (chdir(docroot.c_str()) != 0)
	{
		cerr << "chdir: " << docroot << ": " << strerror(errno) << endl;
		return 1;
	}
	
	PTable ptable(cfg, db.get(), dbuf.st_mtime, purge, verbose, modifyTime, accessTime);
	string ok;
	
	if ((ok = ptable.load()) != "")
	{
		cerr << "load: " << ok << endl;
		return 1;
	}
	
	if ((ok = ptable.calc()) != "")
	{
		cerr << "calc: " << ok << endl;
		return 1;
	}
	
	if ((ok = ptable.kill()) != "")
	{
		cerr << "kill: " << ok << endl;
		return 1;
	}
	
	// set the mtime stamp to beginfix
	struct utimbuf touch;
	touch.actime = touch.modtime = beginfix;
	if (utime("lurker.docroot", &touch) < 0)
	{
		cerr << "touching " << docfile << ": " << strerror(errno) << endl;
		return 1;
	}
	
	return 0;
}
	
int main(int argc, char** argv)
{
	int c;
	
	const char* config  = DEFAULT_CONFIG_FILE;
	const char* docroot = 0;
	
	srandom(time(0));
	
	while ((c = getopt(argc, (char*const*)argv, "c:f:m:a:vp?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'f':
			docroot = optarg;
			break;
		case 'm':
			modifyTime = atol(optarg)*60*60*24;
			if (!modifyTime)
			{
				cerr << "Modification time is not a number!\n";
				return 1;
			}
			break;
		case 'a':
			accessTime = atol(optarg)*60*60*24;
			if (!accessTime)
			{
				cerr << "Access time is not a number!\n";
				return 1;
			}
			break;
		case 'p':
			purge = true;
			break;
		case 'v':
			verbose = true;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	while (optind < argc)
	{
		if (!argv[optind][0])
		{	// ignore empty arguments
			optind++;
			continue;
		}
		
		cerr << "Unexpected argument: '" << argv[optind] << "'\n";
		return 1;
	}
	
	Config cfg;
	if (cfg.load(config) != 0)
	{
		cerr << cfg.getError() << flush;
		return 1;
	}
	
	if (docroot) execute(cfg, docroot);
	else
	{
		Config::Frontends::const_iterator i,
			s = cfg.frontends.begin(),
			e = cfg.frontends.end();
		for (i = s; i != e; ++i)
		{
			execute(cfg, i->first);
		}
	}
	
	return 0;
}
