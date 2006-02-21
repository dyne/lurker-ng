/*  $Id: ConfigFile.h,v 1.12 2006-02-21 15:46:21 terpstra Exp $
 *  
 *  ConfigFile.h - Knows how to load the config file
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

#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <map>
#include <set>
#include <iostream>
#include <vector>

#if __GNUC__ == 2
#include <strstream>
#else
#include <sstream>
#endif

// get VERSION
#include "../config.h"

using std::string;
using std::map;
using std::set;
using std::ostream;
using std::vector;

// localized string
class lstring
{
 protected:
 	map<string, string> s;
 	static map<string, string>* c; // conversion from one spelling to common
 	
 	static void prep_c();
 	
 public:
 	lstring(const string& fb);
 	lstring() { }
 	
 	// If the locale is funky, returns false
 	bool translate(const string& locale, const string& translation);
 	
 	string localize(const string& locale) const;
 	string operator () (const string& locale) const
 	{ return localize(locale); }
 	
 	bool is_set() const;
 	
 	// normalize the language to a common spelling - false if broken
 	static bool lang_normalize(string& lang);
 	static bool locale_normalize(string& locale);
};

struct List
{
	// filenames and addresses are not localized, nor are identifiers
	string mbox;
	string address;
	string group;
	set<string> languages;
	
	bool offline;
	bool allowed; // Set to true if the config file enabled this list
	
	// localize these
	lstring title;		// could make sense in some cases 
	lstring description;	// clearly should be translated
	lstring link;		// different language on different pages
	
 	struct SerializeMagic
 	{
 		const List& m;
 		string l;
 		SerializeMagic(const List& m_, const string& l_)
 		: m(m_), l(l_) { }
 	};
 	SerializeMagic operator () (const string& locale) const
 	{ return SerializeMagic(*this, locale); }
};

struct Frontend
{
 	enum Perm { ALLOW, DENY };
 	enum What { GROUP, LIST };
 	
 	struct Entry
	{
		Perm perm;
		What what;
		string key;
 	};
 	
 	vector<Entry> entries;
};

class Config
{
 private:
 	List* list;
 	Frontend* frontend;
 	
 	string group;
#if __GNUC__ == 2
	strstream error;
#else
	std::stringstream error;
#endif
 	
 public:
 	typedef map<string, List> Lists;
 	typedef set<string>       Members;
 	
 	struct GroupData
 	{
 		lstring heading;
 		Members members;
 	};
 	
 	typedef map<string, GroupData> Groups;
 	typedef map<string, Frontend> Frontends;
 	
 	Lists lists;
 	Groups groups;
 	Frontends frontends;
        
 	// never localize paths, commands, or addresses
 	string	dbdir;
 	string	xslt;
 	string	pgpv_mime;
 	string	pgpv_inline;
 	string	admin_address;
 	
 	// some names are spelt differently by locales
 	
 	lstring	archive;
 	lstring	admin_name;
 	
 	bool	web_cache;
 	bool	hide_email;
 	bool	raw_email;
 	time_t	modified; // the timestamp of modification for the config file
 	
 	// parameters specific for rendering
 	string	docUrl;
 	string	cgiUrl;
 	string	command;
 	mutable string options;
 	
 	// get the error string
 	string getError()
 	{
#if __GNUC__ == 2
		string out(error.str(), error.rdbuf()->pcount());
		return out;
#else
		return error.str();
#endif
 	}
 	
 	Config();
 	
 	// Open the config from this file.
 	int load(const string& file, bool toplevel = true);
 	int process_command(const string& key, const string& val, const string& dir);
 	
 	// Set the allowed flag on lists
 	void set_permissions(const Frontend& f);
 	
 	struct SerializeMagic
 	{
 		const Config& c;
 		string l;
 		SerializeMagic(const Config& c_, const string& l_)
 		: c(c_), l(l_) { }
 	};
 	SerializeMagic operator () (const string& locale) const
 	{ return SerializeMagic(*this, locale); }
};

ostream& operator << (ostream& o, const List::SerializeMagic& lm);
ostream& operator << (ostream& o, const Config::SerializeMagic& cm);

#endif
