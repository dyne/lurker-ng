/*  $Id: PTable.h,v 1.5 2004-08-15 10:54:32 terpstra Exp $
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
#include <ConfigFile.h>
#include <esort.h>

using std::set;
using std::map;
using std::string;

class PTable
{
 protected:
	struct KillState
	{
		time_t	mtime;
		time_t	atime;
		bool	kill;
	};
	
	struct Summary
	{
		string		subject;
		set<string>	lists;
		bool		changed;
	};
	
	typedef set<MessageId>		MessageIds;
	typedef map<string, KillState>	State;
	typedef map<MessageId, Summary>	Summaries;
	typedef set<string>		Threads;
	typedef map<string, MessageIds>	Lists;
	
	typedef map<string, KillState>::iterator KSI;
	
	const Config&		cfg;
	ESort::Reader*		reader;
	time_t			config;
	time_t			stamp;
	time_t			now;
	bool			verbose;
	time_t			modifiedLimit;
	time_t			accessedLimit;
	
	MessageIds	newIds;
	State		state;
	Summaries	summaries;
	Threads		threads;
	Lists		lists;
	
	void calc_list   (KSI i);
	void calc_message(KSI i);
	void calc_thread (KSI i);
	void calc_mindex (KSI i);
	void calc_splash (KSI i);
	void calc_search (KSI i);
	void calc_attach (KSI i);
	void calc_mbox   (KSI i);
	
	string loadNewIds();
	string loadDir(const string& dir, bool yank);
	string loadSummaries();
	string loadThreads();
	string loadLists();
	
 public:
 	PTable(const Config& cfg, ESort::Reader* reader, time_t config, time_t stamp, 
 		bool verbose, time_t modifiedLimit, time_t accessedLimit);
	
	string load();	// pull all summaries off disk
	string calc();	// decide what to do with cache
	string kill();	// prune any cache we don't like
};


#endif
