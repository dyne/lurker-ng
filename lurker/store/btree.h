/*  $Id: btree.h,v 1.8 2002-06-21 13:15:37 terpstra Exp $
 *  
 *  btree.h - Knows how manage a binary search tree
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: GPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; version 2.1.
 *    
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *    
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* This is just a collection of macros that can be dumped on top of any type
 * underlying buffer which we are using as a bst. I wish I could use C++ 
 * templates, but I swore an oath! :-)
 *
 * Much assistence in writing this file was derived from:
 *  http://www.purists.org/georg/avltree/
 */

#define LU_BTREE_EQUAL	0
#define LU_BTREE_LEFT	1
#define LU_BTREE_RIGHT	2

#define LU_BTREE_ALREADY_THERE	-1
#define LU_BTREE_NOT_FOUND	-1
#define LU_BTREE_BALANCE	1
#define LU_BTREE_OK		0

/* The user of this file should call:
 *   LU_BTREE_DEFINE(PREFIX, RECTYPE, INVALID, STRUCT, TABLE, COMPARE)
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
 *   RECTYPE my_btree_PREFIX_insert(RECTYPE root, RECTYPE recno)
 *     -> you must have already set TABLE[recno].key to an appropriate value.
 *     -> INVALID if already in tree, the new root else (ok)
 *   RECTYPE my_btree_PREFIX_remove(RECTYPE root, RECTYPE recno)
 *     -> INVALID if not in tree, the new root else (ok)
 */

