/*  $Id: Memory.h,v 1.2 2003-08-17 14:39:44 terpstra Exp $
 *  
 *  Memory.h - Memory segment for inserts prior to commit
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

#ifndef MEMORY_H
#define MEMORY_H

#include "esort.h"
#include <set>

namespace ESort
{

class Source;
class Parameters;
class Memory
{
 protected:
 	typedef std::multiset<string> Data;
 	Data data;
 
 public:
 	/** Insert a key into the memory.
 	 */
 	void insert(const string& k)
 	{
 		data.insert(k);
 	}
 	
 	/** Flush out memory after commit.
 	 */
 	void flush()
 	{
 		data.clear();
 	}
 	
 	/** Empty?
 	 */
 	bool empty()
 	{
 		return data.empty();
 	}
 	
 	/** Summarize the size used.
 	 */
 	int category(const Parameters& p) const;
 	
 	/** Find the first key >= k.
 	 *  Always succeeds
 	 */
 	auto_ptr<Source> openMemory(const string& k, bool forward);
};

}

#endif
