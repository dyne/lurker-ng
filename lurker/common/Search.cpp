/*  $Id: Search.cpp,v 1.3 2004-08-25 12:51:05 terpstra Exp $
 *  
 *  Search.cpp - Execute a keyword search
 *  
 *  Copyright (C) 2004 - Wesley W. Terpstra
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

#include <list>
#include <assert.h>

#include "Search.h"
#include "Keys.h"

using namespace std;

class Searcher
{
 public:
 	MessageId next_id; // not correct till first call of skipto
 	
 	/* Precoditions:
 	 *  has never yet returned EOF.
 	 *  later_than > next_id (wrt. the established direction)
         * Postconditions:
         *  Returns false on EOF
         *  Otherwise, next_id >= later_than (wrt. dir.)
 	 */
 	virtual bool skipto(MessageId later_than) = 0;
 	virtual ~Searcher();
};

Searcher::~Searcher()
{
}

class EmptySearcher : public Searcher // no messages
{
 public:
 	bool skipto(MessageId later_than)
 	{ return false; }
};

class CompleteSearcher : public Searcher // every possible message
{
 public:
 	bool skipto(MessageId later_than)
 	{ next_id = later_than; return true; }
};

class NotSearcher : public Searcher
{
 protected:
 	// Invariant: before first call, s->next_id == next_id
 	//            if s has ever hit EOF, s is null
 	//            else s->next_id is smallest > next_id (wrt. dir.)
 	auto_ptr<Searcher> s;
 	Direction d;
 
 public:
 	NotSearcher(Searcher* s_, Direction d_) : s(s_), d(d_) { }
 	bool skipto(MessageId later_than);
};

bool NotSearcher::skipto(MessageId later_than)
{
	if (s.get() && next_id == s->next_id) // detect startup
	{
		if (!s->skipto(later_than))
			s.reset();
	}
	
	next_id = later_than;
	while (s.get())
	{
		if (d == Forward)
		{
			if (next_id > s->next_id)
				if (!s->skipto(next_id))
					s.reset();
		}
		else
		{
			if (next_id < s->next_id)
				if (!s->skipto(next_id))
					s.reset();
		}
		
		if (!s.get() || s->next_id != next_id) break;
		
		if (d == Forward)
		{
			if (next_id.increment()) return false;
		}
		else
		{
			if (next_id.decrement()) return false;
		}
	}
	
	return true;
}

class AndSearcher : public Searcher
{
 protected:
 	typedef std::list<Searcher*> Parts;
 	
 	// Invariant: 
 	// all parts p: p->next_id == next_id (wrt. dir)
 	//              p has not returned EOF
 	// OR we have returned EOF
 	Direction d;
 	Parts parts;
 	
 public:
 	AndSearcher(Direction d_) : d(d_) { }
 	~AndSearcher();
 	bool skipto(MessageId later_than);
 	
 	void push(Searcher* s) { parts.push_back(s); }
 	Searcher* simplify();
};

AndSearcher::~AndSearcher()
{
	for (Parts::iterator i = parts.begin(); i != parts.end(); ++i)
		delete *i;
}

bool AndSearcher::skipto(MessageId later_than)
{
	assert (!parts.empty()); // simplify fixes this case
	
	/* First run, same source for all sub-problems -> precondition ok
	 * Otherwise, all p->next_ids == next_id < later_than -> precondition ok
	 */
	for (Parts::iterator i = parts.begin(); i != parts.end(); ++i)
		if (!(*i)->skipto(later_than)) return false;
	
	while (1)
	{
		Searcher* low  = parts.front();
		Searcher* high = parts.front();
		
		for (Parts::iterator i = parts.begin(); i != parts.end(); ++i)
		{
			if ((*i)->next_id < low ->next_id) low  = *i;
			if ((*i)->next_id > high->next_id) high = *i;
		}
		
		if (high->next_id == low->next_id)
		{
			next_id = high->next_id;
			break;
		}
		
		// Pre-condition satisfied b/c of ordering: low < high
		if (d == Forward)
		{
			if (!low->skipto(high->next_id))
				return false;
		}
		else
		{
			if (!high->skipto(low->next_id))
				return false;
		}
		
		// low might become highest - something else is now lowest
	}
	
	return true;
}

Searcher* AndSearcher::simplify()
{
	if (parts.empty())
	{
		delete this;
		return new CompleteSearcher();
	}
	if (parts.size() == 1)
	{
		Searcher* h = parts.front();
		parts.pop_front();
		delete this;
		return h;
	}
	
	return this;
}

class OrSearcher : public Searcher
{
 protected:
 	typedef std::list<Searcher*> Parts;
 	
 	// Invariant: 
 	// all parts p: first p->next_id >= next_id (wrt direction)
 	//              p has never returned EOF
 	Direction d;
 	bool first;
 	Parts parts;
 	
 public:
 	OrSearcher(Direction d_) : d(d_), first(true) { }
 	~OrSearcher();
 	bool skipto(MessageId later_than);
 	
 	void push(Searcher* s) { parts.push_back(s); }
 	Searcher* simplify();
};

