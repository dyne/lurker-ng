/*  $Id: test-avl.cc,v 1.3 2002-07-19 14:40:34 terpstra Exp $
 *  
 *  test-avl.cc - Test insert and delete operations of the avl tree
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

#include "avl.h"

#include <set>
#include <assert.h>
#include <time.h>
#include <stdlib.h>

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

AVL_DEFINE_INSERT(foo, rptr, SIZE, Record, compare)
AVL_DEFINE_REMOVE(foo, rptr, SIZE, Record, compare)

int main()
{
	long j;
	
	set<int> foo;
	rfill = 0;
	root = SIZE;
	
	srandom(time(0));
	
	/* Fill them up */
	while (foo.size() != SIZE)
	{
		int x = random();
		if (foo.find(x) != foo.end()) continue;
		
		tbl[rfill].key = x;
		root = my_avl_foo_insert(tbl, root, rfill++);
		assert (root != SIZE);
		
		foo.insert(x);
	}
	
	/* Pick records at random to delete, then add new ones */
	for (j = 0; j < 1000000; j++)
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
		
		root = my_avl_foo_remove(tbl, root, w);
		assert (root != SIZE);
		
		while (1)
		{
			x = random();
			if (foo.find(x) != foo.end()) continue;
			
			tbl[w].key = x;
			root = my_avl_foo_insert(tbl, root, w);
			
			foo.insert(x);
			break;
		}
	}
	
	return 0;
}
