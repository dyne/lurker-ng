/*  $Id: avl.h,v 1.5 2002-07-11 23:50:54 terpstra Exp $
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
 *   AVL_DEFINE_INSERT(PREFIX, RECTYPE, INVALID, STRUCT, TABLE, COMPARE)
 *   AVL_DEFINE_REMOVE(PREFIX, RECTYPE, INVALID, STRUCT, TABLE, COMPARE)
 *
 *   PREFIX  -> an indetifier to prevent symbol conflict
 *   RECTYPE -> the type used to index the table
 *   INVALID -> largest representable size
 *   STRUCT  -> the type of the records maintained as an avl tree
 *	RECTYPE left, right;
 *      sometype key;		<- sometype must be used by COMPARE function
 *      sometype skew;		<- sometype must have >= 2 bits of storage
 *   TABLE   -> the base table pointer of type STRUCT
 *   COMPARE -> a strcmp style compare method
 *
 * This will create the methods: 
 *   RECTYPE my_avl_PREFIX_insert(RECTYPE root, RECTYPE recno)
 *     -> you must have already set TABLE[recno].key to an appropriate value.
 *     -> INVALID if already in tree, the new root else (ok)
 *   RECTYPE my_avl_PREFIX_remove(RECTYPE root, RECTYPE recno)
 *     -> INVALID if not in tree, the new root else (ok)
 */