OrSearcher::~OrSearcher()
{
	for (Parts::iterator i = parts.begin(); i != parts.end(); ++i)
		delete *i;
}

bool OrSearcher::skipto(MessageId later_than)
{
	Searcher* high = 0;
	Searcher* low  = 0;
	
	Parts::iterator i, j;
	for (i = parts.begin(); i != parts.end(); i = j)
	{
		j = i; ++j;
		
		if (first ||
		    (d ==  Forward && later_than > (*i)->next_id) ||
		    (d == Backward && later_than < (*i)->next_id))
		{
			if (!(*i)->skipto(later_than))
			{
				delete *i;
				parts.erase(i);
				continue;
			}
		}
		
		if (!low  || (*i)->next_id < low ->next_id) low  = *i;
		if (!high || (*i)->next_id > high->next_id) high = *i;
	}
	
	first = false;
	if (!low) return false;
	
	if (d == Forward)
		next_id = low->next_id;
	else	next_id = high->next_id;
	return true;
}

Searcher* OrSearcher::simplify()
{
	if (parts.empty())
	{
		delete this;
		return new EmptySearcher();
	}
	if (parts.size() == 1)
	{
		Searcher* h = parts.front();
		parts.pop_front();
		delete this;
		return h;
	}
	
	return this;
}

class WordSearcher : public Searcher
{
 protected:
 	int prefix;
 	Direction d;
 	auto_ptr<Walker> walker;
 	
 public:
 	WordSearcher(const Criterea& c, const string& word);
 	bool skipto(MessageId later_than);
};

WordSearcher::WordSearcher(const Criterea& c, const string& word)
 : d(c.dir),
   prefix(sizeof(LU_KEYWORD)-1 + word.length() + 1),
   walker(c.db->seek(LU_KEYWORD + word + '\0', c.source.raw(), c.dir))
{
}

bool WordSearcher::skipto(MessageId later_than)
{
	while ((d == Forward  && later_than > next_id) ||
	       (d == Backward && later_than < next_id))
	{
		// later_than > next_id, so we are free to blindly advance.
		int out = walker->advance();
		
		if (out == -1)
			return false; // hit end of file (errno == 0) ?
		
		if (walker->key.length() != prefix + MessageId::raw_len)
			return false; // corrupt!
		
		next_id = MessageId(walker->key.c_str() + prefix, 1);
	}
	
	return true;
}

Search::Search(const Config& cfg_, ESort::Reader* db, Direction dir, const MessageId& source)
 : cfg(cfg_)
{
	criterea.db  = db;
	criterea.dir = dir;
	criterea.source = source;
	
	root = new AndSearcher(dir);
	need_any = true; // until we see a search term without a '-' we need it
	
	// do not include deleted messages
	keyword(string("-") + LU_KEYWORD_DELETED);
}

Search::~Search()
{
	delete root;
}

void Search::keyword(const string& key)
{
	string word;
	if (key[0] == '-' || key[0] == '+')
		word.assign(key, 1, string::npos);
	else
		word = key;
	
	if (key[0] != '-')
		need_any = false;
	
	Searcher* s;
	if (word.substr(0, sizeof(LU_KEYWORD_GROUP)-1) == LU_KEYWORD_GROUP)
	{
		OrSearcher* o = new OrSearcher(criterea.dir);
		Config::Groups::const_iterator g = 
			cfg.groups.find(word.substr(sizeof(LU_KEYWORD_GROUP)-1, string::npos));
		if (g != cfg.groups.end())
		{
			Config::Members::iterator m;
			for (m = g->second.members.begin(); m != g->second.members.end(); ++m)
				o->push(new WordSearcher(criterea, LU_KEYWORD_LIST + *m));
		}
		s = o->simplify();
	}
	else if (word.substr(0, sizeof(LU_KEYWORD_LANGUAGE)-1) == LU_KEYWORD_LANGUAGE)
	{
		string lang(word, sizeof(LU_KEYWORD_LANGUAGE)-1, string::npos);
		OrSearcher* o = new OrSearcher(criterea.dir);
		for (Config::Lists::const_iterator l = cfg.lists.begin();
		     l != cfg.lists.end(); ++l)
		{
			if (l->second.language != lang) continue;
			o->push(new WordSearcher(criterea, LU_KEYWORD_LIST + l->first));
		}
		s = o->simplify();
	}
	else
	{
		s = new WordSearcher(criterea, word);
	}
	
	if (key[0] == '-')
		s = new NotSearcher(s, criterea.dir);
	root->push(s);
}

bool Search::pull(int n, vector<Summary>& o)
{
	if (need_any)
	{
		// if only negative terms are included, add this
		keyword(LU_KEYWORD_EVERYTHING);
		need_any = false;
	}
	
	while (n--)
	{
		if (!root->skipto(criterea.source))
			break;
		
		// cout << "pushing: " << root->next_id.serialize() << "\n";
		o.push_back(Summary(root->next_id));
		criterea.source = root->next_id;
		
		if (criterea.dir ==  Forward && criterea.source.increment()) break;
		if (criterea.dir == Backward && criterea.source.decrement()) break;
	}
	
	return true;
}
