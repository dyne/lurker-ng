/*  $Id: Source.h,v 1.2 2003-08-17 14:39:44 terpstra Exp $
 *  
 *  Source.h - Common segment interface
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

#ifndef SOURCE_H
#define SOURCE_H

namespace ESort
{

/** One of the database fragments.
 */
class Source
{
 public:
 	virtual ~Source();
 	
 	/** The tail of the returned key.
 	 *  The length is for the whole key.
 	 *  The tail includes only what is not compressed.
 	 *  -- these are not initialized till first call of advance
 	 *
 	 * The tail MUST be unsigned. Why? Because std::string compares
 	 * as though it were unsigned. ie: '\0x7f' < '\0x80' even though
 	 * if it were really lexicographical compare it wouldn't.
 	 */
 	const unsigned char*	tail;
 	unsigned long		length;
 	unsigned int		dup;	// amount in common with last
 	
 	/** Advance to the next key in the source.
 	 *  -1 on error, errno = 0 -> eof.
 	 */
 	virtual int advance() = 0;
};

}

#endif
