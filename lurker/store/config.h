/*  $Id: config.h,v 1.12 2002-05-04 03:41:21 terpstra Exp $
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
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>

/*------------------------------------------------ Public types */            

struct Lu_Config_Mmap
{
	/* mmap details - base = 0 determines if mapped */
	
	off_t	off;	/* locate of mmap in file */
	void*	base;	/* memory of the mmap */
	size_t	size;	/* size of the mmap */
};

struct Lu_Config_Message
{
	struct Lu_Config_Mmap	map;
	
	off_t	headers;
	off_t	body;
	off_t	end;
};

typedef struct Lu_Config_Mbox_T
{
	lu_word	id;
	char*	path;
	int	fd;
	
	/* What is the state of the mailbox? If we have next_message 0 it
	 * that we need to check if the mailbox is longer than mbox_length.
	 * Otherwise, we decide what to import based solely on the next_message.
	 */
	time_t	next_message;
	off_t	length;
	
	int	locked;
	
	struct Lu_Config_Message msg;
	
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
	
	/* Used in the expiry module for fast lookups */
	lu_word		cache_head;
	
	/* only used during load of config */
	Lu_Config_Mbox*			mbox_head;
	struct Lu_Config_List_T*	next;
} Lu_Config_List;

/*------------------------------------------------ Public global vars */

extern Lu_Config_List*	lu_config_list;
extern int		lu_config_lists;

extern char* lu_config_dbdir;
extern char* lu_config_pidfile;

extern char* lu_config_list_host;
extern char* lu_config_admin_name;
extern char* lu_config_admin_address;

extern DB_ENV* lu_config_env;

extern long	lu_config_cache_cutoff;
extern long	lu_config_cache_files;
extern long	lu_config_cache_size;

extern time_t	lu_config_cache_search_ttl;
extern time_t	lu_config_cache_message_ttl;
extern time_t	lu_config_cache_index_ttl;

/*------------------------------------------------- Public component methods */

extern int lu_config_init (const char* cfg);
extern int lu_config_open (void);
extern int lu_config_sync (void);
extern int lu_config_close(void);
extern int lu_config_quit (void);

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
