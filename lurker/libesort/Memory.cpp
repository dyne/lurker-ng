/*  $Id: Memory.cpp,v 1.5 2003-06-23 14:38:42 terpstra Exp $
 *  
 *  Memory.cpp - Memory segment for inserts prior to commit
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

#include "Memory.h"
#include "Source.h"
#include "esort.h"
#include "File.h"

namespace ESort
{

class ForwardMemorySource : public Source
{
 protected:
 	typedef std::multiset<std::string> Data;
 	Data::iterator i, e;
 
 public:
 	ForwardMemorySource(Data::iterator i_, Data::iterator e_)
 	 : i(i_), e(e_) { }
 	
 	int advance();
};

int ForwardMemorySource::advance()
{
	// Test for eof
	if (i == e)
	{
		errno = 0;
		return -1;
	}
	
	// Return the key
	tail = reinterpret_cast<const unsigned char*>(i->c_str());
	length = i->length();
	dup = 0;
	++i;
	
	return 0;
}

class BackwardMemorySource : public Source
{
 protected:
 	typedef std::multiset<std::string> Data;
 	Data::iterator i, b;
 
 public:
 	BackwardMemorySource(Data::iterator i_, Data::iterator b_)
 	 : i(i_), b(b_) { }
 	
 	int advance();
};

int BackwardMemorySource::advance()
{
	// Test for eof
	if (i == b)
	{
		errno = 0;
		return -1;
	}
	
	// move to the next position
	--i;
	
	// Return the key
	tail = reinterpret_cast<const unsigned char*>(i->c_str());
	length = i->length();
	dup = 0;
	
	return 0;
}

auto_ptr<Source> Memory::openMemory(const string& k, bool forward)
{
	if (forward)
		return auto_ptr<Source>(
		new  ForwardMemorySource(data.lower_bound(k), data.end()));
	else	return auto_ptr<Source>(
		new BackwardMemorySource(data.lower_bound(k), data.begin()));
}

int Memory::category(const Parameters& p) const
{
	long sum = 0;
	for (Data::const_iterator i = data.begin(); i != data.end(); ++i)
		sum += i->length();
	
	return categorize((sum / p.blockSize()) + 1); //!!! +1 is evil
}

}
