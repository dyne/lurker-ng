/*  $Id: Master.cpp,v 1.13 2003-07-01 14:05:52 terpstra Exp $
 *  
 *  Master.cpp - Coordinate commit+read interface
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
  
#include "io.h"

#include "Master.h"
#include "Source.h"
#include "Failer.h"
#include "Transaction.h"

#include <list>
#include <iostream>
#include <cerrno>
#include <cassert>

namespace ESort
{

Master::Master(const Parameters& p)
 : view(p), memory(), man()
{
}

Master::~Master()
{
}

int Master::init(const string& db, int mode)
{
	if (man.dbopen(view, db, mode) != 0) return -1;
	
	return 0;
}

struct CleanupHelper
{
 public:
 	DbMan*	man;
 	int	fd;
 	string	id;
 	
 	CleanupHelper(DbMan* man_) : man(man_), fd(-1) { }
 	~CleanupHelper()
 	{
 		if (fd != -1)
 		{
 			close(fd);
 			man->killSub(id);
 		}
 	}
};

int Master::commit()
{
	if (memory.empty()) return 0;
	
	CleanupHelper scopedData(&man);
	scopedData.fd = man.openNew(scopedData.id);
	if (scopedData.fd == -1) return -1;
	
	Transaction tran(scopedData.fd, &view.params);
	Merger merge(view.params.unique(), true); // forward
	auto_ptr<Source> m(memory.openMemory("", true)); // forward
	int ok = m->advance();
	assert (ok != -1); // memory was already checked -> not empty!
	merge.merge(m.get());
	m.release(); // seperate in case of exception above
	
	// What category is our RAM?
	int category = memory.category(view.params);
//	std::cout << "Category: " << category << std::endl;
	
	View::Files::iterator kill;
	for (kill = view.files.begin(); kill != view.files.end(); ++kill)
	{
//		std::cout << "Merging: " << kill->category << std::endl;
		if (kill->category > category)
			break; // keep this one
		
		// If we have at least one of the same size, roll up as in
		// binary addition
		if (kill->category == category)
			category++;
		
		auto_ptr<Source> f(
			const_cast<File*>(&*kill)->openBlock(0, true));
		if (!f.get()) return -1; // something broke?
		if (f->advance() == -1) return -1; // always has keys?!
		merge.merge(f.get());
		f.release(); // above might throw
	}
	
	// The list of ids which we commit
	std::set<string> keepIds;
	keepIds.insert(scopedData.id); // the new id
	
	// keep all of these ids
	for (View::Files::iterator keep = kill; keep != view.files.end(); ++keep) 
		keepIds.insert(keep->id);
	
	if (merge.skiptill("", true) != 0)
		return -1; // must work?! ram has entries
	
	int dup;
	while ((dup = merge.advance()) != -1)
	{	// there is stuff to merge
		if (tran.write(merge.key.length(), dup, merge.key.c_str()) != 0)
			return -1;
	}
	if (tran.finish() != 0)
		return -1;
	
	// Commit the summary information
	if (man.commit(view.params, keepIds) != 0)
		return -1;
	
	// done with the buffer
	memory.flush();
	
	// Queue useless files for delete
	std::set<string> killIds;
	for (View::Files::iterator zap = view.files.begin(); zap != kill; ++zap)
		killIds.insert(zap->id);
	
	// Purge useless files from the View (closes them)
	view.files.erase(view.files.begin(), kill);
	
	// Now that they are closed, delete them
	for (std::set<string>::iterator del = killIds.begin(); del != killIds.end(); ++del)
		man.killSub(*del);
	
	// Move the transaction to a File; thus becoming a part of the view
	int fd = scopedData.fd;
	scopedData.fd = -1;
	view.files.insert(File(scopedData.id, fd, &view.params));
	
	return 0;
}

int Master::insert(const string& k)
{
	assert (k.length() < view.params.keySize());
	
	// always succeeds
	memory.insert(k);
	return 0;
}

auto_ptr<Walker> Master::seek(const string& k, Direction dir)
{
	assert (dir == Forward || dir == Backward);
	
	auto_ptr<Merger> out(new Merger(view.params.unique(), dir == Forward));
	
	if (view.rawseek(out.get(), k, dir == Forward) != 0)
		return auto_ptr<Walker>(new Failer(errno));
	
	auto_ptr<Source> s = memory.openMemory(k, dir == Forward);
	assert (s.get()); // always works
	
	// only possible error is eof
	if (s->advance() != -1) out->merge(s.release());
	// else kill it on scope out
	
	if (out->skiptill(k, dir == Forward) == -1)
		return auto_ptr<Walker>(new Failer(errno));
	
	return auto_ptr<Walker>(out);
}

auto_ptr<Walker> Master::seek(const string& pfx, const string& k, Direction dir)
{
	assert (dir == Forward || dir == Backward);
	
	auto_ptr<PrefixMerger> out(new PrefixMerger(view.params.unique(), dir == Forward));
	
	if (view.rawseek(out.get(), pfx + k, dir == Forward) != 0)
		return auto_ptr<Walker>(new Failer(errno));
	
	auto_ptr<Source> s = memory.openMemory(k, dir == Forward);
	assert (s.get()); // always works
	
	// only possible error is eof
	if (s->advance() != -1) out->merge(s.release());
	// else kill it on scope out
	
	if (out->skiptill(pfx, k, dir == Forward) == -1)
		return auto_ptr<Walker>(new Failer(errno));
	
	return auto_ptr<Walker>(out);
}

auto_ptr<Writer> Writer::opendb(const string& db, const Parameters& p, int mode)
{
	auto_ptr<Master> m(new Master(p));
	
	if (m->init(db, mode) != 0)
		return auto_ptr<Writer>(0);
	
	return auto_ptr<Writer>(m);
}

}
