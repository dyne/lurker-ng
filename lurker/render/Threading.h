/*  $Id: Threading.h,v 1.3 2003-07-01 12:39:40 terpstra Exp $
 *  
 *  Threading.h - Helper which can load a thread tree
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
 
#ifndef THREADING_H
#define THREADING_H

#include <MessageId.h>
#include <esort.h>
#include <set>
#include <map>
#include <vector>
#include <memory>

#include "Summary.h"

using std::set;
using std::map;
using std::ostream;

class Threading
{
 public:
 	struct Node
 	{
 		Summary		summary;
 		
 		int		replies;
 		int		replyee;
 		int		replyor_first;
 		int		replyor_next;
 		
 		int		depth;
 		int		consumed;
 		int		column;
 		int		draw_next;
 		
 		string		in_reply_tos;
 		
 		Node(const MessageId& id_) : summary(id_) { }
 		Node() : summary(MessageId()) { }
 	};
 	
 protected:
 	typedef std::vector<Node> Nodes;
 	Nodes nodes;
	map<string, int> hashes;
	
 public:
 	typedef Nodes::size_type Key;
 	
 	string load(ESort::Reader* r, const Summary& sum, Key& out); // "" = ok
 	
 	Key	size()      const { return nodes.size(); }
 	Key	root()      const { return 0; }
 	Key	prev(Key m) const { return (m > 0)              ? m-1 : m; }
 	Key	next(Key m) const { return (m < nodes.size()-1) ? m+1 : m; }
 	
 	bool hasMessage(string hash) { return hashes.find(hash) != hashes.end(); }
 	
 	set<Summary> replies(Key k);
 	Summary& getSummary(Key m);
 	
 	string draw_tree   (ESort::Reader* db);	// prepare for row drawing
 	string draw_snippet(ESort::Reader* db, Key root);
 	
 	void draw_tree_row   (ostream& o, int* h, Key row); // "" = ok
 	void draw_snippet_row(ostream& o, int* h, Key row, Key root); // "" = ok
};

#endif
