/*  $Id: search.cpp,v 1.6 2004-08-19 14:52:29 terpstra Exp $
 *  
 *  mbox.cpp - Cleanup after an mbox/ command
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

// cache a search for only 2 hours
#define EXPIRE_TIME_CREATION	60*60*2

using namespace std;

bool PTable::test_search(KSI ks)
{
	/* format: id@keywords.* */
	const string::size_type skip = sizeof("search"); // null is /
	
	string::size_type o = ks->first.find('@', skip);
	return	o != string::npos &&
		o == MessageId::full_len + skip &&
		MessageId::is_full(ks->first.c_str() + skip);
}

void PTable::calc_search(KSI ks)
{
	/* search contents are very hard to predict
	 *
	 * Policy:
	 *   kill it after a short expiry
	 */
	
	if (!test_search(ks))
	{
		if (verbose)
			cout << ks->first << ": not a lurker file." << endl;
		return;
	}
	
	if (ks->second.mtime <= cfg.modified)
	{	// die - it's older than the config file
		ks->second.kill = true;
		if (verbose)
			cout << ks->first << ": older than config file." << endl;
		return;
	}
	
	if (now - ks->second.mtime >= EXPIRE_TIME_CREATION)
	{
		ks->second.kill = true;
		if (verbose)
			cout << ks->first << ": expired due to maximum age." << endl;
		return;
	}
	
	if (verbose)
		cout << ks->first << ": not expired" << endl;
}
