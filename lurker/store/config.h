/*  $Id: config.h,v 1.16 2002-07-21 19:26:08 terpstra Exp $
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

#include <kap.h>

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
	const char*	name;
	const char*	path;
	int		fd;
	
	/* What is the state of the mailbox? If we have next_message 0 it
	 * that we need to check if the mailbox is longer than mbox_length.
	 * Otherwise, we decide what to import based solely on the next_message.
	 */
	time_t	next_message;
	off_t	length;
	
	int	locked;
	
	lu_word	key;
	lu_word	left;
	lu_word	right;
	lu_byte skew;
	
	struct Lu_Config_Message msg;
} Lu_Config_Mbox;

typedef struct Lu_Config_List_T
{
	const char*	name;
	const char*	address;
	const char*	description;
	
	lu_word	key;
	lu_word	left;
	lu_word	right;
	lu_byte skew;
	
	Lu_Config_Mbox*	mbox;
	int		mboxs;
	lu_word		mbox_root;
	
	/* Used in the expiry module for fast lookups */
	lu_word		cache_head;
} Lu_Config_List;

typedef struct Lu_Config_File_T
{
	Lu_Config_List*	list;
	int		lists;
	lu_word		list_root;
	
	char*		string;
	int		strings;
	
	Lu_Config_Mbox*	mbox;
	int		mboxs;

	const char*		dbdir;
	const char*		maildir;
	const char*		pidfile;

	const char*		list_host;
	const char*		admin_name;
	const char*		admin_address;
	
	long		cache_cutoff;
	long		cache_files;
	long		cache_size;
	
	time_t		cache_search_ttl;
	time_t		cache_message_ttl;
	time_t		cache_index_ttl;
} Lu_Config_File;

/*------------------------------------------------ Public global vars */

extern Lu_Config_File*	lu_config_file;
extern Kap		lu_config_keyword;

/*------------------------------------------------- Public component methods */

extern int lu_config_init (const char* cfg);
extern int lu_config_open (void);
extern int lu_config_sync (void);
extern int lu_config_close(void);
extern int lu_config_quit (void);

/*------------------------------------------------- Direct access methods */

/** Search the configuration for the mailing list which has the specified id
 */
extern Lu_Config_List* lu_config_find_listi(Lu_Config_File* file, lu_word id);
extern Lu_Config_List* lu_config_find_listn(Lu_Config_File* file, const char* name);

/** Search the configuration for the mbox which has the specified id.
 */
extern Lu_Config_Mbox* lu_config_find_mboxi(Lu_Config_List* list, lu_word id);
extern Lu_Config_Mbox* lu_config_find_mboxn(Lu_Config_List* list, const char* name);

/** Move the end marker for a mailbox.
 */
extern int lu_config_move_mbox_end(
	Lu_Config_Mbox* mbox,
	Lu_Config_List* list,
	off_t now);
