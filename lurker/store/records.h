/*  $Id: records.h,v 1.2 2002-01-21 02:35:26 terpstra Exp $
 *  
 *  records.h - the format of the databases
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

#include "common.h"

#ifdef HAVE_SYS_TYPES_H 
#include <sys/types.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif 

/* lu_import_message */
#define	LU_KEYWORD_LIST		'l'
#define LU_KEYWORD_AUTHOR	'a'
#define LU_KEYWORD_SUBJECT	's'

/* lu_reply_to_resolution */
#define LU_KEYWORD_MESSAGE_ID	'i'
#define LU_KEYWORD_REPLY_TO	'r'

/* import processing */
#define LU_KEYWORD_WORD		'w'

/** This is the type we use as a unique message identifier
 */
typedef lu_quad	message_id;

/** Message summary database - summary.flat
 *
 *  This is a flat file that can be indexed directly by docid.
 *  It is 64 bytes big. <-- nice and small. :-)
 */
typedef struct MessageSummary_T
{
	/* index into mbox
	 * high 16 bits are mbox index number
	 */
	lu_addr		mbox_offset;
	
	/* text file ptr
	 * high 16 bits are list index number
	 */
	lu_addr		flat_offset;
	
	/* timestamp of the message */
	lu_quad		timestamp;
	
	/* message that this is in-reply-to */
	message_id	in_reply_to;
	
	/* union-find parent ptr */
	message_id	thread_parent;
	
	/* next ptr for thread list */
	message_id	thread_next;
} MessageSummary;

/** Message variable length data database - variable.flat
 *   the subject  <null>
 *   author name  <null>
 *   author email <null>
 */

/** Thread summary info - thread.hash
 *  Size - 24 bytes
 */
typedef struct ThreadSummary_T
{
	/* Start date of the thread */
	lu_quad		start;
	
	/* End date of the thread */
	lu_quad		end;
	
	/* Number of messages in the thread */
	message_id	message_count;
	
	/* Last record in the thread list */
	message_id	thread_end;
	
	/* The next thread in mbox (sorted by end time) */
	message_id	next_thread;
	
	/* The prev thread in mbox (sorted by end time) */
	message_id	prev_thread;
} ThreadSummary;

/** Thread merging meta-data key - merge.hash
 *  Size - ?
 */
typedef struct ThreadMergeKey_T
{
	lu_word		list_index;
	char		squishy_subject[0];
	/* Followed by:
	 * lu_quad	start;
	 */
} ThreadMergeKey;

extern int lu_open_db();
