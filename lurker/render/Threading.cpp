/*  $Id: Threading.cpp,v 1.10 2003-07-01 12:39:39 terpstra Exp $
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
 
#define _XOPEN_SOURCE 500
#define _FILE_OFFSET_BITS 64

#include "Threading.h"
#include <Keys.h>
#include <memory>
#include <cerrno>
#include <iostream>
#include <list>

#define	DAY_GAP_FOR_NEW_THREAD	40

#define	EMPTY_CELL	"<a/>"
#define BAR_NS		"<b/>"
#define BAR_EW		"<c/>"
#define CORNER_SW	"<d/>"
#define TEE_WSE		"<e/>"

#define MESSAGE_END	'f'
#define MESSAGE_DOWN	'g'
#define MESSAGE_BOTH	'h'
#define TOPMESSAGE_END	'i'
#define TOPMESSAGE_DOWN	'j'
#define TOPMESSAGE_BOTH	'k'

using namespace std;
using namespace ESort;

string Threading::load(Reader* r, const Summary& sum, Key& out)
{
	// dump any prior state
	hashes.clear();
	nodes.clear();
	
	string prefix = 
		LU_THREADING + 
		subject_hash(sum.subject().c_str());
	
	auto_ptr<Walker> backwards(r->seek(prefix, sum.id().raw(), Backward));
	
	/** Walk backwards until we find step off the subject, or there is
	 *  a break of more than 40 days between messages.
	 */
	MessageId root = sum.id();
	while (backwards->advance() != -1)
	{
		if (backwards->key.length() < prefix.length() + 8)
			return "corrupt threading record -- too short";
		
		MessageId x(backwards->key.c_str() + prefix.length(), 1);
		if (x.timestamp() + 60*60*24*DAY_GAP_FOR_NEW_THREAD 
			< root.timestamp())
			break;
		
		root = x;
	}
	
	/** Ok, we have found what will be the root of the tree.
	 *  Now, we shall seek and read the messages off. 
	 */
	auto_ptr<Walker> forwards(r->seek(prefix, root.raw(), Forward));
	
	/* We read the nodes off in timestamp sorted order
	 */
	std::list<Node> timestamp_sorted;
	
	/** Walk forwards until we find step off the subject, or there is
	 *  a break of more than 40 days between messages.
	 */
	MessageId prev = root;
	while (forwards->advance() != -1)
	{
		if (forwards->key.length() < prefix.length() + 8)
			return "corrupt forwardthreading record -- too short";
		
		MessageId x(forwards->key.c_str() + prefix.length(), 1);
		
		if (prev.timestamp() + 60*60*24*DAY_GAP_FOR_NEW_THREAD 
			< x.timestamp())
			break;
		
		timestamp_sorted.push_back(Node(x));
		Node& b = timestamp_sorted.back();
		
		b.in_reply_tos.assign(
			forwards->key.c_str() + prefix.length() + 8,
			forwards->key.length() - prefix.length() - 8);
		
		// record that this hash does in fact exist
		hashes[b.summary.id().hash()] = -1;
		
		prev = x;
	}
	
	/** We now have all the messages in timestamp sorted order.
	 *  Let's scan the list for the first message with no predecessor.
	 */
	while (!timestamp_sorted.empty())
	{
		std::list<Node>::iterator i;
		
		for (i = timestamp_sorted.begin(); i != timestamp_sorted.end(); ++i)
		{
			// scan all the in_reply_tos. if none of them are left
			// in timestamp_sorted, then this should come next.
			// (ie: no predecessors and lowest timestamp)
			
			i->replyee = -1; // find best predecessor also
			
			string::size_type replyto;
			for (replyto = 0; replyto+4 <= i->in_reply_tos.size(); replyto += 4)
			{
				map<string, int>::iterator r = hashes.find(
					i->in_reply_tos.substr(replyto, 4));
				
				if (r != hashes.end())
				{
					if (r->second == -1)
					{	// still in the timestamp queue
						break;
					}
					else if (r->second > i->replyee)
					{	// an older predecessor
						i->replyee = r->second;
					}
				}
			}
			
			// Did we find no queued predecessors?
			if (replyto+4 > i->in_replytos.size())
				break;
		}
		
		// deal with cycles (theorectically impossible, but ...)
		if (i == timestamp_sorted.end()) i = timestamp_sorted.begin();
		
		hashes[i->summary.id().hash()] = nodes.size();
		nodes.push_back(*i);
		timestamp_sorted.erase(i);
		
		// prep the node
		Node& b = nodes.back();
		b.replies       = 0;
		b.replyor_first = -1;
		b.replyor_next  = -1;
		b.draw_next     = -1;
		b.depth         = nodes.size() - 1;
		b.consumed      = 0;
		
		if (b.replyee != -1)
			nodes[b.replyee].replies++;
		
		// check for high-lighted node
		if (b.summary.id() == sum.id())
			out = b.depth;
	}
	
	Node* tree = &nodes[0];
	
	/** Resolve back links
	 */
	for (int i = nodes.size()-1; i > 0; i--)
	{
		int p = tree[i].replyee;
		if (p == -1) continue;
				
		if (tree[p].depth < tree[i].depth)
			tree[p].depth = tree[i].depth;
		
		/* Insertion sort is not that fast...
		 * But compared to the drawing algo, this n^2 is negligable.
		 */
		int* w = &tree[p].replyor_first;
		while (*w != -1 && tree[*w].depth > tree[i].depth)
			w = &tree[*w].replyor_next;
		
		tree[i].replyor_next = *w;
		*w                   = i;
	}
	
	return "";
}

