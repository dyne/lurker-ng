/*  $Id: globals.h,v 1.4 2002-01-23 07:33:12 terpstra Exp $
 *  
 *  globals.h - the global variables and methods for the daemon
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

#include <db.h>

typedef struct Mbox_T
{
	lu_word	id;
	char*	path;
	int	fd;
	
	/* only used during load of config */
	struct Mbox_T*	next;
} Mbox;

typedef struct List_T
{
	lu_word	id;
	
	char*	name;
	char*	address;
	char*	description;
	
	Mbox*	mbox;
	int	mboxs;
	
	/* only used during load of config */
	Mbox*		mbox_head;
	struct List_T*	next;
} List;

extern char*	lu_dbdir;
extern char*	lu_wwwdir;
extern char*	lu_pidfile;

extern List*	lu_list;
extern int	lu_lists;

extern int	lu_summary_fd;
extern int	lu_variable_fd;
extern int	lu_keyword_fd;
extern DB_ENV*	lu_db_env;
extern DB*	lu_thread_db;
extern DB*	lu_merge_db;
extern DB*	lu_mbox_db;
extern DB*	lu_keyword_db;

/** This loads the configuration file into the appropriate globals.
 *  It also opens all the mboxs for the tail functionality.
 */
extern int lu_load_config(const char* cfg);

/** This opens all the databases that we track data with.
 */
extern int lu_open_db();

/** Double check that the mboxs haven't been switched. Then move the read
 *  heads to the right locations.
 */
extern int lu_sync_mbox();

/** Grab the summary information record for the specified mid
 */
extern MessageSummary lu_read_msummary(message_id mid);

/** Grab the summary information record for the specified tid
 */
extern ThreadSummary  lu_read_tsummary(message_id tid);

/** This writes 'subject<null>authorname<null>authoremail<null>' to the
 *  specified file. This should be all the is required outside the db.c
 *  module. The flat_offset can be found in a message summary record.
 */
extern int lu_write_variable(FILE* out, lu_addr flat_offset);

/** Pass the timestamp of the server arrival time and the client send
 *  time. The heuristic of forward import is used to calculate a real
 *  import time. The caller should check all candidate messages and select
 *  that which has the earliest timestamp to import first.
 */
extern time_t lu_timestamp_heuristic(time_t server, time_t client);

/** This imports the passed summary information about the message into
 *  our database. You should have selected the message with the earliest
 *  timestamp_heuristic available.
 *  
 *  Next, you have to call lu_reply_to_resolution.
 */
extern message_id lu_import_message(
	lu_word list, lu_word mbox, lu_addr mbox_offset,
	time_t timestamp, 
	const char* subject,
	const char* author_name,
	const char* author_email);

/** This connects reply-to links for (possibly out of sequence) messages.
 *  Don't forget to push all the body keywords after this.
 */
extern int lu_reply_to_resolution(
	message_id id,
	const char* msg_id,
	const char* reply_to_msg_id);

/** Search the configuration for the mailing list which has the specified id
 */
extern List* lu_find_list(lu_word id);

/** Search the configuration for the mbox which has the specified id.
 */
extern Mbox* lu_find_mbox(List* list, lu_word id);
