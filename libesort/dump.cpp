/*  $Id: dump.cpp,v 1.1.1.1 2003-08-15 13:59:07 terpstra Exp $
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
	
	auto_ptr<Reader> r(Reader::opendb(argv[1]));
	if (!r.get())
	{
		perror("Reader::opendb");
		return 1;
	}
	
	auto_ptr<Walker> w(r->seek("", Forward));
	
	while (w->advance() != -1)
		cout << w->key << "\n";
	
	if (errno != 0)
	{
		perror("Walker::advance");
		return 1;
	}
	
	return 0;
}
