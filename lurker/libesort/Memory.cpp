/*  $Id: Memory.cpp,v 1.2 2003-04-21 18:25:32 terpstra Exp $
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

#include "Memory.h"
#include "Source.h"
#include "esort.h"
#include "File.h"

namespace ESort
{

class MemorySource : public Source
{
 protected:
 	typedef std::multiset<std::string> Data;
 	Data::iterator i, e;
 
 public:
 	MemorySource(Data::iterator i_, Data::iterator e_)
 	 : i(i_), e(e_) { }
 	
 	int advance();
};

int MemorySource::advance()
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

Source* Memory::openMemory(const string& k)
{
	return new MemorySource(data.lower_bound(k), data.end());
}

int Memory::category(const Parameters& p) const
{
	long sum = 0;
	for (Data::const_iterator i = data.begin(); i != data.end(); ++i)
		sum += i->length();
	
	return categorize((sum / p.blockSize()) + 1); //!!! +1 is evil
}

}
