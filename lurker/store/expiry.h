/*  $Id: expiry.h,v 1.1 2002-05-03 05:21:27 terpstra Exp $
 *  
 *  expiry.h - Record when pages should be destroyed
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

extern int lu_expiry_init (void);
extern int lu_expiry_open (void);
extern int lu_expiry_sync (void);
extern int lu_expiry_close(void);
extern int lu_expiry_quit (void);

/*------------------------------------------------- Public expiry methods */

#define LU_EXPIRY_NO_LIST       0xFFFFUL

/** Record the creation of a file for later removal.
 *  Return: 1 - don't cache, 0 - do cache
 */
extern int lu_expiry_record_file(
	const char*	path,
	long		size,
	time_t		ttl,
	lu_word		list_watch);

/** Report to the expiry subsystem that a message was imported.
 */
extern int lu_expiry_notice_import(
	lu_word	list);
