/*  $Id: service.h,v 1.3 2002-02-10 21:50:38 terpstra Exp $
 *  
 *  service.h - Knows how to deal with request from the cgi
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

#include <st.h>

/*------------------------------------------------- Public component methods */

extern int lu_service_init (void);
extern int lu_service_open (void);
extern int lu_service_sync (void);
extern int lu_service_close(void);
extern int lu_service_quit (void);

/*------------------------------------------------- Request handler */

extern int lu_service_connection(st_netfd_t fd);
