/*  $Id: keyword.h,v 1.6 2002-01-27 01:47:27 terpstra Exp $
 *  
 *  keyword.h - manages a database for keyword searching
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

#include "records.h"

extern message_id lu_kw_invalid;

/** Initialize the keyword database in case it's empty.
 */
extern int lu_keyword_init();

/*-------------------------------------------------- Indexing methods */

/** Record that named keyword appears in the message.
 */
extern int lu_push_keyword(const char* keyword, message_id message);

/** Pop the last record off the last keyword if it is 'id'.
 *  This is intended for backing out of indexing a message.
 */
extern int lu_pop_keyword(const char* keyword, message_id id);

/*-------------------------------------------------- Direct access methods */

struct Handle_T;
typedef struct Handle_T* Handle;

/** Open up a record for reading.
 *  Returns 0 to mean failure.
 */
extern Handle lu_open_handle(const char* keyword);

/** How many records are in the handle.
 */
extern message_id lu_handle_records(Handle h);

/** Retrieve the specified range of records.
 */
extern int lu_handle_read(Handle h, message_id index, message_id* buf, message_id count);

/** Close a record handle.
 */
extern void lu_close_handle(Handle h);

/*-------------------------------------------------- Search methods */

struct Search_T;
typedef struct Search_T* Search;

/** Start a new keyword search.
 */
extern Search lu_new_search(const char* const * words);

/** Skip ahead <n> records.
 */
extern int lu_search_skip(Search search, int records);

/** Extract the next matching document. lu_kw_invalid if no more hits.
 */
extern message_id lu_search_next(Search search);

/** Free the resources used by the search
 */
extern void lu_free_search(Search search);

/*-------------------------------------------------- Test methods */

/* These are all noops if DEBUG is not defined */
int lu_test_index();
int lu_test_handle();
int lu_test_search();