#define AVL_DEFINE_INSERT(PREFIX, RECTYPE, INVALID, STRUCT, TABLE, COMPARE) \
\
\
inline void my_avl_##PREFIX##_rot_left(RECTYPE* n) \
{ \
	RECTYPE tmp = *n; \
	\
	*n = TABLE[*n].right; \
	TABLE[tmp].right = TABLE[*n].left; \
	TABLE[*n].left = tmp; \
} \
\
\
inline void my_avl_##PREFIX##_rot_right(RECTYPE* n) \
{ \
	RECTYPE tmp = *n; \
	\
	*n = TABLE[*n].left; \
	TABLE[tmp].left = TABLE[*n].right; \
	TABLE[*n].right = tmp; \
} \
\
inline int my_avl_##PREFIX##_right_grown(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case AVL_LEFT: \
		TABLE[*n].skew = AVL_EQUAL; \
		return AVL_OK; \
	\
	case AVL_RIGHT: \
		if (TABLE[TABLE[*n].right].skew == AVL_RIGHT) \
		{ \
			TABLE[*n].skew = TABLE[TABLE[*n].right].skew = \
				AVL_EQUAL; \
			my_avl_##PREFIX##_rot_left(n); \
		} \
		else \
		{ \
			if (TABLE[TABLE[*n].right].skew == AVL_RIGHT) \
			{ \
				TABLE[*n].skew = \
					TABLE[TABLE[*n].right].skew = \
						AVL_EQUAL; \
				\
				my_avl_##PREFIX##_rot_left(n); \
			} \
			else \
			{ \
				switch (TABLE[TABLE[TABLE[*n].right].left].skew) \
				{ \
				case AVL_RIGHT: \
					TABLE[*n].skew = AVL_LEFT; \
					TABLE[TABLE[*n].right].skew = \
						AVL_EQUAL; \
					break; \
				\
				case AVL_LEFT: \
					TABLE[*n].skew = AVL_EQUAL; \
					TABLE[TABLE[*n].right].skew = \
						AVL_RIGHT; \
					break; \
				\
				case AVL_EQUAL: \
					TABLE[*n].skew = AVL_EQUAL; \
					TABLE[TABLE[*n].right].skew = \
						AVL_EQUAL; \
					break; \
				\
				default: \
					assert(0); \
				} \
				\
				TABLE[TABLE[TABLE[*n].right].left].skew = \
					AVL_EQUAL; \
				my_avl_##PREFIX##_rot_right(&TABLE[*n].right); \
				my_avl_##PREFIX##_rot_left(n); \
			} \
		} \
		\
		return AVL_OK; \
	\
	case AVL_EQUAL: \
		TABLE[*n].skew = AVL_RIGHT; \
		return AVL_BALANCE; \
	\
	default: \
		assert(0); \
		return AVL_OK; \
	} \
} \
\
\
inline int my_avl_##PREFIX##_left_grown(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case AVL_RIGHT: \
		TABLE[*n].skew = AVL_EQUAL; \
		return AVL_OK; \
	\
	case AVL_LEFT: \
		if (TABLE[TABLE[*n].left].skew == AVL_LEFT) \
		{ \
			TABLE[*n].skew = TABLE[TABLE[*n].left].skew = \
				AVL_EQUAL; \
			my_avl_##PREFIX##_rot_right(n); \
		} \
		else \
		{ \
			if (TABLE[TABLE[*n].left].skew == AVL_LEFT) \
			{ \
				TABLE[*n].skew = \
					TABLE[TABLE[*n].left].skew = \
						AVL_EQUAL; \
				\
				my_avl_##PREFIX##_rot_right(n); \
			} \
			else \
			{ \
				switch (TABLE[TABLE[TABLE[*n].left].right].skew) \
				{ \
				case AVL_LEFT: \
					TABLE[*n].skew = AVL_RIGHT; \
					TABLE[TABLE[*n].left].skew = \
						AVL_EQUAL; \
					break; \
				\
				case AVL_RIGHT: \
					TABLE[*n].skew = AVL_EQUAL; \
					TABLE[TABLE[*n].left].skew = \
						AVL_LEFT; \
					break; \
				\
				case AVL_EQUAL: \
					TABLE[*n].skew = AVL_EQUAL; \
					TABLE[TABLE[*n].left].skew = \
						AVL_EQUAL; \
					break; \
				\
				default: \
					assert(0); \
				} \
				\
				TABLE[TABLE[TABLE[*n].left].right].skew = \
					AVL_EQUAL; \
				my_avl_##PREFIX##_rot_left(&TABLE[*n].left); \
				my_avl_##PREFIX##_rot_right(n); \
			} \
		} \
		\
		return AVL_OK; \
	\
	case AVL_EQUAL: \
		TABLE[*n].skew = AVL_LEFT; \
		return AVL_BALANCE; \
	\
	default: \
		assert(0); \
		return AVL_OK; \
	} \
} \
\
\
static int my_avl_##PREFIX##_ins(RECTYPE* n, RECTYPE recno) \
{ \
	int tmp, dir; \
	\
	if (*n == INVALID) \
	{ \
		*n = recno; \
		\
		TABLE[recno].left = TABLE[recno].right = INVALID; \
		TABLE[recno].skew = AVL_EQUAL; \
		return AVL_BALANCE; \
	} \
	\
	dir = COMPARE(TABLE[recno].key, TABLE[*n].key); \
	\
	if (dir < 0) \
	{ \
		tmp = my_avl_##PREFIX##_ins(&TABLE[*n].left, recno); \
		if (tmp == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_left_grown(n); \
		} \
		return tmp; \
	} \
	\
	if (dir > 0) \
	{ \
		tmp = my_avl_##PREFIX##_ins(&TABLE[*n].right, recno); \
		if (tmp == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_right_grown(n); \
		} \
		\
		return tmp; \
	} \
	\
	return AVL_ALREADY_THERE; \
} \
\
\
static RECTYPE my_avl_##PREFIX##_insert(RECTYPE root, RECTYPE recno) \
{ \
	if (my_avl_##PREFIX##_ins(&root, recno) == AVL_ALREADY_THERE) \
		return INVALID; \
	\
	return root; \
}





#define AVL_DEFINE_REMOVE(PREFIX, RECTYPE, INVALID, STRUCT, TABLE, COMPARE) \
\
\
inline int my_avl_##PREFIX##_left_shrunk(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case AVL_LEFT: \
		TABLE[*n].skew = AVL_EQUAL; \
		return AVL_BALANCE; \
		\
	case AVL_EQUAL: \
		TABLE[*n].skew = AVL_RIGHT; \
		return AVL_OK; \
		\
	case AVL_RIGHT: \
		switch (TABLE[TABLE[*n].right].skew) \
		{ \
		case AVL_RIGHT: \
			TABLE[*n].skew = TABLE[TABLE[*n].right].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_left(n); \
			return AVL_BALANCE; \
			\
		case AVL_EQUAL: \
			TABLE[*n].skew = AVL_RIGHT; \
			TABLE[TABLE[*n].right].skew = AVL_LEFT; \
			my_avl_##PREFIX##_rot_left(n); \
			return AVL_OK; \
			\
		case AVL_LEFT: \
			switch (TABLE[TABLE[TABLE[*n].right].left].skew) \
			{ \
			case AVL_LEFT: \
				TABLE[*n].skew = AVL_EQUAL; \
				TABLE[TABLE[*n].right].skew = AVL_RIGHT; \
				break; \
				\
			case AVL_RIGHT: \
				TABLE[*n].skew = AVL_LEFT; \
				TABLE[TABLE[*n].right].skew = AVL_EQUAL; \
				break; \
				\
			case AVL_EQUAL: \
				TABLE[*n].skew = AVL_EQUAL; \
				TABLE[TABLE[*n].right].skew = AVL_EQUAL; \
				break; \
				\
			default: \
				assert(0); \
			} \
			\
			TABLE[TABLE[TABLE[*n].right].left].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_right(&TABLE[*n].right); \
			my_avl_##PREFIX##_rot_left(n); \
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
inline int my_avl_##PREFIX##_right_shrunk(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case AVL_RIGHT: \
		TABLE[*n].skew = AVL_EQUAL; \
		return AVL_BALANCE; \
		\
	case AVL_EQUAL: \
		TABLE[*n].skew = AVL_LEFT; \
		return AVL_OK; \
		\
	case AVL_LEFT: \
		switch (TABLE[TABLE[*n].left].skew) \
		{ \
		case AVL_LEFT: \
			TABLE[*n].skew = TABLE[TABLE[*n].left].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_right(n); \
			return AVL_BALANCE; \
			\
		case AVL_EQUAL: \
			TABLE[*n].skew = AVL_LEFT; \
			TABLE[TABLE[*n].left].skew = AVL_RIGHT; \
			my_avl_##PREFIX##_rot_right(n); \
			return AVL_OK; \
			\
		case AVL_RIGHT: \
			switch (TABLE[TABLE[TABLE[*n].left].right].skew) \
			{ \
			case AVL_RIGHT: \
				TABLE[*n].skew = AVL_EQUAL; \
				TABLE[TABLE[*n].left].skew = AVL_LEFT; \
				break; \
				\
			case AVL_LEFT: \
				TABLE[*n].skew = AVL_RIGHT; \
				TABLE[TABLE[*n].left].skew = AVL_EQUAL; \
				break; \
				\
			case AVL_EQUAL: \
				TABLE[*n].skew = AVL_EQUAL; \
				TABLE[TABLE[*n].left].skew = AVL_EQUAL; \
				break; \
				\
			default: \
				assert(0); \
			} \
			\
			TABLE[TABLE[TABLE[*n].left].right].skew = AVL_EQUAL; \
			my_avl_##PREFIX##_rot_left(&TABLE[*n].left); \
			my_avl_##PREFIX##_rot_right(n); \
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
static int my_avl_##PREFIX##_kill_largest(RECTYPE* n, RECTYPE* w) \
{ \
	int tmp; \
	\
	assert (*n != INVALID); \
	\
	if (TABLE[*n].right != INVALID) \
	{ \
		if ((tmp = my_avl_##PREFIX##_kill_largest( \
			&TABLE[*n].right, w)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_right_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	*w = *n; \
	*n = TABLE[*n].left; \
	\
	return AVL_BALANCE; \
} \
\
static int my_avl_##PREFIX##_kill_smallest(RECTYPE* n, RECTYPE* w) \
{ \
	int tmp; \
	\
	assert (*n != INVALID); \
	\
	if (TABLE[*n].left != INVALID) \
	{ \
		if ((tmp = my_avl_##PREFIX##_kill_smallest( \
			&TABLE[*n].left, w)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_left_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	*w = *n; \
	*n = TABLE[*n].right; \
	\
	return AVL_BALANCE; \
} \
\
static int my_avl_##PREFIX##_rmv(RECTYPE* n, RECTYPE victim) \
{ \
	int dir, tmp; \
	\
	if (*n == INVALID) \
	{ \
		return AVL_NOT_FOUND; \
	} \
	\
	dir = COMPARE(TABLE[victim].key, TABLE[*n].key); \
	\
	if (dir < 0) \
	{ \
		if ((tmp = my_avl_##PREFIX##_rmv( \
			&TABLE[*n].left, victim)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_left_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	if (dir > 0) \
	{ \
		if ((tmp = my_avl_##PREFIX##_rmv( \
			&TABLE[*n].right, victim)) == AVL_BALANCE) \
		{ \
			return my_avl_##PREFIX##_right_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	assert (*n == victim); \
	\
	if (TABLE[*n].left != INVALID) \
	{ \
		RECTYPE tn; \
		tmp = my_avl_##PREFIX##_kill_largest( \
			&TABLE[*n].left, &tn); \
		\
		TABLE[tn].left  = TABLE[*n].left; \
		TABLE[tn].right = TABLE[*n].right; \
		TABLE[tn].skew  = TABLE[*n].skew; \
		*n = tn; \
		\
		if (tmp == AVL_BALANCE) \
			return my_avl_##PREFIX##_left_shrunk(n); \
		return tmp; \
	} \
	\
	if (TABLE[*n].right != INVALID) \
	{ \
		RECTYPE tn; \
		tmp = my_avl_##PREFIX##_kill_smallest( \
			&TABLE[*n].right, &tn); \
		\
		TABLE[tn].left  = TABLE[*n].left; \
		TABLE[tn].right = TABLE[*n].right; \
		TABLE[tn].skew  = TABLE[*n].skew; \
		*n = tn; \
		\
		if (tmp == AVL_BALANCE) \
			return my_avl_##PREFIX##_right_shrunk(n); \
		return tmp; \
	} \
	\
	*n = INVALID; \
	return AVL_BALANCE; \
} \
\
static RECTYPE my_avl_##PREFIX##_remove(RECTYPE root, RECTYPE recno) \
{ \
	if (my_avl_##PREFIX##_rmv(&root, recno) == AVL_NOT_FOUND) \
		return INVALID; \
	\
	return root; \
}
