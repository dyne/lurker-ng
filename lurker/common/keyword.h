/*  $Id: keyword.h,v 1.3 2003-04-21 18:25:31 terpstra Exp $
 *  
 *  prefix.h - What to place in front of keywords we search for
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

/* fields indexed early */
#define LU_KEYWORD_MESSAGE_ID	"id:"	/* Message id */
#define LU_KEYWORD_REPLY_TO	"rt:"	/* Reply-to */
#define LU_KEYWORD_THREAD	"th:"	/* THread */

/* lu_import_message */
#define	LU_KEYWORD_LIST		"ml:"	/* Mailing List */
#define LU_KEYWORD_AUTHOR	"au:"	/* AUthor */
#define LU_KEYWORD_SUBJECT	"sb:"	/* SuBject */

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
