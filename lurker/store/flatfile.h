/*  $Id: flatfile.h,v 1.3 2002-02-03 03:13:27 terpstra Exp $
 *  
 *  flatfile.h - Knows how to manage the keyword flatfile database
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

/*------------------------------------------------- Public types */
 
struct My_Flatfile_Handle_T;
typedef struct My_Flatfile_Handle_T* Lu_Flatfile_Handle;

/*------------------------------------------------- Public component methods */

extern int lu_flatfile_init();
extern int lu_flatfile_open();
extern int lu_flatfile_sync();
extern int lu_flatfile_close();
extern int lu_flatfile_quit();

/*-------------------------------------------------- Append methods */

extern int lu_flatfile_write_keyword_block(
	const char*	keyword, 
	message_id*	buf,
	message_id	count);

extern int lu_flatfile_pop_keyword(
	const char* keyword, 
	message_id id);

/*-------------------------------------------------- Direct access methods */

/** Open up a record for reading.
 *  Returns 0 to mean failure.
 */
extern Lu_Flatfile_Handle lu_flatfile_open_handle(
	const char* keyword);

/** How many records are in the handle.
 */
extern message_id lu_flatfile_handle_records(
	Lu_Flatfile_Handle h);

/** Retrieve the specified range of records.
 */
extern int lu_flatfile_handle_read(
	Lu_Flatfile_Handle h,
	message_id index, 
	message_id* buf, 
	message_id count);

/** Close a record handle.
 */
extern void lu_flatfile_close_handle(
	Lu_Flatfile_Handle h);
