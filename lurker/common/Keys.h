/*  $Id: Keys.h,v 1.4 2003-04-25 16:38:17 terpstra Exp $
 *  
 *  Keys.h - What prefixs various database keys
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

#ifndef KEYS_H
#define KEYS_H

#include "keyword.h"
#include <string>
#include <vector>

using std::string;
using std::vector;

// These must have length 1
#define LU_THREADING	"t"
#define LU_KEYWORD	"k"
#define LU_SUMMARY	"s"

#define LU_MESSAGE_AUTHOR_EMAIL	'e'
#define LU_MESSAGE_AUTHOR_NAME	'n'
#define LU_MESSAGE_SUBJECT	's'
#define LU_MESSAGE_MBOX		'x'

string subject_hash(const char* subject);
vector<string> extract_message_ids(const char* str);

/* The database has these types:
 *
 * Threading:
 *   LU_THREADING 
 *   subject_hash 
 *   '\0'
 *   message_id
 *   (reply_to_hash*)
 * 
 * Summary:
 *  LU_SUMMARY
 *  message_id
 *  AUTHOR_EMAIL/AUTHOR_NAME/SUBJECT/MBOX
 *  ... if it has any of these then it must have all of them, in that order
 *  For all except the MBOX, one has just a string value.
 *  For Mbox:
 *   <64bit offset> <32bit length> <mailbox-as-string>
 *
 * Keyword:
 *  LU_KEYWORD
 *  keyword
 *  '\0'
 *  mid
 */

#endif
