/*  $Id: PTable.cpp,v 1.15 2003-06-23 14:38:42 terpstra Exp $
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

#include "PTable.h"
#include <Keys.h>

#include <iostream>

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// If the stuff imported spans more than a week, kill all cache
#define MASSACRE_THRESHOLD	7*24*60*60

using namespace std;

PTable::PTable(ESort::Reader* reader_, time_t config_, time_t stamp_, 
               bool verbose_, time_t modifiedLimit_, time_t accessedLimit_)
 : reader(reader_), config(config_), stamp(stamp_), now(time(0)), 
   verbose(verbose_), modifiedLimit(modifiedLimit_), accessedLimit(accessedLimit_)
{
}

string PTable::loadDir(const string& dir, bool yank)
{
	if (verbose) cout << "Loading directory " << dir << endl;
	DIR* d = opendir(dir.c_str());
	if (d)
	{
		struct dirent* de;
		while ((de = ::readdir(d)) != 0)
		{
			string name = dir + "/" + de->d_name;
			struct stat sbuf;
			if (stat(name.c_str(), &sbuf) >= 0)
			{
				if (!S_ISREG(sbuf.st_mode))
					continue;
				
				KillState ks;
				ks.mtime = sbuf.st_mtime;
				ks.atime = sbuf.st_atime;
				ks.kill  = false;
				
				state[name] = ks;
				if (yank)
				{
					MessageId id(de->d_name);
					if (id.timestamp() != 0)
						summaries[id].changed = false;
				}
			}
		}
		
		closedir(d);
	}
	
	return "";
}

string PTable::loadNewIds()
{
	string pfx = LU_CACHE;
	MessageId offid(stamp);
	
	if (verbose) cout << "Loading new ids since last run ..." << endl;
	
	auto_ptr<ESort::Walker> walker(
		reader->seek(pfx, offid.raw().substr(0, 4), ESort::Forward));
	while (walker->advance() != -1)
	{
		if (walker->key.length() != 13)
			return string("corrupt import stamp");
			
		MessageId id(walker->key.c_str()+5, 8);
		newIds.insert(id);
		summaries[id].changed = true;
		
		if (verbose) cout << "New message: " << id.serialize() << endl;
	}
	if (errno != 0)
	{
		return string("Walker::advance:") + strerror(errno);
	}
	
	return "";
}

string PTable::loadSummaries()
{
	if (verbose) cout << "Loading summary data ..." << endl;
	
	int ok;
	string pfx = LU_SUMMARY;
	auto_ptr<ESort::Walker> walker(
		reader->seek(pfx, summaries.begin()->first.raw(), ESort::Forward));
	while ((ok = walker->advance()) != -1)
	{
		if (walker->key.length() < 1 + 8 + 1)
			return "invalid mbox entry -- way too short";
		
		MessageId id(walker->key.c_str() + 1, 8);
		
		if (id > (--summaries.end())->first)
			break; // done pulling
		
		if (summaries.find(id) == summaries.end())
			continue; // don't process this one
		
		// We use this for getting an unsigned value below.
		const unsigned char* k = (const unsigned char*)
			walker->key.c_str()+1+8;
		
		// read all the values
		switch (*k)
		{
		case LU_MESSAGE_AUTHOR_EMAIL:
		case LU_MESSAGE_AUTHOR_NAME:
			// noop
			break;
			
		case LU_MESSAGE_SUBJECT:
			summaries[id].subject =
				walker->key.substr(1+8+1, string::npos);
			break;
			
		case LU_MESSAGE_MBOX:
			if (walker->key.length() < 1+8+1+1+12)
				return "invalid mbox entry -- too short";
			
			//!!! could be more careful about corrupt dbs here
			summaries[id].lists.insert((const char*)(k+1));
			lists[(const char*)(k+1)]; // poke the list table
			break;
		
		default:
			return "unknown mbox summary control code";
		}
	}
	
	if (ok == -1 && errno != 0)
		return string("Walker::advance:") + strerror(errno);
	
	return "";
}

string PTable::loadThreads()
{
	Summaries::const_iterator i;
	for (i = summaries.begin(); i != summaries.end(); ++i)
	{
		if (!i->second.changed) continue;
		string tid(subject_hash(i->second.subject.c_str()));
		threads.insert(tid);
	}
	
	return "";
}

string PTable::loadLists()
{
	MessageId oldest(*newIds.begin());
	MessageId newest(*--newIds.end());
	
	// The goal is to get all the message ids within a list for
	// the last imported message - 40 to the newest imported message +40.
	// We load extra messages (over 36) so outliers can be detected.
	for (Lists::iterator list = lists.begin(); list != lists.end(); ++list)
	{
		if (verbose) cout << "Loading list data: " << list->first << endl;
		
		string pfx = 
			string(LU_KEYWORD LU_KEYWORD_LIST)
			+ list->first
			+ '\0';
			
		auto_ptr<ESort::Walker> forward(
			reader->seek(pfx, oldest.raw(), ESort::Forward));
		auto_ptr<ESort::Walker> backward(
			reader->seek(pfx, oldest.raw(), ESort::Backward));
		
		// first walk backwards a few steps
		for (int backCount = 0; backCount < 40; ++backCount)
		{
			if (backward->advance() == -1) break;
			if (backward->key.length() != pfx.length() + 8)
				return "corrupt keyword entry";
			MessageId id(backward->key.c_str() + pfx.length(), 8);
			list->second.insert(id);
		}
		if (errno != 0) return string("Walker::advance:") + strerror(errno);
		
		// walk forward till we pass newest
		int ok;
		while ((ok = forward->advance()) != -1)
		{
			if (forward->key.length() != pfx.length() + 8)
				return "corrupt keyword entry";
			MessageId id(forward->key.c_str() + pfx.length(), 8);
			list->second.insert(id);
			if (id >= newest) break;
		}
		if (ok == -1 && errno != 0) return string("Walker::advance:") + strerror(errno);
		if (ok == -1) continue;
		
		for (int forCount = 0; forCount < 40; ++forCount)
		{
			if (forward->advance() == -1) break;
			if (forward->key.length() != pfx.length() + 8)
				return "corrupt keyword entry";
			MessageId id(forward->key.c_str() + pfx.length(), 8);
			list->second.insert(id);
		}
	}
	
	return "";
}

string PTable::load()
{
	loadDir("attach",  false);
	loadDir("list",    false);
	loadDir("mbox",    false);
	loadDir("message", true);
	loadDir("mindex",  false);
	loadDir("search",  false);
	loadDir("splash",  false);
	loadDir("thread",  true);
	
	loadNewIds();
	
	if (newIds.empty()) return "";
	time_t threshold = 
		(--newIds.end())->timestamp() -
		newIds.begin()->timestamp();
	if (threshold > MASSACRE_THRESHOLD)
		return "";
	
	loadSummaries();
	loadThreads();
	loadLists();
	
	return "";
}

string PTable::calc()
{
	time_t threshold = 0;
	if (!newIds.empty())
	{
		threshold = 
			(--newIds.end())->timestamp() -
			newIds.begin()->timestamp();
	}
	
	if (threshold > MASSACRE_THRESHOLD)
	{
		if (verbose)
			cout << "New messages span too large a window; entering massacre mode." << endl;
		
		for (KSI i = state.begin(); i != state.end(); ++i)
			i->second.kill = true;
		
		return "";
	}
	else
	{
		for (KSI i = state.begin(); i != state.end(); ++i)
		{
			if (i->first.substr(0, 6) == "attach" ) calc_attach (i);
			if (i->first.substr(0, 4) == "mbox"   ) calc_mbox   (i);
			if (i->first.substr(0, 4) == "list"   ) calc_list   (i);
			if (i->first.substr(0, 7) == "message") calc_message(i);
			if (i->first.substr(0, 6) == "mindex" ) calc_mindex (i);
			if (i->first.substr(0, 6) == "search" ) calc_search (i);
			if (i->first.substr(0, 6) == "splash" ) calc_splash (i);
			if (i->first.substr(0, 6) == "thread" ) calc_thread (i);
		}
	}
	
	return "";
}

string PTable::kill()
{
	State::const_iterator i;
	for (i = state.begin(); i != state.end(); ++i)
	{
		if (i->second.kill)
		{
			if (verbose) cout << "Deleting: " << i->first << endl;
			if (unlink(i->first.c_str()) < 0)
			{
				cerr << "Cannot unlink: " << i->first << ": " << strerror(errno) << endl;
				// continue
			}
		}
	}
	
	return "";
}
