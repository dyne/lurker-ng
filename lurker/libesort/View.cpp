/*  $Id: View.cpp,v 1.2 2003-04-21 18:25:32 terpstra Exp $
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

namespace ESort
{

Merger* View::rawseek(const string& k)
{
	Merger* out = new Merger(params.unique());
	
	// Push in decreasing size
	for (Files::iterator i = files.begin(); i != files.end(); ++i)
	{
		/* Binary search till we find key */
		int stop = 0;
		long left = 0;
		long right = i->blocks;
		long mid = -1;
		Source* s = 0;
		
		// consider starting s at [left, right)
		while ((right - left) > 1)
		{
			if (s) delete s;
			
			// mid is rounded down -> mid < right
			mid = (left + right) / 2;
			s = const_cast<File*>(&*i)->openBlock(mid);
			if (!s)
			{
				stop = -1;
				break; // should always work
			}
			
			// eof is impossible since mid < right
			if ((stop = s->advance()) == -1) break;
			
			assert (s->dup == 0); // first key; no compression
			
			if (string(
				// std::string compares like unsigned char
				reinterpret_cast<const char*>(s->tail), 
				s->length) >= k)
				right = mid;
			else	left  = mid;
		}
		if (stop == -1) continue;
		
		if (mid != left) // s hold mid
		{
			delete s;
			
			s = const_cast<File*>(&*i)->openBlock(left);
			if (!s) continue;
			// empty File?
			if ((stop = s->advance()) == -1) continue;
			assert (s->dup == 0);
		}
		
		out->merge(s);
	}
	
	return out;
}

Merger* View::seek(const string& k)
{
	Merger* out = rawseek(k);
	if (out->skiptill(k) == -1)
	{
		delete out;
		out = 0;
	}
	return out;
}

Reader* Reader::open(const string& db)
{
	DbMan man;
	Parameters p(1,4,1);
	
	if (man.open(db, p) != 0) return 0;
	
	View* o = new View(p);
	if (man.snapshot(*o) != 0)
	{
		delete o;
		o = 0;
	}
	
	return o;
}

Reader::~Reader()
{
}

}
