/*  $Id: View.h,v 1.2 2003-08-16 18:36:21 terpstra Exp $
 *  
 *  View.h - Snapshot of commit state
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

#ifndef VIEW_H
#define VIEW_H

#include "esort.h"
#include "Merger.h"
#include "File.h"

#include <set>

namespace ESort
{

class Merger;

class View : public Reader
{
 public:
 	struct FileOrder
 	{
 		bool operator() (const File& x, const File& y) const
 		{ return x.category < y.category; }
 	};
 	
 	typedef std::multiset<File, FileOrder> Files;
 	
 	Parameters params;
 	View(const Parameters& p) : params(p) { }
 	
 	/** Collection of all files open - in INcreasing size.
 	 */
 	Files	files;
 	
 	int rawseek(Merger* out, const string& k, bool forward);
 	
 	auto_ptr<Walker> seek(const string& k, Direction dir);
 	auto_ptr<Walker> seekp(const string& pfx, const string& k, Direction dir);
};

}

#endif
