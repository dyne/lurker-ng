/*  $Id: btree.h,v 1.3 2002-02-20 03:24:25 terpstra Exp $
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
#define LU_BTREE_BALANCE	1
#define LU_BTREE_OK		0

/* The user of this file should call:
 *   LU_BTREE_DEFINE(PREFIX, RECTYPE, INVALID, STRUCT, TABLE, SIZE, COMPARE)
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
 * This will create the method: 
 *   lu_btree_PREFIX_insert(RECTYPE recno)
 *     -> you must have already set key to an appropriate value.
 *     -> -1 if already in tree, 0 else (ok)
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
static int my_btree_##PREFIX##_insert(RECTYPE recno) \
{ \
	RECTYPE root = 0; \
	\
	if (recno == 0) \
	{ \
		TABLE[0].left = TABLE[0].right = INVALID; \
		TABLE[0].skew = LU_BTREE_EQUAL; \
		return 0; \
	} \
	\
	if (my_btree_##PREFIX##_ins(&root, recno) == LU_BTREE_ALREADY_THERE) \
		return -1; \
	\
	return 0; \
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
}
