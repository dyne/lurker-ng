/*  $Id: Keyword.h,v 1.4 2003-04-25 16:38:18 terpstra Exp $
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
using namespace ESort;

class Keyword
{
 protected:
 	auto_ptr<Walker>	walker;
 	string::size_type	skip;
 	
 public:
 	Keyword(Reader*          db = 0, 
 		Direction        dir = Forward, 
 		const string&    keyword = "", 
 		const MessageId& id = MessageId());
 	
 	string pull(int n, vector<Summary>& o);
};

#endif
