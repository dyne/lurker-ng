/*  $Id: prune.cpp,v 1.12 2004-08-19 14:52:29 terpstra Exp $
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

#define _XOPEN_SOURCE 500
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

void help(const char* name)
{
	cerr << "Lurker-prune (v" << VERSION << ") prunes the web-server cache.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> -d <docroot> [-m <d> -a <d> -v]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-d <docroot>     The base directory of the lurker cache\n";
	cerr << "\t-m <days>        Keep cached files for at most this many days      [7]\n";
	cerr << "\t-a <days>        Kill cached files not accessed for this many days [1]\n";
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
	time_t      modifyTime = 60*60*24*7;
	time_t      accessTime = 60*60*24*1;
	
	srandom(time(0));
	
	while ((c = getopt(argc, (char*const*)argv, "c:d:m:a:v?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'd':
			docroot = optarg;
			break;
		case 'm':
			modifyTime = atol(optarg)*60*60*24;
			if (!modifyTime)
			{
				help(argv[0]);
				return 1;
			}
			break;
		case 'a':
			accessTime = atol(optarg)*60*60*24;
			if (!accessTime)
			{
				help(argv[0]);
				return 1;
			}
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
	
	string docfile = string(docroot) + "/lurker.docroot";
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
		if (verbose) cout << "Already pruning this docroot" << endl;
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
	
	if (chdir(docroot) != 0)
	{
		cerr << "chdir: " << docroot << ": " << strerror(errno) << endl;
		return 1;
	}
	
	PTable ptable(cfg, db.get(), dbuf.st_mtime, verbose, modifyTime, accessTime);
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
