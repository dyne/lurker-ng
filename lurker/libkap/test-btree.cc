/*  $Id: test-btree.cc,v 1.7 2002-07-09 22:42:45 terpstra Exp $
 *  
 *  test-btree.cc - Test btree insert and read consistency
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: LGPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation;
 *    version 2 of the License.
 *    
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *    
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

#include "kap.h"

#include <unistd.h>
#include <map>
#include <assert.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

void test_write(Kap k, const char* key, string val, int exist)
{
	int out;
//	cout << "TEST WRITE: " << key << endl;
	
	out = kap_btree_write(k, key, 
		(const unsigned char*)val.c_str(), val.length());
	
	assert (exist || out == 0);
	assert (!exist || out == KAP_KEY_EXIST);
}

void test_read(Kap k, const char* key, string val, int exist)
{
	int out;
	unsigned char buf[1000];
	ssize_t len;
	
//	cout << "TEST READ: " << key << endl;
	
	out = kap_btree_read(k, key, &buf[0], &len);
	assert (exist || out == KAP_NOT_FOUND);
	if (!exist) return;
	
	assert (len == (ssize_t)val.length());
	assert (memcmp(val.c_str(), &buf[0], val.length()) == 0);
}

int main()
{
	int out;
	Kap k;
	map<string, string> tbl;
	
	out = kap_create(&k, KAP_BTREE);
	assert (out == 0);
	
	out = kap_open(k, "nonexist", "foo");
	assert (out == ENOENT);
	
	unlink("test-btree.btree");
	
	out = kap_btree_set_sectorsize(k, 512);
	assert (out == ERANGE);
	
	out = kap_btree_set_sectorsize(k, 2048);
	assert (out == 0);
	
	out = kap_open(k, ".", "test-btree");
	assert (out == 0);
	
	test_read(k, "snoopy", "xxx", 0);
	test_read(k, "peanut", "xxx", 0);
	
	test_write(k, "foo", "bar", 0);
	tbl["foo"] = "bar";
	test_read(k, "snoopy", "xxx", 0);
	test_read(k, "foo", "bar", 1);
	
	test_write(k, "foo", "sdgf", 1);
	test_write(k, "joe", "bob", 0);
	tbl["joe"] = "bob";
	test_write(k, "jan", "help", 0);
	tbl["jan"] = "help";
	
	test_read(k, "joe", "bob", 1);
	
	srandom(time(0));
	for (int i = 0; i < 50000; i++)
	{
		char buf[7];
		string val = "       ";
		
		buf[0] = 'a' + random() % 26;
		buf[1] = 'a' + random() % 26;
		buf[2] = 'a' + random() % 26;
		buf[3] = 'a' + random() % 26;
		buf[4] = 'a' + random() % 26;
		buf[5] = 'a' + random() % 26;
		buf[6] = 0;
		
		val[0] = random() % 256;
		val[1] = random() % 256;
		val[2] = random() % 256;
		val[3] = random() % 256;
		val[4] = random() % 256;
		val[5] = random() % 256;
		val[6] = random() % 256;
		
		test_write(k, buf, val, tbl.find(buf) != tbl.end());
		if (tbl.find(buf) == tbl.end())
			tbl[buf] = val;
	}
	
	map<string, string>::iterator i;
	for (i = tbl.begin(); i != tbl.end(); i++)
		test_read(k, i->first.c_str(), i->second, 1);
	
	test_read(k, "peas", "xxx", 0);
	test_read(k, "jjjs", "xxx", 0);
	test_read(k, "zzda", "xxx", 0);
	test_read(k, "qqwr", "xxx", 0);
	test_read(k, "uasf", "xxx", 0);
	
	return 0;
}
