/*  $Id: Master.cpp,v 1.2 2003-04-21 18:25:32 terpstra Exp $
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

#include "Master.h"
#include "Source.h"
#include "Transaction.h"

#include <list>
#include <iostream>

namespace ESort
{

Master::Master()
 : view(Parameters()), memory(), man()
{
}

Master::~Master()
{
	man.unlock_database_rw();
}

int Master::init(const string& db, const Parameters& p, int mode)
{
	view.params = p;
	
	if (man.open(db, view.params, mode) != 0) return -1;
	if (man.lock_database_rw()          != 0) return -1;
	if (man.snapshot(view)              != 0) return -1;
	
	return 0;
}

int Master::commit()
{
	if (memory.empty()) return 0;
	
	string id;
	int fd = man.openNew(id);
	if (fd == -1) return -1;
	
	Transaction tran(fd, &view.params);
	Merger merge(view.params.unique());
	Source* s = memory.openMemory("");
	if (s->advance() == -1) return -1;
	merge.merge(s);
	
	std::list<File> doomed;
	
	// What category is our RAM?
	int category = memory.category(view.params);
//	std::cout << "Category: " << category << std::endl;
	
	View::Files::iterator i, j;
	for (i = view.files.begin(); i != view.files.end(); i = j)
	{
//		std::cout << "Merging: " << i->category << std::endl;
		if (i->category > category)
			break; // keep this one
		
		// If we have at least one of the same size, roll up as in
		// binary addition
		if (i->category == category)
			category++;
		
		//!!! this bad because it prevents us from proper unwinding
		// -- fix it
		doomed.push_back(*i);
		s = doomed.back().openBlock(0);
		if (!s) return -1; // impossible
		if (s->advance() == -1) return -1; // impossible
		merge.merge(s);
		
		j = i;
		++j;
		view.files.erase(i);
	}
	
	for (int dup = merge.skiptill(""); dup != -1; dup = merge.advance())
	{	// there is stuff to merge
		if (tran.write(merge.key.length(), dup, merge.key.c_str()) != 0)
			return -1;
	}
	if (tran.finish() != 0)
		return -1;
	
	// Move the transaction to a File; thus becoming a part of the view
	view.files.insert(File(id, fd, &view.params));
	
	if (man.commit(view) != 0)
		return -1;
	
	// done with the buffer
	memory.flush();
	
	// Destroy useless files
	for (std::list<File>::iterator i = doomed.begin(); i != doomed.end(); ++i)
		man.killSub(i->id);
	
	return 0;
}

int Master::insert(const string& k)
{
	assert (k.length() < view.params.keySize());
	
	// always succeeds
	memory.insert(k);
	return 0;
}

Merger* Master::seek(const string& k)
{
	// might not have any results
	Merger* out = view.rawseek(k);
	assert (out);
	
	Source* s = memory.openMemory(k);
	if (s->advance() != -1)
		out->merge(s);
	else	delete s;
	
	if (out->skiptill(k) == -1)
	{
		delete out;
		out = 0;
	}
	
	return out;
}

Writer* Writer::open(const string& db, const Parameters& p, int mode)
{
	Master* m = new Master;
	if (m->init(db, p, mode) != 0)
	{
		delete m;
		return 0;
	}
	
	return m;
}

}
