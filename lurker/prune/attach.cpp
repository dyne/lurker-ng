/*  $Id: attach.cpp,v 1.8 2006-02-19 01:17:22 terpstra Exp $
 *  
 *  attach.cpp - Cleanup after an attach/ command
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
#include <cstdlib>

using namespace std;

bool PTable::test_attach(KSI ks)
{
	/* id@YYYYMMDD.HHMMSS.hashcode.* */
	const string::size_type skip = sizeof("attach"); // null is /
	
	string::size_type o = ks->first.find('@', skip);
	
	return	o != string::npos &&		// there is an @ sign
		atol(ks->first.c_str()+skip) &&	// a number precedes it
		MessageId::is_full(ks->first.c_str()+o+1); // valid message id
}

void PTable::calc_attach(KSI ks)
{
	/* Attachment contents never change
	 *
	 * Policy:
	 *   kill after a bounded lifetime
	 *   kill after a period of no accesses
	 */
	
	if (!test_attach(ks))
	{
		if (verbose)
			cout << ks->first << ": not a lurker file." << endl;
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
	
	if (verbose)
		cout << ks->first << ": not expired" << endl;
}
