/*  $Id: profile.cc,v 1.3 2002-07-09 22:42:45 terpstra Exp $
 *  
 *  profile.cc - Test the performance of libkap by indexing text files
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
#include <assert.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <stdio.h>

int main()
{
	int out, c;
	Kap k;
	
	out = kap_create(&k, KAP_FAST);
	assert (out == 0);
	
	unlink("profile.btree");
	unlink("profile.append");
	
	out = kap_open(k, ".", "profile");
	assert (out == 0);
	
	string key;
	c = 0;
	while (cin >> key)
	{
		if (key.length() >= 100)
			key.resize(99);
			
		c++;
		if (c % 1000 == 0)
		{
			printf(".");
			fflush(stdout);
		}
		
		int i = random();
		out = kap_append(k, key.c_str(), &i, 1);
		assert (out == 0);
	}
	
	out = kap_close(k);
	assert (out == 0);
	
	return 0;
}
