/*  $Id: Keys.h,v 1.12 2004-08-20 14:09:20 terpstra Exp $
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

#include <string>
#include <vector>

using std::string;
using std::vector;

// These must have length 1
#define LU_THREADING	"t"
#define LU_KEYWORD	"k"
#define LU_SUMMARY	"s"
#define LU_CACHE	"c"
#define LU_NEW_TOPICS	"n"

#define LU_MESSAGE_AUTHOR_EMAIL	'e'
#define LU_MESSAGE_AUTHOR_NAME	'n'
#define LU_MESSAGE_SUBJECT	's'
#define LU_MESSAGE_MBOX		'x'

/* special keywords */
#define LU_KEYWORD_DELETED	"id:deleted"	/* a deleted message */

/* fields indexed early */
#define LU_KEYWORD_EVERYTHING	"id:any"	/* empty search */
#define LU_KEYWORD_MESSAGE_ID	"id:"		/* Message id */
#define LU_KEYWORD_REPLY_TO	"rt:"		/* Reply-to */
#define LU_KEYWORD_THREAD	"th:"		/* THread */

/* lu_import_message */
#define	LU_KEYWORD_LIST		"ml:"	/* Mailing List */
#define LU_KEYWORD_AUTHOR	"au:"	/* AUthor */
#define LU_KEYWORD_SUBJECT	"sb:"	/* SuBject */

/* magic automatic keywords */
#define LU_KEYWORD_GROUP	"gr:"	/* Mailing group */
#define LU_KEYWORD_LANGUAGE	"lang:"	/* Language */

/* import processing */
#define LU_KEYWORD_WORD		""	/* body keywords */

/* The longest length a keyword can be */
#define LU_KEYWORD_LEN		80

/* Analyze the string for keywords. */
extern int my_keyword_digest_string(
	const char* buf,
	int         len,
	const char* prefix,
	int       (*writefn)(const char* keyword, void* arg),
	void*       arg,
	int         do_div);

const char* skipSubjectStart(const char* subject);
string subject_hash(const char* subject);
vector<string> extract_message_ids(const char* str);

/* The database has these types:
 *
 * Threading:
 *   LU_THREADING 
 *   subject_hash 
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
 *   <mailbox-as-string> '\0' <64bit offset> <32bit length>
 *
 * Keyword:
 *  LU_KEYWORD
 *  keyword
 *  '\0'
 *  mid
 *
 * Cache cleaning:
 *  LU_CACHE
 *  import timestamp as 4 bytes in bigendian order
 *  mid
 * 
 * New topics fields:
 *  LU_NEW_TOPICS
 *  listid '\0'
 *  timestamp
 *  threadhash
 */

#endif
