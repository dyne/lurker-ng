/*  $Id: search.cpp,v 1.1 2003-05-14 10:36:13 terpstra Exp $
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

// cache a search for only 2 hours
#define EXPIRE_TIME_CREATION	60*60*2

using namespace std;

void PTable::calc_search(KSI ks)
{
	/* search contents are very hard to predict
	 *
	 * Policy:
	 *   kill it after a short expiry
	 */
	
	if (now - ks->second.mtime >= EXPIRE_TIME_CREATION)
	{
		ks->second.kill = true;
		if (verbose)
			cout << ks->first << ": expired due to maximum age." << endl;
		return;
	}
}
