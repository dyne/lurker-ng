/*  $Id: Search.h,v 1.4 2003-04-25 16:38:18 terpstra Exp $
 *  
 *  Search.h - Helper which can intersect keywords
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
 
#ifndef SEARCH_H
#define SEACH_H

#include <list>
#include "Summary.h"

using namespace ESort;

class Search
{
 protected:
 	struct KEntry
 	{
 		Walker*	walker;
 		long	skip;
 	};
 	
	typedef std::list<KEntry> Entries;
	
	Reader*		db;
	Direction	dir;
	MessageId	id;
	
	bool	eof;
	Entries	entries;
	
	static MessageId top(Entries::iterator i);
	string Search::advance(Entries::iterator i);
	
 public:
 	Search(Reader* db_, Direction dir_, const MessageId& id_);
 	~Search();
 	
 	string keyword(const string& key);
 	
 	string pull(int n, vector<Summary>& o);
};

#endif
