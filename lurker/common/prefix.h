/*  $Id: prefix.h,v 1.4 2002-02-10 21:00:18 terpstra Exp $
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

/* lu_import_message */
#define	LU_KEYWORD_LIST		"ml:"	/* Mailing List */
#define LU_KEYWORD_MBOX		"mb:"	/* Mail Box */
#define LU_KEYWORD_AUTHOR	"au:"	/* AUthor */
#define LU_KEYWORD_SUBJECT	"sb:"	/* SuBject */

/* date related words */
#define LU_KEYWORD_DAY_OF_MONTH	"dm:"	/* Day of Month */
#define LU_KEYWORD_MONTH	"mn:"	/* MoNth */
#define LU_KEYWORD_YEAR		"yr:"	/* YeaR */
#define LU_KEYWORD_WEEKDAY	"wd:"	/* WeekDay */
#define LU_KEYWORD_HOUR		"hr:"	/* HouR */

/* lu_reply_to_resolution */
#define LU_KEYWORD_MESSAGE_ID	"id:"	/* message ID */
#define LU_KEYWORD_REPLY_TO	"rt:"	/* Reply To id */

/* import processing */
#define LU_KEYWORD_WORD		""	/* body keywords */

/* The longest length a keyword can be */
#define LU_KEYWORD_LEN		80

/*------------------------------------------------- Public keyword methods */

/* Cleanup the contents of a message id 
 */
const char* lu_common_cleanup_id(
	const char* id);
