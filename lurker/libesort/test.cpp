/*  $Id: test.cpp,v 1.7 2003-04-25 14:55:43 terpstra Exp $
 *  
 *  test.cpp - Write lots of crap and compare with RAM
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
#include <set>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cassert>

// #define DEBUG 1
#define DEBUG 0

using namespace std;
using namespace ESort;

multiset<string> ram;
auto_ptr<Writer> db;

string makeStr()
{	// for testing with a human
	
	int len = random() % 20;
	int i;
	
	string out;
	for (i = 0; i < len; ++i)
		out += char('A' + (random() % ('Z' - 'A' + 1)));
	
	return out;
}

string makeBin()
{	// for testing without a human
	int len = random() % 256;
	int i;
	
	string out;
	for (i = 0; i < len; ++i)
		out += char(random() % 256);
	
	return out;
}

void check_forward(const string& k)
{
	if (DEBUG) cout << "Forward check: " << k << endl;
	
	multiset<string>::iterator i;
	int eo;
	
	auto_ptr<Walker> walk(db->seek(k, Forward));
	assert (walk.get()); // always succeeds
	
	for (eo = walk->advance(), i = ram.lower_bound(k);
	     eo != -1 && i != ram.end();
	     eo = walk->advance(), ++i)
	{
		if (DEBUG) cout << " Vals: " << walk->key << " - " << *i << endl;
		
		if (walk->key != *i)
		{
			cerr << "Wrong forward keys: " << walk->key << " != " << *i << endl;
			exit(1);
		}
	}
	
	if (eo == -1 && i != ram.end())
	{
		cerr << "Ram has: " << *i << endl;
		perror("db forward underrun vs ram");
		exit(1);
	}
	
	if (eo == 0 && i == ram.end())
	{
		cerr << "Db has: " << walk->key << endl;
		cerr << "db forward overrun vs ram" << endl;
		exit(1);
	}
}

void check_backward(const string& k)
{
	if (DEBUG) cout << "Backard check: " << k << endl;
	
	multiset<string>::iterator i;
	int eo;
	
	auto_ptr<Walker> walk(db->seek(k, Backward));
	assert (walk.get()); // always succeeds
	
	for (eo = walk->advance(), i = ram.lower_bound(k);
	     eo != -1 && i != ram.begin();
	     eo = walk->advance())
	{
		--i;
		
		if (DEBUG) cout << " Vals: " << walk->key << " - " << *i << endl;
		
		if (walk->key != *i)
		{
			cerr << "Wrong backward keys: " << walk->key << " != " << *i << endl;
			exit(1);
		}
	}
	
	if (eo == -1 && i != ram.begin())
	{
		cerr << "Ram has: " << *--i << endl;
		perror("db backward underrun vs ram");
		exit(1);
	}
	
	if (eo == 0 && i == ram.end())
	{
		cerr << "Db has: " << walk->key << endl;
		cerr << "db backward overrun" << endl;
		exit(1);
	}
}

void check(const string& k)
{
	check_forward(k);
	check_backward(k);
}

void check()
{
	check("");
	check("ZZZZZZZZ");
	check(makeStr()); // ??? change me here for bin
	check(makeStr()); // ??? change me here for bin
	check(makeStr()); // ??? change me here for bin
}

void insert(const string& k)
{
	if (DEBUG) cout << "Insert: " << k << endl;
	
	int o = db->insert(k);
	if (o == -1)
	{
		perror("Writer::insert");
		exit(1);
	}
	
	ram.insert(k);
}

int main(int argc, char** argv)
{
	time_t seed;
	
	if (argc == 2)
		seed = atol(argv[1]);
	else	seed = time(0);
	
	cout << "For replay of this run, seed: " << seed << endl;
	srandom(seed);
	
	db = Writer::open("turnip", Parameters(1, 8192, 255, false));
	if (!db.get())
	{
		perror("Writer::open");
		return 1;
	}
	
	for (int i = 0; i < 1000; ++i)
	{
		check();
		
		int num = random() % 500; // up to 500 inserts
		for (int j = 0; j < num; ++j)
		{
			string x = makeStr();
			insert(x);
			check(x);
		}
		
		check();
		
		cout << "Committing" << endl;
		
		int c = db->commit();
		if (c != 0)
		{
			perror("Writer::commit");
			return 1;
		}
	}
	
	return 0;
}
