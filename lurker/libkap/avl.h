/*  $Id: avl.h,v 1.6 2002-07-19 14:40:34 terpstra Exp $
 *  
 *  avl.h - Manage a AVL search tree
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

/* This is just a collection of macros that can be dumped on top of any type
 * underlying buffer which we are using as a bst. I wish I could use C++ 
 * templates, but I swore an oath! :-)
 *
 * Much assistence in writing this file was derived from:
 *  http://www.purists.org/georg/avltree/
 */

#define AVL_EQUAL	0
#define AVL_LEFT	1
#define AVL_RIGHT	2

#define AVL_ALREADY_THERE	-1
#define AVL_NOT_FOUND	-1
#define AVL_BALANCE	1
#define AVL_OK		0

/* The user of this file should call:
 *   AVL_DEFINE_INSERT(PREFIX, RECTYPE, INVALID, STRUCT, COMPARE)
 *   AVL_DEFINE_REMOVE(PREFIX, RECTYPE, INVALID, STRUCT, COMPARE)
 *
 *   PREFIX  -> an indetifier to prevent symbol conflict
 *   RECTYPE -> the type used to index the table
 *   INVALID -> largest representable size
 *   STRUCT  -> the type of the records maintained as an avl tree
 *	RECTYPE left, right;
 *      sometype key;		<- sometype must be used by COMPARE function
 *      sometype skew;		<- sometype must have >= 2 bits of storage
 *   COMPARE -> a strcmp style compare method
 *
 * This will create the methods: 
 *   RECTYPE my_avl_PREFIX_insert(RECTYPE root, STRUCT* table, RECTYPE recno)
 *     -> you must have already set TABLE[recno].key to an appropriate value.
 *     -> INVALID if already in tree, the new root else (ok)
 *   RECTYPE my_avl_PREFIX_remove(RECTYPE root, STRUCT* table, RECTYPE recno)
 *     -> INVALID if not in tree, the new root else (ok)
 */

