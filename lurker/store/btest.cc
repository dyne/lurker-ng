#include <set>
#include "btree.h"

#define SIZE	100

typedef short rptr;
struct Record
{
	int	key;
	
	rptr	left;
	rptr	right;
	char	skew;
};

rptr	root;
rptr	rfill;
Record	tbl[SIZE];

int compare(long a, long b) { return a - b; }

LU_BTREE_DEFINE(foo, rptr, SIZE, Record, tbl, compare)

int main()
{
	set<int> foo;
	rfill = 0;
	root = SIZE;
	
	/* Fill them up */
	while (foo.size() != SIZE)
	{
		int x = random();
		if (foo.find(x) != foo.end()) continue;
		
		tbl[rfill].key = x;
		root = my_btree_foo_insert(root, rfill++);
		assert (root != SIZE);
		
		foo.insert(x);
	}
	
	/* Pick records at random to delete, then add new ones */
	while (1)
	{
		int x = random() % SIZE;
		set<int>::iterator i = foo.begin();
		while (x--) i++;
		
		x = *i;
		foo.erase(i);
		
		rptr w = root;
		while (w != SIZE)
		{
			if      (x < tbl[w].key)	w = tbl[w].left;
			else if (x > tbl[w].key)	w = tbl[w].right;
			else break;
		}
		
		assert (w != SIZE);
		
		root = my_btree_foo_remove(root, w);
		assert (root != SIZE);
		
		
		while (1)
		{
			x = random();
			if (foo.find(x) != foo.end()) continue;
			
			tbl[w].key = x;
			root = my_btree_foo_insert(root, w);
			
			foo.insert(x);
			break;
		}
	}
	
	return 0;
}
