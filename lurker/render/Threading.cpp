/*  $Id: Threading.cpp,v 1.2 2003-04-21 18:26:19 terpstra Exp $
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
 
#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

#include "Threading.h"
#include <Keys.h>
#include <memory>
#include <cerrno>
#include <iostream>

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

string Threading::load(ESort::Reader* r, const Summary& sum, Key& out)
{
	string find_start = 
		LU_THREADING + 
		subject_hash(sum.subject().c_str()) + 
		'\0' + 
		LU_BACKWARD + 
		sum.id().serialize_backward();
	
	auto_ptr<ESort::Walker> backwards(r->seek(find_start));
	if (!backwards.get())
	{
		if (errno == 0) return "does not exist? end of database";
		return string("Reader::seek:") + strerror(errno);
	}
	
	// Does it not exist?
	if (backwards->key.substr(0, find_start.length()) != find_start)
		return "does not exist";
	
	/** Walk backwards until we find step off the subject, or there is
	 *  a break of more than 40 days between messages.
	 */
	int j, l = find_start.length() - 8;
	MessageId root = sum.id();
	for (j = l; j >= l; j = backwards->advance())
	{
		if ((int)backwards->key.length() < l + 8)
			return "corrupt threading record -- too short";
		
		MessageId x(backwards->key.c_str() + l, false); // backwards
		if (x.timestamp() + 60*60*24*DAY_GAP_FOR_NEW_THREAD 
			< root.timestamp())
			break;
		
		root = x;
	}
	
	/** Ok, we have found what will be the root of the tree.
	 *  Now, we shall seek and read the messages off. To resolve reply-to
	 *  we keep a map of all the hashes we have seen.
	 */
	hashes.clear();
	
	string find_end = 
		LU_THREADING + 
		subject_hash(sum.subject().c_str()) + 
		'\0' + 
		LU_FORWARD + 
		root.serialize_forward();
	
	auto_ptr<ESort::Walker> forwards(r->seek(find_end));
	if (!forwards.get())
	{
		if (errno == 0) return "forward does not exist? end of database";
		return string("Reader::seek:") + strerror(errno);
	}
	
	// Does it not exist?
	if (forwards->key.substr(0, find_end.length()) != find_end)
		return "forward does not exist";
	
	/** Walk forwards until we find step off the subject, or there is
	 *  a break of more than 40 days between messages.
	 */
	MessageId prev = root;
	l = find_end.length() - 8;
	for (j = l; j >= l; j = forwards->advance())
	{
		if ((int)forwards->key.length() < l + 8)
			return "corrupt forwardthreading record -- too short";
		
		MessageId x(forwards->key.c_str() + l, true); // forwards
		
		if (prev.timestamp() + 60*60*24*DAY_GAP_FOR_NEW_THREAD 
			< x.timestamp())
			break;
		
		nodes.push_back(Node(x));
		Node& b = nodes.back();
		
		if (b.summary.id() == sum.id())
			out = nodes.size()-1;
		
		b.replies       = 0;
		b.replyee       = -1;
		b.replyor_first = -1;
		b.replyor_next  = -1;
		b.draw_next     = -1;
		b.depth         = nodes.size()-1;
		b.consumed      = 0;
		
		/** Try all possibilities for the in-reply-to
		 */
		long candidates = (forwards->key.length() - l - 8) / 4;
		const char* cand = forwards->key.c_str() + l + 8;
		
		while (candidates)
		{
			if (hashes.find(string(cand, 4)) != hashes.end())
			{
				b.replyee = hashes[string(cand, 4)];
				nodes[b.replyee].replies++;
				break;
			}
			
			cand += 4;
			--candidates;
		}
		
		hashes[b.summary.id().hash()] = nodes.size()-1;
		prev = x;
	}
	
	/** Resolve back links
	 */
	Node* tree = &nodes[0];
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

string Threading::draw_tree(ESort::Reader* db)
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
	string& out)
{
	int col;
	int c;
	
	col = tree[p].column = tree[p].consumed;
	
	if (row < 3)
	{
		out = tree[p].summary.load(db);
		if (out != "") return -1;
		
		for (c = tree[p].replyor_first; c != -1; c = tree[c].replyor_next)
		{
			tree[c].consumed = col;
			col = my_service_draw_snippet(db, tree, c, row+1, out);
			if (col == -1) return -1;
		}
	}
	
	if (tree[p].replyor_first == -1 || row >= 3) col++;
	
	return col;
}

int my_service_pick_p(Threading::Node* tree, int root)
{
	int n = root;
	int p;
	
	if (tree[n].replyor_first == -1 && tree[n].replyee != -1)
		p = tree[n].replyee;
	else	p = n;
	
	if (tree[p].replyee != -1) p = tree[p].replyee;
	
	return p;
}

string Threading::draw_snippet(ESort::Reader* db, Key root)
{
	Threading::Node* tree = &nodes[0];
	string out;
	my_service_draw_snippet(db, tree, my_service_pick_p(tree, root), 0, out);
	return out;
}

void my_service_draw_snippet_row(
	ostream&		o,
	Threading::Node*	tree,
	int* draw_head, 
	int row,
	int hl)
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
	}
	
	/* Terminate the list */
	*draw_head = -1;
}

void Threading::draw_snippet_row(ostream& o, int* h, Key row, Key root)
{
	Threading::Node* tree = &nodes[0];
	if (*h == -2) *h = my_service_pick_p(tree, root);
	my_service_draw_snippet_row(o, tree, h, row, root);
}