#define LU_BTREE_DEFINE(PREFIX, RECTYPE, INVALID, STRUCT, TABLE, COMPARE) \
\
\
inline void my_btree_##PREFIX##_rot_left(RECTYPE* n) \
{ \
	RECTYPE tmp = *n; \
	\
	*n = TABLE[*n].right; \
	TABLE[tmp].right = TABLE[*n].left; \
	TABLE[*n].left = tmp; \
} \
\
\
inline void my_btree_##PREFIX##_rot_right(RECTYPE* n) \
{ \
	RECTYPE tmp = *n; \
	\
	*n = TABLE[*n].left; \
	TABLE[tmp].left = TABLE[*n].right; \
	TABLE[*n].right = tmp; \
} \
\
inline int my_btree_##PREFIX##_right_grown(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case LU_BTREE_LEFT: \
		TABLE[*n].skew = LU_BTREE_EQUAL; \
		return LU_BTREE_OK; \
	\
	case LU_BTREE_RIGHT: \
		if (TABLE[TABLE[*n].right].skew == LU_BTREE_RIGHT) \
		{ \
			TABLE[*n].skew = TABLE[TABLE[*n].right].skew = \
				LU_BTREE_EQUAL; \
			my_btree_##PREFIX##_rot_left(n); \
		} \
		else \
		{ \
			if (TABLE[TABLE[*n].right].skew == LU_BTREE_RIGHT) \
			{ \
				TABLE[*n].skew = \
					TABLE[TABLE[*n].right].skew = \
						LU_BTREE_EQUAL; \
				\
				my_btree_##PREFIX##_rot_left(n); \
			} \
			else \
			{ \
				switch (TABLE[TABLE[TABLE[*n].right].left].skew) \
				{ \
				case LU_BTREE_RIGHT: \
					TABLE[*n].skew = LU_BTREE_LEFT; \
					TABLE[TABLE[*n].right].skew = \
						LU_BTREE_EQUAL; \
					break; \
				\
				case LU_BTREE_LEFT: \
					TABLE[*n].skew = LU_BTREE_EQUAL; \
					TABLE[TABLE[*n].right].skew = \
						LU_BTREE_RIGHT; \
					break; \
				\
				case LU_BTREE_EQUAL: \
					TABLE[*n].skew = LU_BTREE_EQUAL; \
					TABLE[TABLE[*n].right].skew = \
						LU_BTREE_EQUAL; \
					break; \
				\
				default: \
					assert(0); \
				} \
				\
				TABLE[TABLE[TABLE[*n].right].left].skew = \
					LU_BTREE_EQUAL; \
				my_btree_##PREFIX##_rot_right(&TABLE[*n].right); \
				my_btree_##PREFIX##_rot_left(n); \
			} \
		} \
		\
		return LU_BTREE_OK; \
	\
	case LU_BTREE_EQUAL: \
		TABLE[*n].skew = LU_BTREE_RIGHT; \
		return LU_BTREE_BALANCE; \
	\
	default: \
		assert(0); \
		return LU_BTREE_OK; \
	} \
} \
\
\
inline int my_btree_##PREFIX##_left_grown(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case LU_BTREE_RIGHT: \
		TABLE[*n].skew = LU_BTREE_EQUAL; \
		return LU_BTREE_OK; \
	\
	case LU_BTREE_LEFT: \
		if (TABLE[TABLE[*n].left].skew == LU_BTREE_LEFT) \
		{ \
			TABLE[*n].skew = TABLE[TABLE[*n].left].skew = \
				LU_BTREE_EQUAL; \
			my_btree_##PREFIX##_rot_right(n); \
		} \
		else \
		{ \
			if (TABLE[TABLE[*n].left].skew == LU_BTREE_LEFT) \
			{ \
				TABLE[*n].skew = \
					TABLE[TABLE[*n].left].skew = \
						LU_BTREE_EQUAL; \
				\
				my_btree_##PREFIX##_rot_right(n); \
			} \
			else \
			{ \
				switch (TABLE[TABLE[TABLE[*n].left].right].skew) \
				{ \
				case LU_BTREE_LEFT: \
					TABLE[*n].skew = LU_BTREE_RIGHT; \
					TABLE[TABLE[*n].left].skew = \
						LU_BTREE_EQUAL; \
					break; \
				\
				case LU_BTREE_RIGHT: \
					TABLE[*n].skew = LU_BTREE_EQUAL; \
					TABLE[TABLE[*n].left].skew = \
						LU_BTREE_LEFT; \
					break; \
				\
				case LU_BTREE_EQUAL: \
					TABLE[*n].skew = LU_BTREE_EQUAL; \
					TABLE[TABLE[*n].left].skew = \
						LU_BTREE_EQUAL; \
					break; \
				\
				default: \
					assert(0); \
				} \
				\
				TABLE[TABLE[TABLE[*n].left].right].skew = \
					LU_BTREE_EQUAL; \
				my_btree_##PREFIX##_rot_left(&TABLE[*n].left); \
				my_btree_##PREFIX##_rot_right(n); \
			} \
		} \
		\
		return LU_BTREE_OK; \
	\
	case LU_BTREE_EQUAL: \
		TABLE[*n].skew = LU_BTREE_LEFT; \
		return LU_BTREE_BALANCE; \
	\
	default: \
		assert(0); \
		return LU_BTREE_OK; \
	} \
} \
\
\
static int my_btree_##PREFIX##_ins(RECTYPE* n, RECTYPE recno) \
{ \
	int tmp, dir; \
	\
	if (*n == INVALID) \
	{ \
		*n = recno; \
		\
		TABLE[recno].left = TABLE[recno].right = INVALID; \
		TABLE[recno].skew = LU_BTREE_EQUAL; \
		return LU_BTREE_BALANCE; \
	} \
	\
	dir = COMPARE(TABLE[recno].key, TABLE[*n].key); \
	\
	if (dir < 0) \
	{ \
		tmp = my_btree_##PREFIX##_ins(&TABLE[*n].left, recno); \
		if (tmp == LU_BTREE_BALANCE) \
		{ \
			return my_btree_##PREFIX##_left_grown(n); \
		} \
		return tmp; \
	} \
	\
	if (dir > 0) \
	{ \
		tmp = my_btree_##PREFIX##_ins(&TABLE[*n].right, recno); \
		if (tmp == LU_BTREE_BALANCE) \
		{ \
			return my_btree_##PREFIX##_right_grown(n); \
		} \
		\
		return tmp; \
	} \
	\
	return LU_BTREE_ALREADY_THERE; \
} \
\
\
static RECTYPE my_btree_##PREFIX##_insert(RECTYPE root, RECTYPE recno) \
{ \
	if (my_btree_##PREFIX##_ins(&root, recno) == LU_BTREE_ALREADY_THERE) \
		return INVALID; \
	\
	return root; \
} \
\
\
inline int my_btree_##PREFIX##_left_shrunk(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case LU_BTREE_LEFT: \
		TABLE[*n].skew = LU_BTREE_EQUAL; \
		return LU_BTREE_BALANCE; \
		\
	case LU_BTREE_EQUAL: \
		TABLE[*n].skew = LU_BTREE_RIGHT; \
		return LU_BTREE_OK; \
		\
	case LU_BTREE_RIGHT: \
		switch (TABLE[TABLE[*n].right].skew) \
		{ \
		case LU_BTREE_RIGHT: \
			TABLE[*n].skew = TABLE[TABLE[*n].right].skew = LU_BTREE_EQUAL; \
			my_btree_##PREFIX##_rot_left(n); \
			return LU_BTREE_BALANCE; \
			\
		case LU_BTREE_EQUAL: \
			TABLE[*n].skew = LU_BTREE_RIGHT; \
			TABLE[TABLE[*n].right].skew = LU_BTREE_LEFT; \
			my_btree_##PREFIX##_rot_left(n); \
			return LU_BTREE_OK; \
			\
		case LU_BTREE_LEFT: \
			switch (TABLE[TABLE[TABLE[*n].right].left].skew) \
			{ \
			case LU_BTREE_LEFT: \
				TABLE[*n].skew = LU_BTREE_EQUAL; \
				TABLE[TABLE[*n].right].skew = LU_BTREE_RIGHT; \
				break; \
				\
			case LU_BTREE_RIGHT: \
				TABLE[*n].skew = LU_BTREE_LEFT; \
				TABLE[TABLE[*n].right].skew = LU_BTREE_EQUAL; \
				break; \
				\
			case LU_BTREE_EQUAL: \
				TABLE[*n].skew = LU_BTREE_EQUAL; \
				TABLE[TABLE[*n].right].skew = LU_BTREE_EQUAL; \
				break; \
				\
			default: \
				assert(0); \
			} \
			\
			TABLE[TABLE[TABLE[*n].right].left].skew = LU_BTREE_EQUAL; \
			my_btree_##PREFIX##_rot_right(&TABLE[*n].right); \
			my_btree_##PREFIX##_rot_left(n); \
			return LU_BTREE_BALANCE; \
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
inline int my_btree_##PREFIX##_right_shrunk(RECTYPE* n) \
{ \
	switch (TABLE[*n].skew) \
	{ \
	case LU_BTREE_RIGHT: \
		TABLE[*n].skew = LU_BTREE_EQUAL; \
		return LU_BTREE_BALANCE; \
		\
	case LU_BTREE_EQUAL: \
		TABLE[*n].skew = LU_BTREE_LEFT; \
		return LU_BTREE_OK; \
		\
	case LU_BTREE_LEFT: \
		switch (TABLE[TABLE[*n].left].skew) \
		{ \
		case LU_BTREE_LEFT: \
			TABLE[*n].skew = TABLE[TABLE[*n].left].skew = LU_BTREE_EQUAL; \
			my_btree_##PREFIX##_rot_right(n); \
			return LU_BTREE_BALANCE; \
			\
		case LU_BTREE_EQUAL: \
			TABLE[*n].skew = LU_BTREE_LEFT; \
			TABLE[TABLE[*n].left].skew = LU_BTREE_RIGHT; \
			my_btree_##PREFIX##_rot_right(n); \
			return LU_BTREE_OK; \
			\
		case LU_BTREE_RIGHT: \
			switch (TABLE[TABLE[TABLE[*n].left].right].skew) \
			{ \
			case LU_BTREE_RIGHT: \
				TABLE[*n].skew = LU_BTREE_EQUAL; \
				TABLE[TABLE[*n].left].skew = LU_BTREE_LEFT; \
				break; \
				\
			case LU_BTREE_LEFT: \
				TABLE[*n].skew = LU_BTREE_RIGHT; \
				TABLE[TABLE[*n].left].skew = LU_BTREE_EQUAL; \
				break; \
				\
			case LU_BTREE_EQUAL: \
				TABLE[*n].skew = LU_BTREE_EQUAL; \
				TABLE[TABLE[*n].left].skew = LU_BTREE_EQUAL; \
				break; \
				\
			default: \
				assert(0); \
			} \
			\
			TABLE[TABLE[TABLE[*n].left].right].skew = LU_BTREE_EQUAL; \
			my_btree_##PREFIX##_rot_left(&TABLE[*n].left); \
			my_btree_##PREFIX##_rot_right(n); \
			return LU_BTREE_BALANCE; \
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
inline int my_btree_##PREFIX##_kill_largest(RECTYPE* n, RECTYPE* w) \
{ \
	int tmp; \
	\
	assert (*n != INVALID); \
	\
	if (TABLE[*n].right != INVALID) \
	{ \
		if ((tmp = my_btree_##PREFIX##_kill_largest( \
			&TABLE[*n].right, w)) == LU_BTREE_BALANCE) \
		{ \
			return my_btree_##PREFIX##_right_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	*w = *n; \
	*n = TABLE[*n].left; \
	\
	return LU_BTREE_BALANCE; \
} \
\
inline int my_btree_##PREFIX##_kill_smallest(RECTYPE* n, RECTYPE* w) \
{ \
	int tmp; \
	\
	assert (*n != INVALID); \
	\
	if (TABLE[*n].left != INVALID) \
	{ \
		if ((tmp = my_btree_##PREFIX##_kill_smallest( \
			&TABLE[*n].left, w)) == LU_BTREE_BALANCE) \
		{ \
			return my_btree_##PREFIX##_left_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	*w = *n; \
	*n = TABLE[*n].right; \
	\
	return LU_BTREE_BALANCE; \
} \
\
static int my_btree_##PREFIX##_rmv(RECTYPE* n, RECTYPE victim) \
{ \
	int dir, tmp; \
	\
	if (*n == INVALID) \
	{ \
		return LU_BTREE_NOT_FOUND; \
	} \
	\
	dir = COMPARE(TABLE[victim].key, TABLE[*n].key); \
	\
	if (dir < 0) \
	{ \
		if ((tmp = my_btree_##PREFIX##_rmv( \
			&TABLE[*n].left, victim)) == LU_BTREE_BALANCE) \
		{ \
			return my_btree_##PREFIX##_left_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	if (dir > 0) \
	{ \
		if ((tmp = my_btree_##PREFIX##_rmv( \
			&TABLE[*n].right, victim)) == LU_BTREE_BALANCE) \
		{ \
			return my_btree_##PREFIX##_right_shrunk(n); \
		} \
		return tmp; \
	} \
	\
	assert (*n == victim); \
	\
	if (TABLE[*n].left != INVALID) \
	{ \
		RECTYPE tn; \
		int tmp = my_btree_##PREFIX##_kill_largest( \
			&TABLE[*n].left, &tn); \
		\
		*n = tn; \
		TABLE[tn].left  = TABLE[*n].left; \
		TABLE[tn].right = TABLE[*n].right; \
		TABLE[tn].skew  = TABLE[*n].skew; \
		\
		if (tmp == LU_BTREE_BALANCE) \
			return my_btree_##PREFIX##_left_shrunk(n); \
		return tmp; \
	} \
	\
	if (TABLE[*n].right != INVALID) \
	{ \
		RECTYPE tn; \
		int tmp = my_btree_##PREFIX##_kill_smallest( \
			&TABLE[*n].right, &tn); \
		\
		*n = tn; \
		TABLE[tn].left  = TABLE[*n].left; \
		TABLE[tn].right = TABLE[*n].right; \
		TABLE[tn].skew  = TABLE[*n].skew; \
		\
		if (tmp == LU_BTREE_BALANCE) \
			return my_btree_##PREFIX##_right_shrunk(n); \
		return tmp; \
	} \
	\
	*n = INVALID; \
	return LU_BTREE_BALANCE; \
} \
\
static RECTYPE my_btree_##PREFIX##_remove(RECTYPE root, RECTYPE recno) \
{ \
	if (my_btree_##PREFIX##_rmv(&root, recno) == LU_BTREE_NOT_FOUND) \
		return INVALID; \
	\
	return root; \
}
