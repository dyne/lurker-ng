/*  $Id: View.cpp,v 1.4 2003-04-25 14:05:37 terpstra Exp $
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

auto_ptr<Merger> View::rawseek(const string& k, bool forward)
{
	auto_ptr<Merger> out(new Merger(params.unique(), forward));
	
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
			s = const_cast<File*>(&*i)->openBlock(mid, true);
			if (!s.get()) return auto_ptr<Merger>(0);
			
			// eof is impossible since mid < right
			if ((stop = s->advance()) == -1) return auto_ptr<Merger>(0);
			
			if (s->dup != 0) // first key; no compression
			{
				errno = EINVAL;
				return auto_ptr<Merger>(0);
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
			s = const_cast<File*>(&*i)->openBlock(left, true);
			if (!s.get()) return auto_ptr<Merger>(0);
			
			// empty File?
			if ((stop = s->advance()) == -1)
			{
				if (errno == 0) continue; // eof
				return auto_ptr<Merger>(0); // fucked?
			}
		}
		else if (!forward)
		{
			s = const_cast<File*>(&*i)->openBlock(left, false);
			if (!s.get()) return auto_ptr<Merger>(0);
			
			// empty File?
			if ((stop = s->advance()) == -1)
			{
				if (errno == 0) continue;
				return auto_ptr<Merger>(0); // fucked?
			}
		}
		
		if (s->dup != 0)
		{	// must be uncompressed!
			errno = EINVAL;
			return auto_ptr<Merger>(0);
		}
		
		out->merge(s.release());
	}
	
	return out;
}

auto_ptr<Walker> View::seek(const string& k, bool forward)
{
	auto_ptr<Merger> out = rawseek(k, forward);
	if (!out.get() || out->skiptill(k, forward) == -1)
		return auto_ptr<Walker>(new Failer(errno));
	
	return auto_ptr<Walker>(out);
}

auto_ptr<Reader> Reader::open(const string& db)
{
	DbMan man;
	Parameters p(1,8,1);
	
	if (man.open(db, p) != 0)
	{
		auto_ptr<View> o(new View(p));
		if (man.snapshot(*o.get()) == 0)
			return auto_ptr<Reader>(o);
		// else return 0
	}
	// else return 0
}

Reader::~Reader()
{
}

}
