/*  $Id: prefix.h,v 1.1 2002-02-03 05:55:14 terpstra Exp $
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
#define	LU_KEYWORD_LIST		'l'
#define LU_KEYWORD_MBOX		'b'
#define LU_KEYWORD_AUTHOR	'a'
#define LU_KEYWORD_SUBJECT	's'

/* date related words */
#define LU_KEYWORD_DAY_OF_MONTH	'd'
#define LU_KEYWORD_MONTH	'm'
#define LU_KEYWORD_YEAR		'y'
#define LU_KEYWORD_WEEKDAY	'x'
#define LU_KEYWORD_HOUR		'h'

/* lu_reply_to_resolution */
#define LU_KEYWORD_MESSAGE_ID	'i'
#define LU_KEYWORD_REPLY_TO	'r'

/* import processing */
#define LU_KEYWORD_WORD		'w'
