/*  $Id: breader.h,v 1.3 2002-02-10 06:34:24 terpstra Exp $
 *  
 *  breader.h - Knows how to use the abstracted read interface for buffered access
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
 
typedef int Lu_Breader_Handle;

/*------------------------------------------------- Public component methods */

extern int lu_breader_init();
extern int lu_breader_open();
extern int lu_breader_sync();
extern int lu_breader_close();
extern int lu_breader_quit();

/*-------------------------------------------------- Reader methods */

/* Open a handle to a keyword with which to read.
 */
extern Lu_Breader_Handle lu_breader_new(
	const char* keyword);

/* Find the index which contains the largest id <= id.
 * lu_common_minvalid if there is no such index.
 * 0 for success, -1 on error
 */
extern int lu_breader_offset(
	Lu_Breader_Handle h,
	message_id  id,
	message_id* index);

/* Read [index, index+count) into out.
 * 0 for success, -1 on error
 */
extern int lu_breader_read(
	Lu_Breader_Handle h,
	message_id index,
	message_id count,
	message_id* out);

/* Close the handle.
 */
extern void lu_breader_free(
	Lu_Breader_Handle h);
