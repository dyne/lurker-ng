/*  $Id: DbMan.cpp,v 1.6 2003-05-03 18:12:24 terpstra Exp $
 *  
 *  DbMan.cpp - Manage the commit'd segments and parameters
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
  
#include "io.h"
#include "DbMan.h"
#include "esort.h"
#include "View.h"

#include <cstdlib>

namespace ESort
{

static int shared_file_lock(int fd)
{
#ifdef LOCK_SH
	if (flock(fd, LOCK_SH) != 0) return -1;
#else
#ifdef F_SETLK
	 struct flock lock;
	 memset(&lock, 0, sizeof(lock));
	 lock.l_type = F_RDLCK;
	 lock.l_whence = SEEK_SET;
	 if (fcntl(fd, F_SETLKW, &lock) != 0) return -1;
#endif
#endif
	return 0;
}

static int exclusive_file_lock(int fd)
{
#ifdef LOCK_EX
	if (flock(fd, LOCK_EX) != 0) return -1;
#else
#ifdef F_SETLK
	 struct flock lock;
	 memset(&lock, 0, sizeof(lock));
	 lock.l_type = F_WRLCK;
	 lock.l_whence = SEEK_SET;
	 if (fcntl(fd, F_SETLKW, &lock) != 0) return -1;
#endif
#endif
	return 0;
}

static int unlock_file_lock(int fd)
{
#ifdef LOCK_UN
	if (flock(fd, LOCK_UN) != 0) return -1;
#else
#ifdef F_SETLK
	 struct flock lock;
	 memset(&lock, 0, sizeof(lock));
	 lock.l_type = F_UNLCK;
	 lock.l_whence = SEEK_SET;
	 if (fcntl(fd, F_SETLKW, &lock) != 0) return -1;
#endif
#endif
	return 0;
}

DbMan::DbMan()
 : dbname(), dbfile(0), cmode(0), dblock(-1)
{
}

DbMan::~DbMan()
{
	if (dbfile) fclose(dbfile);
	if (dblock != -1)
		unlock_database_rw();
}

int DbMan::qualify(string& db)
{
	char buf[10240];
	string::size_type slash = db.rfind('/');
	string name, path;
	
	if (slash != string::npos)
	{
		getcwd(buf, sizeof(buf));
		string old(buf);
		
		int cd = chdir(db.substr(0, slash).c_str());
		if (cd != 0) return -1;
		
		getcwd(buf, sizeof(buf));
		path = buf;
		path += "/";
		name = db.substr(slash+1, string::npos);
				
		chdir(old.c_str());
	}
	else
	{
		getcwd(buf, sizeof(buf));
		path = buf;
		path += "/";
		
		name = db;
	}
	
	db = path + name;
	return 0;
}

int DbMan::scanFile(Parameters& p)
{
	int version;
	long blockSize, keySize;
	char id[80];
	
	fseek(dbfile, 0, SEEK_SET);
	if (fscanf(dbfile, "%d %ld %ld", &version, &blockSize, &keySize) != 3)
		return -1;
	p = Parameters(version, blockSize, keySize);
	
	ids.clear();
	fgets(id, sizeof(id), dbfile); // eat eof
	while (fgets(id, sizeof(id), dbfile) != 0)
	{
		string i = id;
		if (i[i.length()-1] == '\n') i.resize(i.length()-1);
		ids.insert(i);
	}
	
	return 0;
}

int DbMan::open(const string& db, Parameters& p)
{
	assert (dbname == "");
	
	dbname = db;
	if (qualify(dbname) != 0) return -1;
	
	dbfile = fopen(dbname.c_str(), "r");
	if (dbfile == 0) return -1;
	
	if (scanFile(p) != 0) return -1;
	
	return 0;
}

int DbMan::open(const string& db, Parameters& p, int mode)
{
	assert (dbname == "");
	
	dbname = db;
	if (qualify(dbname) != 0) return -1;
	
	cmode = mode;
	int fd = ::open(dbname.c_str(), O_RDWR | O_CREAT, cmode);
	if (fd == -1) return -1;
	dbfile = fdopen(fd, "r+");
	assert (dbfile != 0);
	
	Parameters x(1,8,1);
	if (scanFile(x) != 0)
	{
		fseek(dbfile, 0, SEEK_SET);
		fprintf(dbfile, "%d %ld %ld\n", 
			p.version(), p.blockSize(), p.keySize());
		fflush(dbfile);
		return 0;
	}
	if (!x.isWider(p))
	{
		errno = EINVAL;
		return -1;
	}
	p = x;
	
	return 0;
}

int DbMan::lock_snapshot_ro()
{
	assert (dbfile != 0);
	return shared_file_lock(fileno(dbfile));
}

void DbMan::unlock_snapshot_ro()
{
	assert (dbfile != 0);
	unlock_file_lock(fileno(dbfile));
}

int DbMan::lock_snapshot_rw()
{
	assert (dbfile != 0);
	return exclusive_file_lock(fileno(dbfile));
}

void DbMan::unlock_snapshot_rw()
{
	assert (dbfile != 0);
	unlock_file_lock(fileno(dbfile));
}

int DbMan::lock_database_rw()
{
	assert (dbname != "" && dblock == -1);
	
	string name = dbname + ".writer";
	dblock = ::open(name.c_str(), O_RDWR | O_CREAT, cmode);
	return exclusive_file_lock(dblock);
}

void DbMan::unlock_database_rw()
{
	if (dblock == -1) return;
	
	unlock_file_lock(dblock);
	close(dblock);
	dblock = -1;
}

int DbMan::snapshot(View& view)
{
	int ok = lock_snapshot_ro();
	if (ok != 0) return ok;
	
	Parameters p(1,8,1);
	if (scanFile(p) != 0)	// get the ids
	{
		unlock_snapshot_ro();
		return -1;
	}
	
	view.files.clear();
	for (std::set<string>::iterator i = ids.begin(); i != ids.end(); ++i)
	{
		string name = dbname + "." + *i;
		int fd = ::open(name.c_str(), O_RDONLY);
//		std::cout << "opening: " <<  name << ": " << fd << std::endl;
		if (fd == -1)
		{
			unlock_snapshot_ro();
			return -1;
		}
		view.files.insert(File(*i, fd, &view.params));
	}
	
	unlock_snapshot_ro();
	return 0;
}

int DbMan::commit(const View& view)
{
	int ok = lock_snapshot_rw();
	if (ok != 0) return ok;
	
	assert (dblock != -1); // must be lock_database_rw'd
	
	ids.clear();
	for (View::Files::const_iterator i = view.files.begin(); i != view.files.end(); ++i)
		ids.insert(i->id);
	
	fseek(dbfile, 0, SEEK_SET);
	ftruncate(fileno(dbfile), 0);
	
	fprintf(dbfile, "%d %ld %ld\n", 
		view.params.version(), 
		view.params.blockSize(), 
		view.params.keySize());
	for (std::set<string>::iterator i = ids.begin(); i != ids.end(); ++i)
		fprintf(dbfile, "%s\n", i->c_str());
	
	fflush(dbfile);
	fsync(fileno(dbfile));
	
	unlock_snapshot_rw();
	return 0;
}

int DbMan::openNew(string& id)
{
	char ext[4];
	ext[0] = 'i';
	ext[3] = 0;
	
	do
	{
		ext[1] = 'a' + (random() % 26);
		ext[2] = 'a' + (random() % 26);
	} while (ids.find(ext) != ids.end());
	
	string name = dbname + "." + ext;
	int fd = ::open(name.c_str(), O_RDWR | O_CREAT | O_TRUNC, cmode);
	if (fd == -1) return -1;
	
	id = ext;
	return fd;
}

int DbMan::killSub(const string& id)
{
	string name = dbname + "." + id;
	if (unlink(name.c_str()) != 0) return -1;
	return 0;
}

}