set<Summary> Threading::replies(Key k)
{
	set<Summary> out;
	for (int l = nodes[k].replyor_first; l != -1; l = nodes[l].replyor_next)
		out.insert(nodes[l].summary);
	return out;
}

Summary& Threading::getSummary(Key m)
{
	return nodes[m].summary;
}

void my_service_tree_message_link(
	ostream&		o,
	Threading::Node*	tree,
	int 			i,
	int			hl)
{
	int  selected	= (hl == i);
//	int  drift	= 0; // !!! too bad I can't do this...
	char x;
	
	if (tree[i].replyee == -1)
	{
		switch (tree[i].replies)
		{
		case 0:  x = TOPMESSAGE_END;  break;
		case 1:  x = TOPMESSAGE_DOWN; break;
		default: x = TOPMESSAGE_BOTH; break;
		}
	}
	else
	{
		switch (tree[i].replies)
		{
		case 0:  x = MESSAGE_END;  break;
		case 1:  x = MESSAGE_DOWN; break;
		default: x = MESSAGE_BOTH; break;
		}
	}
	
	o << "<" << x;
	if (selected) o << " selected=\"yes\"";
	o << " id=\"" << tree[i].summary.id().serialize() << "\"/>";
}

int my_service_draw_tree(
	Threading::Node* tree,
	int n, int u)
{
	int i;
	int c;
	int col = 0;
	
	/* Find the column we can use. */
	for (i = u; i <= tree[n].depth; i++)
		if (col < tree[i].consumed)
			col = tree[i].consumed;
	
	tree[n].consumed = col;
	
	tree[n].column = col;
	for (c = tree[n].replyor_first; c != -1; c = tree[c].replyor_next)
	{
		col = my_service_draw_tree(tree, c, n); 
		for (i = n; i < c; i++)
			tree[i].consumed = col+1;
	}
	
	return tree[n].column;
}

string Threading::draw_tree(Reader* db)
{
	Threading::Node* tree = &nodes[0];
	int tree_size = nodes.size();
	
	for (int i = 0; i < tree_size; ++i)
		if (tree[i].replyee == -1)
			my_service_draw_tree(tree, i, i);
	
	return "";
}

void my_service_draw_tree_row(
	ostream&		o,
	Threading::Node*	tree,
	int* head, 
	int i)
{
	int	col, p;
	int*	w;

#ifdef DEBUG
	printf("\nOffset: %d", tree[i].column);
#endif
	
	col = 0;
	w = head;
	while (*w != -1)
	{
		for (; col < tree[*w].column; col++)
			o << EMPTY_CELL;
		
		if (*w == i) break;
		
		o << BAR_NS;
		col++;
		w = &tree[*w].draw_next;
	}
	
	for (; col < tree[i].column; col++)
		o << EMPTY_CELL;
	
	my_service_tree_message_link(o, tree, i, -1);
	col++;
	
	/* Cut ourselves out of the list and graft on our
	 * children. While we're at it, draw the links.
	 */
	for (p = tree[i].replyor_first; p != -1; p = tree[p].replyor_next)
	{
		*w = p;
		w = &tree[p].draw_next;
		
		if (col > tree[p].column) continue;
		
		for (; col < tree[p].column; col++)
			o << BAR_EW;
		col++;
		if (tree[p].replyor_next == -1)
			o << CORNER_SW;
		else	o << TEE_WSE;
	}
	*w = tree[i].draw_next;
	
	/* Continue drawing the other children */
	for (p = *w; p != -1; p = tree[p].draw_next)
	{
		for (; col < tree[p].column; col++)
			o << EMPTY_CELL;
		col++;
		o << BAR_NS;
	}
}

