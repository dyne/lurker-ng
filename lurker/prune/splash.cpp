/*  $Id: splash.cpp,v 1.5 2003-06-23 14:38:42 terpstra Exp $
 *  
 *  splash.cpp - Cleanup after a splash/ command
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

using namespace std;

void PTable::calc_splash(KSI ks)
{
	/* Splash pages depend solely on the config file
	 *
	 * Policy:
	 *   kill if obsolete due to config file
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
	
	if (verbose)
		cout << ks->first << ": not expired" << endl;
}
