/*  $Id: indexer.h,v 1.8 2002-05-09 06:28:58 terpstra Exp $
 *  
 *  indexer.h - Handles indexing a message for keyword searching
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

/*------------------------------------------------- Public component methods */

extern int lu_indexer_init (void);
extern int lu_indexer_open (void);
extern int lu_indexer_sync (void);
extern int lu_indexer_close(void);
extern int lu_indexer_quit (void);

/*------------------------------------------------- Indexing algorithm */

struct Lu_Mbox_Message;

/** This will prepare for a new bunch of keywords 
 */
void lu_indexer_prep();

/** Call this to push all the keywords related to the location of the message
 */
void lu_indexer_location(
	lu_word		list,
	lu_word		mbox,
	message_id	thread,
	int		is_head);

/** Call this to push all the keywords related to this message
 */
void lu_indexer_message(
	struct Lu_Mbox_Message*	body, 
	time_t			stamp,
	const char*		reply_id);

/** Call this to dump all the keywords added to the disk 
 */
int lu_indexer_dump(message_id id);
