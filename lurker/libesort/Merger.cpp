/*  $Id: Merger.cpp,v 1.7 2003-04-25 21:06:10 terpstra Exp $
 *  
 *  Merger.cpp - Combine segments to obtain a database view
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

#include "Merger.h"
#include "Source.h"

#include <iostream>

namespace ESort
{

Walker::~Walker()
{
}

Merger::~Merger()
{
	for (Sources::iterator i = sources.begin(); i != sources.end(); ++i)
		delete i->source;
}

int Merger::advance()
{
	// Did we last consume an empty queue?
	if (point == eov)
	{
		if (restart != 0)
		{	// tell the user the first element is there
			point = restart;
			restart = 0;
			return 0;
		}
		else
		{	// we are at the eof
			errno = 0;
			return -1;
		}
	}
	
	if (point == bov)
	{
		// No need to correct the base-case; always right.
		++point;
	}
	
	// Correct the compression for min.
	Source* fix = (point-1)->min;
	unsigned int upto;
	if (fix->length > key.length())
		upto = key.length();
	else	upto = fix->length;
	
	while (fix->dup < upto && 
	       fix->tail[0] == ((unsigned char)key[fix->dup]))
	{
		++fix->tail;
		++fix->dup;
	}
	
	// Correct the invariant to the eov
	while (point != eov)
	{
		// First, fix compression
		fix = point->source;
		
		if (fix->length > key.length())
			upto = key.length();
		else	upto = fix->length;
		
		while (fix->dup < upto && 
		       fix->tail[0] == ((unsigned char)key[fix->dup]))
		{
			++fix->tail;
			++fix->dup;
		}
		
		// Now update the invariant.
		// Logically we are doing:
		// if (point->key < (point-1)->min->key)
		//      point->min = point;
		// else point->min = (point-1)->min;
		
		Source*  ms = (point-1)->min;
		Source*  ps = point->source;
		
		// The key with the most in common must be smaller
		// (it must be larger if going backwards too)
		if (ps->dup < ms->dup)
		{	// min has more in common!
			point->min = ms;
		}
		else if (ps->dup > ms->dup)
		{	// point has more in common
			point->min = ps;
		}
		else
		{	// string compare
			const unsigned char* mt = ms->tail;
			const unsigned char* pt = ps->tail;
			unsigned int ml = ms->length - ms->dup;
			unsigned int pl = ps->length - ps->dup;
			
			const unsigned char* me = mt;
			if (ml < pl)
				me += ml;
			else	me += pl;
			
			
			while (mt != me && *mt == *pt)
			{
				++mt;
				++pt;
			}
			
			if (*mt < *pt)
			{	// min is smaller.
				point->min = forward ? ms : ps;
			}
			else if (*mt > *pt)
			{	// point is smaller
				point->min = forward ? ps : ms;
			}
			else if (ml < pl)
			{	// point is longest, so min is smaller
				point->min = forward ? ms : ps;
			}
			else
			{
				point->min = forward ? ps : ms;
			}
		}
		
		++point;
	}
	
	// Find the smallest element via pointer comparison
	while (1)
	{
		// Move back a step, the range satisfies the invariant.
		--point;
		
		// Is this the smallest?
		Source* s;
		if (point->min == (s = point->source))
		{
			// Consume this record. This invalidates the invariant
			// for all records higher than here.
			
			// Move it in place
			key.resize(s->dup);
//			key.reserve(s->length);
			
			// std::string compares like it is unsigned.
			// so tail is kept as unsigned so we compare like
			// std::string, however, we thus need to cast it.
			key.append(
				reinterpret_cast<const char*>(s->tail), 
				s->length - s->dup);
			
			int out = s->dup;
			
			if (s->advance() != 0)
			{
				// Pass the error through
				if (errno != 0) return -1;
				
				// This entry is empty.
				Sources::iterator i = sources.begin();
				i += int(point - bov);
				
				// This may relocate the vector.
				delete i->source;
				sources.erase(i);
				
				// Correct ptrs
				bov = &*sources.begin();
				eov = bov + sources.size();
				point = bov;
				
				if (point != eov)
				{
					point->min = point->source;
				}
			}
			
			// Eliminate duplicates.
			// !!! kills off null string, erg.
			if (unique && out == (int)key.length())
				return advance();
			
			return out;
		}
	}
}

int Merger::skiptill(const string& k, bool forward)
{
	bov = &*sources.begin();
	eov = bov + sources.size();
	point = bov;
	
	if (point != eov)
	{	// establish invariant for the base node
		point->min = point->source;
	}
	
	if (forward)
	{
		// !!! this can be much quicker
		while (advance() != -1)
		{
			if (key >= k)
			{
				restart = point;
				point = eov;
				return 0;
			}
		}
	}
	else
	{
		while (advance() != -1)
		{
			if (key < k)
			{
				restart = point;
				point = eov;
				return 0;
			}
//			else std::cout << "SKIP: " << key << std::endl;
		}
	}
	
	return -1;
}

int PrefixMerger::advance()
{
	int o;
	
	switch (state)
	{
	case pSKIP:
		// pass through during skiptill
		return Merger::advance();
	
	case pFIRST:
		Merger::advance();
		state = pPART;
		return 0;
	
	case pPART:
		o = Merger::advance();
		if (o >= minDup) return o;
		if (o == -1) return -1;
		state = pEOF; // pass through
	
	case pEOF:
		errno = 0;
		return -1;
		
	default:
		assert (0);
		errno = EINVAL;
		return -1;
	}
}

int PrefixMerger::skiptill(const string& pfx, const string& x, bool forward)
{
	// First get in position
	if (Merger::skiptill(pfx + x, forward) != 0) return -1;
	
	// This much must be in common
	minDup = pfx.length();
	state  = pFIRST;
	
	// Use the fact we already loaded the first key
	if (key.substr(0, minDup) != pfx)
	{	// already hit eof
		state = pEOF;
	}
	
	return 0;
}

}
