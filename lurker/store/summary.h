/*  $Id: summary.h,v 1.3 2002-02-10 21:50:38 terpstra Exp $
 *  
 *  summary.h - Knows how to manage digested mail information
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

/*------------------------------------------------ Public types */            

/** Message summary database - summary.flat
 *
 *  This is a flat file that can be indexed directly by docid.
 *  It is 64 bytes big. <-- nice and small. :-)
 */
typedef struct Lu_Summary_Message_T
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
} Lu_Summary_Message;

/** Message variable length data database - variable.flat
 *   the subject  <null>
 *   author name  <null>
 *   author email <null>
 */

/** Thread summary info - thread.hash
 *  Size - 24 bytes
 */
typedef struct Lu_Summary_Thread_T
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
} Lu_Summary_Thread;

/*------------------------------------------------- Public component methods */

extern int lu_summary_init (void);
extern int lu_summary_open (void);
extern int lu_summary_sync (void);
extern int lu_summary_close(void);
extern int lu_summary_quit (void);

/*------------------------------------------------- Direct access methods */

/** Grab the summary information record for the specified mid
 */
extern Lu_Summary_Message lu_summary_read_msummary(
	message_id mid);

/** Grab the summary information record for the specified tid
 */
extern Lu_Summary_Thread lu_summary_read_tsummary(
	message_id tid);

/** Find the most recent thread id in a mailing list.
 *  lu_common_minvalid -> none or error
 */
extern int lu_summary_last_thread(
	lu_word list, 
	message_id* out);

/** This writes 'subject<null>authorname<null>authoremail<null>' to the
 *  specified file. This should be all the is required outside the db.c
 *  module. The flat_offset can be found in a message summary record.
 */
extern int lu_summary_write_variable(
	int (*write)(void* arg, const char* str),
	int (*quote)(void* arg, const char* str, size_t len),
	void* arg,
	lu_addr flat_offset);

/** Pass the timestamp of the server arrival time and the client send
 *  time. The heuristic of forward import is used to calculate a real
 *  import time. The caller should check all candidate messages and select
 *  that which has the earliest timestamp to import first.
 */
extern time_t lu_summary_timestamp_heuristic(
	time_t server, 
	time_t client);

/*------------------------------------------------- Import message methods */

/** This imports the passed summary information about the message into
 *  our database. You should have selected the message with the earliest
 *  timestamp_heuristic available.
 *  
 *  This operation is irreversable. You cannot make the message go away if
 *  the later operations fail. Therefore, it checks to see if we have
 *  already loaded this message. If we have, it reports success again.
 *  
 *  Next, you have to call lu_reply_to_resolution.
 */
extern message_id lu_summary_import_message(
	lu_word list, 
	lu_word mbox, 
	lu_addr mbox_offset,
	time_t timestamp, 
	const char* subject,
	const char* author_name,
	const char* author_email);

/** This connects reply-to links for (possibly out of sequence) messages.
 *  Don't forget to push all the keywords after this (body, subject, list...)
 */
extern int lu_summary_reply_to_resolution(
	message_id id,
	const char* msg_id,
	const char* reply_to_msg_id);
