/*  $Id: mbox.h,v 1.8 2002-07-19 12:24:17 terpstra Exp $
 *  
 *  mbox.h - Knows how to follow mboxes for appends and import messages
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

#include <c-client/mail.h>
#include <c-client/rfc822.h>
#include <c-client/fs.h>

/*------------------------------------------------- Public types */

struct Lu_Mbox_Message
{
	char*				buffer;
	struct	mail_envelope*		env;
	struct	mail_bodystruct*	body;
	STRING				bss;
};

/*------------------------------------------------- Public variables */

extern int lu_mbox_disable_watch;

/*------------------------------------------------- Public methods */

struct Lu_Config_Mbox_T;
struct Lu_Config_Mmap;
struct Lu_Config_Message;

extern char* lu_mbox_select_body(
	struct Lu_Mbox_Message *, 
	BODY *, 
	size_t *, 
	int *);

/* Mmap a message into memory from the specified mbox/start location.
 * The msg structure will have the mmap reused if possible.
 * You should have initialized msg with memset.
 */
extern int lu_mbox_map_message(
	struct Lu_Config_Mbox_T*	mbox, 
	struct Lu_Config_Message*	msg,
	off_t				start);

/* This will unmap the memory used by the specified message.
 */
extern int lu_mbox_destroy_map(
	struct Lu_Config_Message*	msg);
	
/* This will parse a given message from a message mmap.
 */
extern int lu_mbox_parse_message(
	struct Lu_Config_Message*	mmap,
	struct Lu_Mbox_Message*		msg);

/* This releases resources used by the message.
 */
extern int lu_mbox_destroy_message(
	struct Lu_Mbox_Message* m);

/* This finds a charset attribute for the given body.
 */
const char* lu_mbox_find_charset(
	struct mail_bodystruct* body);

/*------------------------------------------------- Public component methods */

extern int lu_mbox_init (void);
extern int lu_mbox_open (void);
extern int lu_mbox_sync (void);
extern int lu_mbox_close(void);
extern int lu_mbox_quit (void);
