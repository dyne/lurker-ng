/*  $Id: mindex.cpp,v 1.3 2003-05-16 16:22:17 terpstra Exp $
 *  
 *  mindex.cpp - Cleanup after a mindex/ command
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

#include "PTable.h"
#include <Keys.h>

#include <iostream>

using namespace std;

void PTable::calc_mindex(KSI ks)
{
	/* Mindex entries use up to 35 records in either direction of the key
	 *
	 * ... but mindex include:
	 *   list info (from config file)
	 *
	 * Policy:
	 *   kill if obsolete due to above
	 *   kill if older than a fixed time
	 *   kill if no recent accesses
	 */
	
	if (ks->second.mtime <= config)
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
	
	string query(ks->first, 7, string::npos);
	string::size_type at = query.find('@');
	if (at == string::npos)
	{
		ks->second.kill = true; // shouldn't be in here
		if (verbose)
			cout << ks->first << ": not a lurker file." << endl;
		return;
	}
	
	string listn(query, 0, at);
	string ids(query, at+1, string::npos);
	
	MessageId id(ids.c_str());
	if (id.timestamp() == 0)
	{
		ks->second.kill = true; // shouldn't be in here
		if (verbose)
			cout << ks->first << ": not a valid timestamp." << endl;
		return;
	}
	
	if (lists.find(listn) == lists.end())
	{	// this list has not changed if not pulled
		if (verbose)
			cout << ks->first << ": not a modified list." << endl;
		return;
	}
	
	MessageIds& list = lists[listn];
	if (list.empty())
	{
		if (verbose)
			cout << ks->first << ": empty list." << endl;
		return;
	}
	
	MessageIds::const_iterator self = list.lower_bound(id);
	if (self == list.end())
	{	// get off the end
		--self;
	}
	
	int c = 0;
	for (	MessageIds::const_iterator next = self; 
		c < 36 && next != list.end();
		++c, ++next)
	{
		Summaries::const_iterator ns;
		if ((ns = summaries.find(*next)) != summaries.end() &&
		   ns->second.changed)
		{
			ks->second.kill = true;
			if (verbose)
				cout << ks->first << ": succesor messages changed." << endl;
			return;
		}
	}
	
	c = 0;
	for (	MessageIds::const_iterator prev = self; 
		c < 36 && prev != list.end();
		++c, --prev)
	{
		Summaries::const_iterator ps;
		if ((ps = summaries.find(*prev)) != summaries.end() &&
		   ps->second.changed)
		{
			ks->second.kill = true;
			if (verbose)
				cout << ks->first << ": predecessor messages changed." << endl;
			return;
		}
	}
	
	if (verbose)
		cout << ks->first << ": content unmodified." << endl;
}
