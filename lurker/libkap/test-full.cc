#include "kap.h"

#include <unistd.h>
#include <map>
#include <assert.h>
#include <string>
#include <stdlib.h>
#include <time.h>
#include <errno.h>
#include <vector>

int main()
{
	int out, c;
	Kap k;
	
	map<string, vector<int> > tbl;
	
	out = kap_create(&k, KAP_FAST);
	assert (out == 0);
	
	out = kap_open(k, ".", "test-full");
	assert (out == 0);
	
	string key;
	c = 0;
	while (cin >> key)
	{
		c++;
		cout << c << " " << key << endl;
		
		int i = random();
		tbl[key].push_back(i);
		kap_append(k, key.c_str(), &i, 1);
	}
	
	map<string, vector<int> >::iterator i;
	for (i = tbl.begin(); i != tbl.end(); i++)
	{
		KRecord	kr;
		
		out = kap_kopen(k, &kr, i->first.c_str());
		assert (out == 0);
		
		vector<int>::size_type j;
		for (j = 0; j < i->second.size(); j++)
		{
			int got;
			out = kap_kread(k, &kr, i->first.c_str(), 
				j, &got, 1);
			assert (out == 0);
			
			assert (got == i->second[j]);
		}
		
		out = kap_kclose(k, &kr, i->first.c_str());
		assert (out == 0);
	}
	
	return 0;
}
