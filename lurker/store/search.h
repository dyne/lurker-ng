/*  $Id: search.h,v 1.4 2002-06-07 10:30:40 terpstra Exp $
 *  
 *  search.h - Uses the breader to execute a given search
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

extern int lu_search_init (void);
extern int lu_search_open (void);
extern int lu_search_sync (void);
extern int lu_search_close(void);
extern int lu_search_quit (void);

/*------------------------------------------------- Public search methods */

/* Begin a search. Expects a space delimited, null terminated search string.
 * Returns: -1 internal error
 *          0  success
 *          1  bad search string
 *
 * message holds information that should be displayed to the user.
 */
extern int lu_search_start(
	const char* keywords,
	const char** message);

/* Retrieve the next result.
 */
extern int lu_search_result(
	message_id* result);

/* Shutdown the current search.
 */
extern int lu_search_end(
	message_id* predict);
