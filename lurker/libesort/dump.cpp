/*  $Id: dump.cpp,v 1.2 2003-04-21 18:25:33 terpstra Exp $
 *  
 *  dump.cpp - Dump the contents of a libesort database to cout
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
  
#include "esort.h"

#include <iostream>
#include <memory>
#include <cstdio>
#include <cerrno>

using namespace std;
using namespace ESort;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		cerr << "Syntax: " << argv[0] << " <database>\n" << endl;
		return 1;
	}
	
	auto_ptr<Reader> r(Reader::open(argv[1]));
	if (!r.get())
	{
		perror("Reader::open");
		return 1;
	}
	
	auto_ptr<Walker> w(r->seek(""));
	if (!w.get())
	{
		if (errno == 0) return 0; // empty
		perror("Reader::seek");
		return 1;
	}
	
	do cout << w->key << "\n";
	while (w->advance() != -1);
	
	if (errno != 0)
	{
		perror("Walker::advance");
		return 1;
	}
	
	return 0;
}
