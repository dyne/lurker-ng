/*  $Id: View.cpp,v 1.8 2003-05-07 15:43:13 terpstra Exp $
 *  
 *  View.cpp - Snapshot of commit state
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

#include "View.h"
#include "Merger.h"
#include "Source.h"
#include "File.h"
#include "DbMan.h"
#include "Failer.h"

#include <cerrno>

namespace ESort
{

int View::rawseek(Merger* out, const string& k, bool forward)
{
	/* The goal is to find the last sector in each file whose first key
	 * is < k. If we are forward reading, merging from here makes no
	 * mistakes since the first key >= lies ahead. If we are backward
	 * reading, merging from here makes no mistakes since the first
	 * key not in the sector is >= k and thus too big for it.
	 */
	 
	// Push in decreasing size
	for (Files::iterator i = files.begin(); i != files.end(); ++i)
	{
		/* Binary search till we find key */
		int stop = 0;
		long left = 0;
		long right = i->blocks;
		long mid = -1;
		std::auto_ptr<Source> s(0);
		
		// consider starting s at [left, right)
		while ((right - left) > 1)
		{
			// mid is rounded down -> mid < right
			mid = (left + right) / 2;
			
			// Use a temporary to work around g++ 2.95 brokenness
			auto_ptr<Source> st(const_cast<File*>(&*i)->openBlock(mid, true));
			s = st;
			
			if (!s.get()) return -1;
			
			// eof is impossible since mid < right
			if ((stop = s->advance()) == -1) return -1;
			
			if (s->dup != 0) // first key; no compression
			{
				errno = EINVAL;
				return -1;
			}
			
			if (string(
				// std::string compares like unsigned char
				reinterpret_cast<const char*>(s->tail), 
				s->length) >= k)
				right = mid;
			else	left  = mid;
		}
		
		if (forward && mid != left) // s hold mid
		{
			// Work around g++ 2.95 bug
			auto_ptr<Source> st(const_cast<File*>(&*i)->openBlock(left, true));
			s = st;
			
			if (!s.get()) return -1;
			
			// empty File is impossible
			if ((stop = s->advance()) == -1)
			{
				if (errno == 0) errno = EINVAL;
				return -1;
			}
		}
		else if (!forward)
		{
			// Work around g++ 2.95 bug
			auto_ptr<Source> st(const_cast<File*>(&*i)->openBlock(left, false));
			s = st;
			
			if (!s.get()) return -1;
			
			// empty File is impossible
			if ((stop = s->advance()) == -1)
			{
				if (errno == 0) errno = EINVAL;
				return -1;
			}
		}
		
		if (s->dup != 0)
		{	// must be uncompressed!
			errno = EINVAL;
			return -1;
		}
		
		out->merge(s.release());
	}
	
	return 0;
}

auto_ptr<Walker> View::seek(const string& k, Direction dir)
{
	assert (dir == Forward || dir == Backward);
	
	auto_ptr<Merger> out(new Merger(params.unique(), dir == Forward));
	
	if (rawseek(out.get(), k, dir == Forward) != 0)
		return auto_ptr<Walker>(new Failer(errno));
	
	if (out->skiptill(k, dir == Forward) == -1)
		return auto_ptr<Walker>(new Failer(errno));
	
	return auto_ptr<Walker>(out);
}

auto_ptr<Walker> View::seek(const string& pfx, const string& k, Direction dir)
{
	assert (dir == Forward || dir == Backward);
	
	auto_ptr<PrefixMerger> out(new PrefixMerger(params.unique(), dir == Forward));
	
	if (rawseek(out.get(), pfx + k, dir == Forward) != 0)
		return auto_ptr<Walker>(new Failer(errno));
	
	if (out->skiptill(pfx, k, dir == Forward) == -1)
		return auto_ptr<Walker>(new Failer(errno));
	
	return auto_ptr<Walker>(out);
}

auto_ptr<Reader> Reader::open(const string& db)
{
	DbMan man;
	
	auto_ptr<View> out(new View(Parameters(1,8,1)));
	
	if (man.open(*out.get(), db) != 0)
		return auto_ptr<Reader>(0);
	
	return auto_ptr<Reader>(out);
}

Reader::~Reader()
{
}

}
