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
	
	return 0;
}