void Threading::draw_tree_row(ostream& o, int* h, Key row)
{
	my_service_draw_tree_row(o, &nodes[0], h, row);
}

int my_service_draw_snippet(
	ESort::Reader*	db,
	Threading::Node* tree,
	int p, 
	int row,
	string& out,
	int num)
{
	int col;
	int c;
	bool dangle_reply = false;
	
	col = tree[p].column = tree[p].consumed;
	
	if (row < 3)
	{
		out = tree[p].summary.load(db);
		if (out != "") return -1;
		
		for (c = tree[p].replyor_first; c != -1; c = tree[c].replyor_next)
		{
			tree[c].consumed = col;
			col = my_service_draw_snippet(db, tree, c, row+1, out, num);
			if (col == -1) return -1;
		}
		
		if (p+1 < num && tree[p+1].replyee == -1)
		{ // draw it as though it were a child
			dangle_reply = true;
			c = p+1;
			
			tree[c].consumed = col;
			col = my_service_draw_snippet(db, tree, c, row+1, out, num);
			if (col == -1) return -1;
		}
	}
	
	if ((tree[p].replyor_first == -1 && !dangle_reply) || row >= 3) col++;
	
	return col;
}

int my_service_pick_p(Threading::Node* tree, int root, int num)
{
	int p = root;
	
	// If we have nothing which is drawn below us move up an extra step
	if (tree[p].replyor_first == -1 && 
	    (p+1 >= num || tree[p+1].replyee != -1))
	{	// we have no children and we have no dangling replyee
		if (tree[p].replyee != -1)
			p = tree[p].replyee;
		else if (p != 0)
			p = p - 1; // use the dangling replyee trick
	}
	
	// always move up at least one row if we can
	if (tree[p].replyee != -1)
		p = tree[p].replyee;
	else if (p != 0)
		p = p - 1; // use dangling replyee trick
	
	return p;
}

string Threading::draw_snippet(ESort::Reader* db, Key root)
{
	Threading::Node* tree = &nodes[0];
	string out;
	my_service_draw_snippet(db, tree, 
		my_service_pick_p(tree, root, nodes.size()), 
		0, out, nodes.size());
	return out;
}

void my_service_draw_snippet_row(
	ostream&		o,
	Threading::Node*	tree,
	int* draw_head, 
	int row,
	int hl,
	int num)
{
	int	p;
	int	c;
	int	col = 0;

	/* First, draw the current row */
	for (p = *draw_head; p != -1; p = tree[p].draw_next)
	{
		for (; col < tree[p].column; col++)
			o << EMPTY_CELL;
		
		
		my_service_tree_message_link(o, tree, p, hl);
		col++;
		
		/* Now, inject our children in place.
		 * Also, draw the stuff down to them.
		 */
		for (c = tree[p].replyor_first; c != -1; c = tree[c].replyor_next)
		{
			*draw_head = c;
			draw_head = &tree[c].draw_next;
			
			if (c == tree[p].replyor_first)
				continue;
			
			for (; col < tree[c].column; col++)
				o << BAR_EW;
			
			if (tree[c].replyor_next == -1)
				o << CORNER_SW;
			else	o << TEE_WSE;
			col++;
		}
		
		// Check if the next message after p has no in-reply-to
		if (p+1 < num && tree[p+1].replyee == -1)
		{ // draw it as though it were a child
			*draw_head = p+1;
			draw_head = &tree[p+1].draw_next;
		}
	}
	
	/* Terminate the list */
	*draw_head = -1;
}

void Threading::draw_snippet_row(ostream& o, int* h, Key row, Key root)
{
	Threading::Node* tree = &nodes[0];
	if (*h == -2) *h = my_service_pick_p(tree, root, nodes.size());
	my_service_draw_snippet_row(o, tree, h, row, root, nodes.size());
}
