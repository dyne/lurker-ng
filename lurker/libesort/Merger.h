/*  $Id: Merger.h,v 1.2 2003-04-21 18:25:32 terpstra Exp $
 *  
 *  Merger.h - Combine segments to obtain a database view
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

#ifndef MERGER_H
#define MERGER_H

#include "esort.h"

#include <vector>

namespace ESort
{

class Source;

class Merger : public Walker
{
 protected:
 	struct Element
 	{
 		Source* source;
 		Source* min;
 		
 		Element(Source* s) : source(s), min(0) { }
 	};
 	
 	typedef std::vector<Element> Sources;
 	
 	bool			unique;
 	
 	/** Invariant:
 	 *   for sources.begin() <= j < point:
 	 *    j->min->key = min { k->key: sources.begin() <= k <= j }
 	 */
 	Sources			sources;
 	Element*		point;
 	Element*		bov;
 	Element*		eov;
 	
 public:
 	Merger(bool unique_) : unique(unique_), sources() { }
 	~Merger(); /* deletes all sources */
 	
 	/** Add a Source to those which the merger merges.
 	 *  Please add Sources in increasing magnitude.
 	 *  Ensure that the source has been pre-advanced!
 	 *  You must guaranteed s->dup == 0.
 	 *  Never merge on something which is at EOF!
 	 */
 	void merge(Source* source)
 	{
// 		assert (source->dup == 0 && source->tail != 0);
 		sources.push_back(source);
 	}
 	
 	/* Implement the advance method.
 	 */
 	int advance();
 	
 	/** Skip until we have something >= k
 	 *  0 on success, -1 on error.
 	 */
 	int skiptill(const string& k);
};

}

#endif