#define AVL_DEFINE_INSERT(PREFIX, RECTYPE, INVALID, STRUCT, COMPARE) \
\
\
inline void my_avl_##PREFIX##_rot_left(STRUCT* table, RECTYPE* n) \
{ \
	RECTYPE tmp = *n; \
	\
	*n = table[*n].right; \
	table[tmp].right = table[*n].left; \
	table[*n].left = tmp; \
} \
\
\
inline void my_avl_##PREFIX##_rot_right(STRUCT* table, RECTYPE* n) \
{ \
	RECTYPE tmp = *n; \
	\
	*n = table[*n].left; \
	table[tmp].left = table[*n].right; \
	table[*n].right = tmp; \
} \
\
inline int my_avl_##PREFIX##_right_grown(STRUCT* table, RECTYPE* n) \
{ \
	switch (table[*n].skew) \
	{ \
	case AVL_LEFT: \
		table[*n].skew = AVL_EQUAL; \
		return AVL_OK; \
	\
	case AVL_RIGHT: \
		if (table[table[*n].right].skew == AVL_RIGHT) \
		{ \
			table[*n].skew = table[table[*n].right].skew = \
				AVL_EQUAL; \
			my_avl_##PREFIX##_rot_left(table, n); \
		} \
		else \
		{ \
			if (table[table[*n].right].skew == AVL_RIGHT) \
			{ \
				table[*n].skew = \
					table[table[*n].right].skew = \
						AVL_EQUAL; \
				\
				my_avl_##PREFIX##_rot_left(table, n); \
			} \
			else \
			{ \
				switch (table[table[table[*n].right].left].skew) \
				{ \
				case AVL_RIGHT: \
					table[*n].skew = AVL_LEFT; \
					table[table[*n].right].skew = \
						AVL_EQUAL; \
					break; \
				\
				case AVL_LEFT: \
					table[*n].skew = AVL_EQUAL; \
					table[table[*n].right].skew = \
						AVL_RIGHT; \
					break; \
				\
				case AVL_EQUAL: \
					table[*n].skew = AVL_EQUAL; \
					table[table[*n].right].skew = \
						AVL_EQUAL; \
					break; \
				\
				default: \
					assert(0); \
				} \
				\
				table[table[table[*n].right].left].skew = \
					AVL_EQUAL; \
				my_avl_##PREFIX##_rot_right(table, &table[*n].right); \
				my_avl_##PREFIX##_rot_left(table, n); \
			} \
		} \
		\
		return AVL_OK; \
	\
	case AVL_EQUAL: \
		table[*n].skew = AVL_RIGHT; \
		return AVL_BALANCE; \
	\
	default: \
		assert(0); \
		return AVL_OK; \
	} \
} \
\
\
inline int my_avl_##PREFIX##_left_grown(STRUCT* table, RECTYPE* n) \
{ \
	switch (table[*n].skew) \
	{ \
	case AVL_RIGHT: \
		table[*n].skew = AVL_EQUAL; \
		return AVL_OK; \
	\
	case AVL_LEFT: \
		if (table[table[*n].left].skew == AVL_LEFT) \
		{ \
			table[*n].skew = table[table[*n].left].skew = \
				AVL_EQUAL; \
			my_avl_##PREFIX##_rot_right(table, n); \
		} \
		else \
		{ \
			if (table[table[*n].left].skew == AVL_LEFT) \
			{ \
				table[*n].skew = \
					table[table[*n].left].skew = \
						AVL_EQUAL; \
				\
				my_avl_##PREFIX##_rot_right(table, n); \
			} \
			else \
			{ \
				switch (table[table[table[*n].left].right].skew) \
				{ \
				case AVL_LEFT: \
					table[*n].skew = AVL_RIGHT; \
					table[table[*n].left].skew = \
						AVL_EQUAL; \
					break; \
				\
				case AVL_RIGHT: \
					table[*n].skew = AVL_EQUAL; \
					table[table[*n].left].skew = \
						AVL_LEFT; \
					break; \
				\
				case AVL_EQUAL: \
					table[*n].skew = AVL_EQUAL; \
					table[table[*n].left].skew = \
						AVL_EQUAL; \
					break; \
				\
				default: \
					assert(0); \
				} \
				\
				table[table[table[*n].left].right].skew = \
					AVL_EQUAL; \
				my_avl_##PREFIX##_rot_left(table, &table[*n].left); \
				my_avl_##PREFIX##_rot_right(table, n); \
			} \
		} \
		\
		return AVL_OK; \
	\
	case AVL_EQUAL: \
		table[*n].skew = AVL_LEFT; \
		return AVL_BALANCE; \
	\
	default: \
		assert(0); \
		return AVL_OK; \
	} \
} \
\
\
static int my_avl_##PREFIX##_ins(STRUCT* table, RECTYPE* n, RECTYPE recno) \
{ \
	int tmp, dir; \
	\
	if (*n == INVALID) \
	{ \
		*n = recno; \
		\
		table[recno].left = table[recno].right = INVALID; \
		table[recno].skew = AVL_EQUAL; \
		return AVL_BALANCE; \
	} \
	\
	dir = COMPARE(table[recno].key, table[*n].key); \
	\
	if (dir < 0) \
	{ \
		tmp = my_avl_##PREFIX##_ins(table, &table[*n].left, recno); \
		if (tmp == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_left_grown(table, n); \
		} \
		return tmp; \
	} \
	\
	if (dir > 0) \
	{ \
		tmp = my_avl_##PREFIX##_ins(table, &table[*n].right, recno); \
		if (tmp == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_right_grown(table, n); \
		} \
		\
		return tmp; \
	} \
	\
	return AVL_ALREADY_THERE; \
} \
\
\
static RECTYPE my_avl_##PREFIX##_insert(STRUCT* table, RECTYPE root, RECTYPE recno) \
{ \
	if (my_avl_##PREFIX##_ins(table, &root, recno) == AVL_ALREADY_THERE) \
		return INVALID; \
	\
	return root; \
}





#define AVL_DEFINE_REMOVE(PREFIX, RECTYPE, INVALID, STRUCT, COMPARE) \
\
\
inline int my_avl_##PREFIX##_left_shrunk(STRUCT* table, RECTYPE* n) \
{ \
	switch (table[*n].skew) \
	{ \
	case AVL_LEFT: \
		table[*n].skew = AVL_EQUAL; \
		return AVL_BALANCE; \
		\
	case AVL_EQUAL: \
		table[*n].skew = AVL_RIGHT; \
		return AVL_OK; \
		\
	case AVL_RIGHT: \
		switch (table[table[*n].right].skew) \
		{ \
		case AVL_RIGHT: \
			table[*n].skew = table[table[*n].right].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_left(table, n); \
			return AVL_BALANCE; \
			\
		case AVL_EQUAL: \
			table[*n].skew = AVL_RIGHT; \
			table[table[*n].right].skew = AVL_LEFT; \
			my_avl_##PREFIX##_rot_left(table, n); \
			return AVL_OK; \
			\
		case AVL_LEFT: \
			switch (table[table[table[*n].right].left].skew) \
			{ \
			case AVL_LEFT: \
				table[*n].skew = AVL_EQUAL; \
				table[table[*n].right].skew = AVL_RIGHT; \
				break; \
				\
			case AVL_RIGHT: \
				table[*n].skew = AVL_LEFT; \
				table[table[*n].right].skew = AVL_EQUAL; \
				break; \
				\
			case AVL_EQUAL: \
				table[*n].skew = AVL_EQUAL; \
				table[table[*n].right].skew = AVL_EQUAL; \
				break; \
				\
			default: \
				assert(0); \
			} \
			\
			table[table[table[*n].right].left].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_right(table, &table[*n].right); \
			my_avl_##PREFIX##_rot_left(table, n); \
			return AVL_BALANCE; \
			\
		default: \
			assert(0); \
		} \
		\
	default: \
		assert(0); \
		return -1; /* unreached */ \
	} \
} \
\
\
inline int my_avl_##PREFIX##_right_shrunk(STRUCT* table, RECTYPE* n) \
{ \
	switch (table[*n].skew) \
	{ \
	case AVL_RIGHT: \
		table[*n].skew = AVL_EQUAL; \
		return AVL_BALANCE; \
		\
	case AVL_EQUAL: \
		table[*n].skew = AVL_LEFT; \
		return AVL_OK; \
		\
	case AVL_LEFT: \
		switch (table[table[*n].left].skew) \
		{ \
		case AVL_LEFT: \
			table[*n].skew = table[table[*n].left].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_right(table, n); \
			return AVL_BALANCE; \
			\
		case AVL_EQUAL: \
			table[*n].skew = AVL_LEFT; \
			table[table[*n].left].skew = AVL_RIGHT; \
			my_avl_##PREFIX##_rot_right(table, n); \
			return AVL_OK; \
			\
		case AVL_RIGHT: \
			switch (table[table[table[*n].left].right].skew) \
			{ \
			case AVL_RIGHT: \
				table[*n].skew = AVL_EQUAL; \
				table[table[*n].left].skew = AVL_LEFT; \
				break; \
				\
			case AVL_LEFT: \
				table[*n].skew = AVL_RIGHT; \
				table[table[*n].left].skew = AVL_EQUAL; \
				break; \
				\
			case AVL_EQUAL: \
				table[*n].skew = AVL_EQUAL; \
				table[table[*n].left].skew = AVL_EQUAL; \
				break; \
				\
			default: \
				assert(0); \
			} \
			\
			table[table[table[*n].left].right].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_left(table, &table[*n].left); \
			my_avl_##PREFIX##_rot_right(table, n); \
			return AVL_BALANCE; \
			\
		default: \
			assert(0); \
		} \
		\
	default: \
		assert(0); \
		return -1; /* unreached */ \
	} \
} \
\
static int my_avl_##PREFIX##_kill_largest(STRUCT* table, RECTYPE* n, RECTYPE* w) \
{ \
	int tmp; \
	\
	assert (*n != INVALID); \
	\
	if (table[*n].right != INVALID) \
	{ \
		if ((tmp = my_avl_##PREFIX##_kill_largest( \
			table, &table[*n].right, w)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_right_shrunk(table, n); \
		} \
		return tmp; \
	} \
	\
	*w = *n; \
	*n = table[*n].left; \
	\
	return AVL_BALANCE; \
} \
\
static int my_avl_##PREFIX##_kill_smallest(STRUCT* table, RECTYPE* n, RECTYPE* w) \
{ \
	int tmp; \
	\
	assert (*n != INVALID); \
	\
	if (table[*n].left != INVALID) \
	{ \
		if ((tmp = my_avl_##PREFIX##_kill_smallest( \
			table, &table[*n].left, w)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_left_shrunk(table, n); \
		} \
		return tmp; \
	} \
	\
	*w = *n; \
	*n = table[*n].right; \
	\
	return AVL_BALANCE; \
} \
\
static int my_avl_##PREFIX##_rmv(STRUCT* table, RECTYPE* n, RECTYPE victim) \
{ \
	int dir, tmp; \
	\
	if (*n == INVALID) \
	{ \
		return AVL_NOT_FOUND; \
	} \
	\
	dir = COMPARE(table[victim].key, table[*n].key); \
	\
	if (dir < 0) \
	{ \
		if ((tmp = my_avl_##PREFIX##_rmv( \
			table, &table[*n].left, victim)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_left_shrunk(table, n); \
		} \
		return tmp; \
	} \
	\
	if (dir > 0) \
	{ \
		if ((tmp = my_avl_##PREFIX##_rmv( \
			table, &table[*n].right, victim)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_right_shrunk(table, n); \
		} \
		return tmp; \
	} \
	\
	assert (*n == victim); \
	\
	if (table[*n].left != INVALID) \
	{ \
		RECTYPE tn; \
		tmp = my_avl_##PREFIX##_kill_largest( \
			table, &table[*n].left, &tn); \
		\
		table[tn].left  = table[*n].left; \
		table[tn].right = table[*n].right; \
		table[tn].skew  = table[*n].skew; \
		*n = tn; \
		\
		if (tmp == AVL_BALANCE) \
			return my_avl_##PREFIX##_left_shrunk(table, n); \
		return tmp; \
	} \
	\
	if (table[*n].right != INVALID) \
	{ \
		RECTYPE tn; \
		tmp = my_avl_##PREFIX##_kill_smallest( \
			table, &table[*n].right, &tn); \
		\
		table[tn].left  = table[*n].left; \
		table[tn].right = table[*n].right; \
		table[tn].skew  = table[*n].skew; \
		*n = tn; \
		\
		if (tmp == AVL_BALANCE) \
			return my_avl_##PREFIX##_right_shrunk(table, n); \
		return tmp; \
	} \
	\
	*n = INVALID; \
	return AVL_BALANCE; \
} \
\
static RECTYPE my_avl_##PREFIX##_remove(STRUCT* table, RECTYPE root, RECTYPE recno) \
{ \
	if (my_avl_##PREFIX##_rmv(table, &root, recno) == AVL_NOT_FOUND) \
		return INVALID; \
	\
	return root; \
}
