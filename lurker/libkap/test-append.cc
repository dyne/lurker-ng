/*  $Id: test-append.cc,v 1.4 2002-07-09 22:42:45 terpstra Exp $
 *  
 *  test-append.cc - Test append fragmentated read and write operations
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
#include <stdio.h>
#include <map>
#include <assert.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <vector>

vector<KRecord>		recs;
vector<vector<int> >	ok;

#define SIZE	1000

int main()
{
	int buf[SIZE];
	int out;
	int i, j, m, n;
	Kap k;
	
	out = kap_create(&k, KAP_APPEND);
	assert (out == 0);
	
	out = kap_open(k, "nonexist", "foo");
	assert (out == ENOENT);
	
	unlink("test-append.append");
	
	out = kap_open(k, ".", "test-append");
	assert (out == 0);
	
	for (i = 0; i < 1000; i++)
	{
		ok.push_back(vector<int>());
		recs.push_back(KRecord());
		
		memset(&recs.back(), 0, sizeof(KRecord));
		// printf("%i\n", i);
		for (j = 0; j < SIZE; j += m)
		{
			m = random() % 100;
			if (m+j > SIZE)
				m = SIZE - j;
			
			for (n = 0; n < m; n++)
			{
				buf[n] = random();
				ok.back().push_back(buf[n]);
			}
			
			kap_append_append(k, &recs.back(), 
				&buf[0], m);
		}
	}
	
	for (i = 0; i < 1000; i++)
	{
		for (j = 0; j < SIZE; j += m)
		{
			m = random() % 100;
			if (m+j > SIZE)
				m = SIZE-j;
			
			kap_append_read(k, &recs[i], j, &buf[j], m);
			
			for (n = j; n < j+m; n++)
			{
				assert (buf[n] == ok[i][n]);
			}
		}
	}
	
	return 0;
}
