/*  $Id: config.h,v 1.2 2002-02-03 05:45:29 terpstra Exp $
 *  
 *  config.h - Knows how to load the config file
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

/*------------------------------------------------ Public types */            

typedef struct Lu_Config_Mbox_T
{
	lu_word	id;
	char*	path;
	int	fd;
	
	/* only used during load of config */
	struct Lu_Config_Mbox_T*	next;
} Lu_Config_Mbox;

typedef struct Lu_Config_List_T
{
	lu_word	id;
	
	char*	name;
	char*	address;
	char*	description;
	
	Lu_Config_Mbox*	mbox;
	int		mboxs;
	
	/* only used during load of config */
	Lu_Config_Mbox*			mbox_head;
	struct Lu_Config_List_T*	next;
} Lu_Config_List;

/*------------------------------------------------ Public global vars */

extern Lu_Config_List*	lu_config_list;
extern int		lu_config_lists;

extern char* lu_config_dbdir;
extern char* lu_config_wwwdir;
extern char* lu_config_pidfile;

extern DB_ENV* lu_config_env;

/*------------------------------------------------- Public component methods */

extern int lu_config_init(const char* cfg);
extern int lu_config_open();
extern int lu_config_sync();
extern int lu_config_close();
extern int lu_config_quit();

/*------------------------------------------------- Direct access methods */

/** Search the configuration for the mailing list which has the specified id
 */
extern Lu_Config_List* lu_config_find_list(
	lu_word id);

/** Search the configuration for the mbox which has the specified id.
 */
extern Lu_Config_Mbox* lu_config_find_mbox(
	Lu_Config_List* list, 
	lu_word id);

/** Move the end marker for a mailbox.
 */
extern int lu_config_move_mbox_end(
	Lu_Config_Mbox* mbox,
	Lu_Config_List* list,
	off_t now);
                        