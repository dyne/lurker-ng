/*  $Id: jump.cpp,v 1.2 2003-04-21 18:26:20 terpstra Exp $
 *  
 *  jump.cpp - Jump to a given date offset
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

#include "parse.h"

#include <ctime>
#include <cstring>

int main()
{
	map<string, string> args = getParams();
	
	struct tm tms;
	memset(&tms, 0, sizeof(tms));
	
	tms.tm_sec  = atol(args["sec" ].c_str());
	tms.tm_min  = atol(args["min" ].c_str());
	tms.tm_hour = atol(args["hour"].c_str());
	tms.tm_mday = atol(args["mday"].c_str());
	tms.tm_mon  = atol(args["mon" ].c_str()) - 1;
	tms.tm_year = atol(args["year"].c_str()) - 1900;
	
	char buf[26];
	strftime(buf, 25, "%Y%m%d.%H%M%S", &tms);
	
	return redirectUrl(
		"mindex/" + 
		args["list"] + 
		"@" +
		buf +
		".00000000." +
		args["format"]);
}