/*  $Id: keyword.h,v 1.2 2002-01-23 07:33:12 terpstra Exp $
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

struct Search_T;
typedef struct Search_T* Search;

/** Initialize the keyword database in case it's empty.
 */
extern int lu_keyword_init();

/** Record that named keyword appears in the message.
 */
extern int lu_push_keyword(const char* keyword, message_id message);

/** Pop the last record off the keyword.
 *  This is intended for backing out of indexing a message.
 */
extern int lu_pop_keyword(const char* keyword);

/** Start a new keyword search.
 */
extern Search lu_new_search(const char* words);

/** Skip ahead <n> records.
 */

/** Extract the next matching document.
 */
extern message_id lu_search_next(Search search);

/** Free the resources used by the search
 */
extern void lu_free_search(Search search);
