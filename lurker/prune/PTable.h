/*  $Id: PTable.h,v 1.1 2003-05-12 00:55:24 terpstra Exp $
 *  
 *  PTable.cpp - Prune table records state for pruning
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

#ifndef PTABLE_H
#define PTABLE_H

#include <string>
#include <set>
#include <map>

#include <ctime>

#include <MessageId.h>
#include <esort.h>

using std::set;
using std::map;
using std::string;

class PTable
{
 protected:
	struct KillState
	{
		string	file;
		time_t	mtime;
		time_t	atime;
		bool	kill;
		
		bool operator < (const KillState& o) const
		{ return file < o.file; }
	};
	
	struct Summary
	{
		string		subject;
		set<string>	lists;
	};
	
	ESort::Reader*		reader;
	time_t			config;
	time_t			stamp;
	bool			verbose;
	
	set<MessageId>		newIds;
	set<KillState>		state;
	map<MessageId, Summary>	summaries;
	
	set<string>	readdir(const string& dir);
	KillState	stat(const string& file);
	
	string prep_message();
	string prep_thread ();
	string prep_mindex ();
	string prep_splash ();
	string prep_search ();
	string prep_attach ();
	string prep_mbox   ();
	
	string calc_message();
	string calc_thread ();
	string calc_mindex ();
	string calc_splash ();
	string calc_search ();
	string calc_attach ();
	string calc_mbox   ();
	
 public:
 	PTable(ESort::Reader* reader, time_t config, time_t stamp, bool verbose);
	
	string pull();	// get the new ids
	string prep();	// get all data for each module
	string load();	// pull all summaries off disk
	string calc();	// decide what to do with cache
	string kill();	// prune any cache we don't like
};


#endif
