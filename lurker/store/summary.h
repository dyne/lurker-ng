/*  $Id: summary.h,v 1.9 2002-07-11 20:35:07 terpstra Exp $
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
	
	/* number of messages in-reply-to this
	 * in the future we can use this in threading to denote a position
	 * where thread drift occures.
	 */
	lu_quad		replies;
	
	/* message that this is in-reply-to */
	message_id	in_reply_to;
	
	/* thread starting message */
	message_id	thread;
} Lu_Summary_Message;

/** Message variable length data database - variable.flat
 *   the messageid <null>
 *   the subject   <null>
 *   author name   <null>
 *   author email  <null>
 */

/*------------------------------------------------- Public component methods */

extern int lu_summary_init (void);
extern int lu_summary_open (void);
extern int lu_summary_sync (void);
extern int lu_summary_close(void);
extern int lu_summary_quit (void);

/*------------------------------------------------- Direct access methods */

/** Look-up a message by message-id.
 */
extern message_id lu_summary_lookup_mid(
	const char* mmessage_id);

/** Grab the summary information record for the specified mid
 */
extern int lu_summary_read_msummary(
	message_id mid, Lu_Summary_Message* m);

/** This writes the variable information out via the given callbacks.
 *  The flat_offset can be found in a message summary record.
 */
extern int lu_summary_write_variable(
	int (*writefn)(void* arg, const char* str),
	int (*quotefn)(void* arg, const char* str, size_t len),
	int (*qurlfn )(void* arg, const char* str, size_t len),
	void* arg,
	lu_addr flat_offset);

/* This will call a method for each list a message occures in.
 */
extern int lu_summary_write_lists(
	int (*writefn)(void* arg, lu_word list, message_id offset),
	void* arg,
	message_id id);

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
extern int lu_summary_import_message(
	lu_word		list, 
	lu_word		mbox, 
	lu_addr		mbox_offset,
	time_t		timestamp, 
	const char*	mmessage_id,
	const char*	subject,
	const char*	author_name,
	const char*	author_email,
	message_id*	out);

/** This connects reply-to links for (possibly out of sequence) messages.
 *  Don't forget to push all the keywords after this (body, subject, list...)
 */
extern int lu_summary_reply_to_resolution(
	message_id id,
	const char* msg_id,
	const char* reply_to_msg_id);
