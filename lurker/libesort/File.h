/*  $Id$
 *  
 *  Master.h - Disk segment for commit'd inserts
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

#ifndef FILE_H
#define FILE_H

#include <cmath>
#include "esort.h"

namespace ESort
{

class Source;
class FileSource;

inline int categorize(long records)
{
	return static_cast<int>(floor(log(static_cast<double>(records+2))/log(2)));
}


class File
{
 protected:
 	int fd;
 	const Parameters* p;
 	long where;
 	
 public:
 	/** Use this file descriptor for operation
 	 */
 	File(const string& id = "", int fd_ = -1, const Parameters* p_ = 0);
 	File(const File& o); 
 	~File(); // closes fd
 	
 	/** The id of the database.
 	 */
 	string id;
 	
 	/** The number of blocks in this file
 	 */
 	long blocks;
 	
 	/** The classification based on size for this file
 	 */
 	int category;
 	
 	/** Always succeeds */
 	auto_ptr<Source> openBlock(long block, bool forward);
 
 friend class FileSource;
};

}

#endif
