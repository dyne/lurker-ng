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
