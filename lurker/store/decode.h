/*  $Id: decode.h,v 1.2 2002-07-11 20:30:10 terpstra Exp $
 *  
 *  common.h - common definitions and types for all tools
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

/*------------------------------------------------ Public helpers */

/* Decode a message hreader into unescaped utf-8.
 * This deals with =?charset?coding?str?= escaping as well as broken mail
 * clients which mistakenly use the same charset they used for the body.
 */
void lu_decode_header(
	const char*	r,
	char*		out,
	size_t		outlen,
	const char*	default_coding);

const char* lu_decode_charset(
	const char*	charset);

#define MAX_MESSAGE_ID	80

/* Cleanup the contents of a message id 
 */
const char* lu_decode_id(
	const char* id);
