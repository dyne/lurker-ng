/*  $Id: Keyword.h,v 1.2 2003-04-21 18:26:19 terpstra Exp $
 *  
 *  Keyword.h - Helper which can stream keywords
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
 
#ifndef KEYWORD_H
#define KEYWORD_H

#include <esort.h>
#include <vector>

#include "Summary.h"

using std::vector;

class Keyword
{
 protected:
 	ESort::Reader*	reader;
 	ESort::Walker*	walker;
 	bool		forward;
 	bool		eof;
 	
 	string		prefix;
 	string		offset;
 	
 public:
 	Keyword(ESort::Reader* r = 0, bool forward = true, const string& prefix = "", const string& offset = "");
 	Keyword(const Keyword& o);
 	~Keyword();
 	
 	Keyword& operator = (const Keyword& o);
 	
 	void swap(Keyword& o);
 	
 	string pull(int n, vector<Summary>& o);
};

#endif
