/*  $Id: globals.h,v 1.8 2002-02-03 03:10:53 terpstra Exp $
 *  
 *  globals.h - the global variables and methods for the daemon
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

#include <db.h>

extern char*	lu_dbdir;
extern char*	lu_wwwdir;
extern char*	lu_pidfile;

/** This loads the configuration file into the appropriate globals.
 *  It also opens all the mboxs for the tail functionality.
 */
extern int lu_load_config(const char* cfg);

/** This opens all the databases that we track data with.
 */
extern int lu_open_db();

extern int lu_sync_db();
extern int lu_close_db();

/** Double check that the mboxs haven't been switched. Then move the read
 *  heads to the right locations.
 */
extern int lu_sync_mbox();

/** This pushes all the keywords

