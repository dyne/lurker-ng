/*  $Id: commands.h,v 1.3 2003-06-09 02:31:48 terpstra Exp $
 *  
 *  commands.h - All the commands we support
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
 
#ifndef COMMANDS_H
#define COMMANDS_H

#include <esort.h>
#include <Config.h>
#include <vector>

#define _(x) x

int handle_message(const Config& c, ESort::Reader* r, const string& param);
int handle_thread (const Config& c, ESort::Reader* r, const string& param);
int handle_mindex (const Config& c, ESort::Reader* r, const string& param);
int handle_splash (const Config& c, ESort::Reader* r, const string& param);
int handle_search (const Config& c, ESort::Reader* r, const string& param);
int handle_attach (const Config& c, ESort::Reader* r, const string& param);
int handle_mbox   (const Config& c, ESort::Reader* r, const string& param);
int handle_list   (const Config& c, ESort::Reader* r, const string& param);

string redirect(const string& url);
string error(
	const string& main, 
	const string& sub, 
	const string& suggest);

using std::vector;

void tokenize(
	const string& str,
	vector<string>& tokens,
	const string& delimiters = " ");

#endif
