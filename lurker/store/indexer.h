/*  $Id: indexer.h,v 1.5 2002-02-21 03:51:38 terpstra Exp $
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

/* Call this to push all the keywords related to this file into the index
 */
int lu_indexer_import(
	struct Lu_Mbox_Message*	body, 
	lu_word		list,
	lu_word		mbox,
	time_t		stamp,
	message_id	id);
