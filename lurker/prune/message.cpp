/*  $Id$
 *  
 *  message.cpp - Cleanup after a message/ command
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

#define _FILE_OFFSET_BITS 64

#include "PTable.h"
#include <Keys.h>

#include <iostream>

using namespace std;

bool PTable::test_message(KSI ks)
{
	const string::size_type skip = sizeof("message"); // null is /
	return MessageId::is_full(ks->first.c_str() + skip);
}

void PTable::calc_message(KSI ks)
{
	/* Messages themselves never change
	 *
	 * ... but messages include:
	 *   list info (from config file)
	 *   thread info
	 *   next/prev for each mbox info
	 *
	 * Policy:
	 *   kill if obsolete due to above
	 *   kill if older than a fixed time
	 *   kill if no recent accesses
	 */
	
	if (!test_message(ks))
	{
		if (verbose)
			cout << ks->first << ": not a lurker file." << endl;
		return;
	}
	
	MessageId id(ks->first.c_str() + 8);
	if (ks->second.mtime <= cfg.modified)
	{	// die - it's older than the config file
		ks->second.kill = true;
		if (verbose)
			cout << ks->first << ": older than config file." << endl;
		return;
	}
	
	if (now - ks->second.mtime >= modifiedLimit)
	{
		ks->second.kill = true;
		if (verbose)
			cout << ks->first << ": expired due to maximum age." << endl;
		return;
	}
	
	if (now - ks->second.atime >= accessedLimit)
	{
		ks->second.kill = true;
		if (verbose)
			cout << ks->first << ": expired due to no access." << endl;
		return;
	}
	
	Summary& sum = summaries[id];
	string tid(subject_hash(sum.subject.c_str()));
	if (threads.find(tid) != threads.end())
	{	// die - the thread changed
		ks->second.kill = true;
		if (verbose)
			cout << ks->first << ": thread modified." << endl;
		return;
	}
	
	set<string>::const_iterator list;
	for (list = sum.lists.begin(); list != sum.lists.end(); ++list)
	{
		const MessageIds& ids = lists[*list];
		MessageIds::const_iterator self = ids.find(id);
		if (self == ids.end())
		{
			// if it can't find us, then we are not in the range
			// of messages whose mindex is affected by import
			continue;
		}
		
		MessageIds::const_iterator next = self; ++next;
		MessageIds::const_iterator prev = self; --prev;
		
		Summaries::const_iterator ns, ps;
		
		if (prev != ids.end() && 
		   (ps = summaries.find(*prev)) != summaries.end() &&
		   ps->second.changed)
		{
			ks->second.kill = true;
			if (verbose)
				cout << ks->first << ": previous message changed in '" << *list << "'." << endl;
			return;
		}
		
		if (next != ids.end() && 
		   (ns = summaries.find(*next)) != summaries.end() &&
		   ns->second.changed)
		{
			ks->second.kill = true;
			if (verbose)
				cout << ks->first << ": next message changed in '" << *list << "'." << endl;
			return;
		}
	}
	
	if (verbose)
		cout << ks->first << ": not expired" << endl;
}
