/*  $Id: config.c,v 1.10 2002-05-11 19:24:59 terpstra Exp $
 *  
 *  config.c - Knows how to load the config file
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"

#include "config.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/*------------------------------------------------ Private types */

/** Tracking how much of an mbox is eaten - mbox.btree
 */
typedef struct My_Config_Mbox_Eat_Key_T
{
	lu_word		list;
	lu_word		mbox;
} My_Config_Mbox_Eat_Key;

typedef struct My_Config_Mbox_Eat_Value_T
{
	lu_addr		offset;
	
	/* The first 1k of the mbox to detect them being switched on us */
	char		front[1024];
} My_Config_Mbox_Eat_Value;

/*------------------------------------------------ Private global vars */

static DB*		my_config_mbox_db;
static const char*	my_config_file;

/*------------------------------------------------ Public global vars */

char*	lu_config_dbdir   = 0;
char*	lu_config_pidfile = 0;

char*	lu_config_list_host	= 0;
char*	lu_config_admin_name	= 0;
char*	lu_config_admin_address	= 0;

DB_ENV*		lu_config_env   = 0;
Lu_Config_List*	lu_config_list  = 0;
int		lu_config_lists = 0;

long	lu_config_cache_cutoff	= 0;
long	lu_config_cache_files	= 0;
long	lu_config_cache_size	= 0;

time_t	lu_config_cache_search_ttl	= 0;
time_t	lu_config_cache_message_ttl	= 0;
time_t	lu_config_cache_index_ttl	= 0;

/*------------------------------------------------ Private helper methods */

static int my_config_compare_lists(
	const void* pa, 
	const void* pb)
{
	const Lu_Config_List* a = pa;
	const Lu_Config_List* b = pb;
	
	/* not using subtraction b/c it's unsigned */
	if (a->id < b->id) return -1;
	if (a->id > b->id) return 1;
	return 0;
}

static int my_config_compare_mboxs(
	const void* pa, 
	const void* pb)
{
	const Lu_Config_Mbox* a = pa;
	const Lu_Config_Mbox* b = pb;
	
	/* not using subtraction b/c it's unsigned */
	if (a->id < b->id) return -1;
	if (a->id > b->id) return 1;
	return 0;
}

static int my_config_sort_mboxs(
	Lu_Config_List* list, 
	const char* cfg)
{
	int		ok = 0;
	Lu_Config_Mbox*	scan;
	Lu_Config_Mbox*	out;
	
	list->mboxs = 0;
	for (scan = list->mbox_head; scan; scan = scan->next)
		list->mboxs++;
	
	if (list->mboxs == 0)
	{
		list->mbox = 0;
		return 0;
	}
	
	/* Get consolidated memory */
	list->mbox = malloc(sizeof(Lu_Config_Mbox) * list->mboxs);
	if (!list->mbox)
	{
		fprintf(stderr, "%s: error: out of memory\n", cfg);
		return -1;
	}
	
	/* Move them into place */
	for (out = list->mbox, scan = list->mbox_head; scan; scan = scan->next, out++)
		memcpy(out, scan, sizeof(Lu_Config_Mbox));
	
	/* Free the excess */
	while (list->mbox_head)
	{
		out = list->mbox_head->next;
		free (list->mbox_head);
		list->mbox_head = out;
	}
	
	/* Now, sort them */
	qsort(list->mbox, list->mboxs, sizeof(Lu_Config_Mbox), 
		&my_config_compare_mboxs);
	
	/* Make sure we have no clones */
	for (scan = list->mbox; scan != list->mbox + list->mboxs; scan++)
	{
		if (scan != list->mbox)
		{
			if (scan->id == (scan-1)->id)
			{
				fprintf(stderr, "%s: error: "
					"duplicated mbox id: %d (%s)\n", 
					cfg, scan->id, list->name);
				ok = -1;
			}
		}
	}
	
	return ok;
}

static int my_config_sort_configuration(
	Lu_Config_List* top, 
	const char* cfg)
{
	int	ok = 0;
	
	Lu_Config_List*	scan;
	Lu_Config_List*	out;
	
	lu_config_lists = 0;
	for (scan = top; scan; scan = scan->next)
	{
		if (my_config_sort_mboxs(scan, cfg) != 0)
			ok = -1;
		
		lu_config_lists++;
	}
	
	if (lu_config_lists == 0)
	{
		fprintf(stderr, "%s: error: no lists defined\n", cfg);
		return -1;
	}
	
	/* Consolidate them in memory */
	lu_config_list = malloc(sizeof(Lu_Config_List) * lu_config_lists);
	if (!lu_config_list)
	{
		fprintf(stderr, "%s: error: out of memory\n", cfg);
		return -1;
	}
	
	/* Move them into place */
	for (out = lu_config_list, scan = top; scan; scan = scan->next, out++)
		memcpy(out, scan, sizeof(Lu_Config_List));
	
	/* Now, sort them */
	qsort(lu_config_list, lu_config_lists, sizeof(Lu_Config_List), 
		&my_config_compare_lists);
	
	/* Make sure we have no clones */
	for (scan = lu_config_list; scan != lu_config_list + lu_config_lists; scan++)
	{
		if (scan != lu_config_list)
		{
			if (scan->id == (scan-1)->id)
			{
				fprintf(stderr, "%s: error: "
					"duplicated list id: %d\n", 
					cfg, scan->id);
				ok = -1;
			}
		}
	}
	
	return ok;
}

static char* my_config_find_after(
	const char* str, 
	const char* cfg, 
	int lines)
{
	char* out;
	char* k;
	
	while (*str)
	{
		if (*str++ == '=')
			break;
	}
	
	if (!*str)
	{
		fprintf(stderr, "%s:%d: error: missing '='\n", cfg, lines);
		return 0;
	}
	
	while (*str)
	{
		if (!isspace(*str))
			break;
		str++;
	}
	
	if (!*str)
	{
		fprintf(stderr, "%s:%d: error: missing value after '='\n", cfg, lines);
		return 0;
	}
	
	out = strdup(str);
	if (!out)
	{
		fprintf(stderr, "%s:%d: error: out of memory\n", cfg, lines);
		return 0;
	}
	
	if (strlen(out) == 0)
	{
		fprintf(stderr, "%s:%d: error: empty value\n", cfg, lines);
		free(out);
		return 0;
	}
	
	for (k = out + strlen(out) - 1; k != out; k--)
		if (!isspace(*k))
			break;
	*++k = 0;
	
	if (strlen(out) == 0)
	{
		fprintf(stderr, "%s:%d: error: empty value\n", cfg, lines);
		free(out);
		return 0;
	}
	
	return out;
}

static long my_config_number_after(
	const char* str,
	const char* cfg,
	int lines)
{
	int   out = 0;
	char* num = my_config_find_after(str, cfg, lines);
	
	if (num)
	{
		out = atol(num);
		if (!out)
		{
			fprintf(stderr, "%s:%d: error: invalid value - must be positive integer\n", cfg, lines);
		}
		free(num);
	}
	
	return out;
}

static int my_config_open_mboxs()
{
	Lu_Config_List* list;
	Lu_Config_Mbox* mbox;
	
	int ok = 0;
	
	for (list = lu_config_list; list != lu_config_list + lu_config_lists; list++)
	{
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
		{
			mbox->fd = open(mbox->path, O_RDONLY);
			if (mbox->fd == -1)
			{
				perror(mbox->path);
				ok = -1;
			}
			
			mbox->next_message = 0;
			mbox->locked       = 0;
			
			/* We have nothing mapped */
			memset(&mbox->msg, 0, sizeof(struct Lu_Config_Message));
		}
	}
	
	return ok;
}

#define PRE_LIST_STR(key, var) \
if (!strcmp(&keyword[0], key)) \
{ \
	if (what) \
	{ \
		fprintf(stderr, "%s:%d: error: %s in list scope\n", \
			my_config_file, lines, key); \
		ok = -1; \
		continue; \
	} \
	\
	if (var) \
	{ \
		fprintf(stderr, "%s:%d: warning: %s reset\n", \
			my_config_file, lines, key); \
		free(var); \
	} \
	\
	var = my_config_find_after(&line[0], my_config_file, lines); \
	if (!var) ok = -1; \
}

#define PRE_LIST_NUM(key, var) \
if (!strcmp(&keyword[0], key)) \
{ \
	if (what) \
	{ \
		fprintf(stderr, "%s:%d: error: %s in list scope\n", \
			my_config_file, lines, key); \
		ok = -1; \
		continue; \
	} \
	\
	if (var) \
	{ \
		fprintf(stderr, "%s:%d: warning: %s reset\n", \
			my_config_file, lines, key); \
	} \
	\
	var = my_config_number_after(&line[0], my_config_file, lines); \
	if (!var) ok = -1; \
}

#define AFT_LIST_STR(key, var) \
if (!strcmp(&keyword[0], key)) \
{ \
	if (!what) \
	{ \
		fprintf(stderr, "%s:%d: error: %s outside list scope\n", \
			my_config_file, lines, key); \
		ok = -1; \
		continue; \
	} \
	\
	if (what->var) \
	{ \
		fprintf(stderr, "%s:%d: warning: %s reset\n", \
			my_config_file, lines, key); \
		free(what->var); \
	} \
	\
	what->var = my_config_find_after(&line[0], my_config_file, lines); \
	if (!what->var) ok = -1; \
}

#define DEF_STR(key, var, val) \
if (!var) \
{ \
	fprintf(stderr, "%s: warning: %s not declared - " \
		"using: %s\n", my_config_file, key, val); \
	var = strdup(DEFAULT_PID_FILE); \
}

#define DEF_NUM(key, var, val) \
if (!var) \
{ \
	fprintf(stderr, "%s: warning: %s not declared - " \
		"using: %d\n", my_config_file, key, val); \
	var = val; \
}

static int my_config_load_config()
{
	Lu_Config_List** target_list;
	Lu_Config_Mbox** target_mbox;
	Lu_Config_List*  what;
	Lu_Config_List*  top;
	
	int	ok = 0;
	int	lines = 0;
	char	line[1000];
	char	keyword[20];
	char*	s;
	char*	tmp;
	int	i;
	FILE*	c;
	
	top  = 0;
	what = 0;
	
	target_list = &top;
	target_mbox = 0;
	
	if ((c = fopen(my_config_file, "r")) == 0)
	{
		perror(my_config_file);
		return -1;
	}
	
	while (!feof(c))
	{
		lines++;
		if (fgets(&line[0], sizeof(line), c) == 0)
			break;
		
		for (s = &line[0]; *s; s++)
		{
			if (*s == '#')
			{
				*s = 0;
				break;
			}
		}
		
		if (sscanf(&line[0], "%19s", &keyword[0]) != 1)
		{
			continue;
		}
		
		if (!strcmp(&keyword[0], "list"))
		{
			if (sscanf(&line[0], "%19s %d = ", &keyword[0], &i) != 2)
			{
				fprintf(stderr, "%s:%d: error: "
					"missing numeric tag for list\n",
					my_config_file, lines);
				ok = -1;
				continue;
			}
			
			if (i < 0 || i >= 0xFFFFUL)
			{
				fprintf(stderr, "%s:%d: error: "
					"id is not within bounds\n",
					my_config_file, lines);
				ok = -1;
				continue;
			}
			
			tmp = my_config_find_after(&line[0], my_config_file, lines);
			if (!tmp)
			{
				ok = -1;
				continue;
			}
			
			what = *target_list = malloc(sizeof(Lu_Config_List));
			if (!*target_list)
			{
				fprintf(stderr, "%s:%d: error: "
					"out of memory\n",
					my_config_file, lines);
				free(tmp);
				ok = -1;
				continue;
			}
			
			what->name = tmp;
			what->id   = i;
			
			what->address     = 0;
			what->description = 0;
			what->mbox_head   = 0;
			what->next        = 0;
			what->cache_head  = 0;
			
			target_mbox = &what->mbox_head;
			target_list = &what->next;
		}
		else if (!strcmp(&keyword[0], "mbox"))
		{
			if (!what)
			{
				fprintf(stderr, "%s:%d: error: "
					"mbox outside list scope\n",
					my_config_file, lines);
				ok = -1;
				continue;
			}
			
			if (i < 0 || i >= 0xFFFFUL)
			{
				fprintf(stderr, "%s:%d: error: "
					"id is not within bounds\n",
					my_config_file, lines);
				ok = -1;
				continue;
			}
			
			if (sscanf(&line[0], "%19s %d = ", &keyword[0], &i) != 2)
			{
				fprintf(stderr, "%s:%d: error: "
					"missing numeric tag for mbox\n",
					my_config_file, lines);
				ok = -1;
				continue;
			}
			
			tmp = my_config_find_after(
				&line[0], my_config_file, lines);
			if (!tmp)
			{
				ok = -1;
				continue;
			}
			
			*target_mbox = malloc(sizeof(Lu_Config_Mbox));
			if (!*target_mbox)
			{
				fprintf(stderr, "%s:%d: error: "
					"out of memory\n",
					my_config_file, lines);
				free(tmp);
				ok = -1;
				continue;
			}
			
			(*target_mbox)->id   = i;
			(*target_mbox)->path = tmp;
			(*target_mbox)->fd   = -1;
			(*target_mbox)->next = 0;
			
			target_mbox = &(*target_mbox)->next;
		}
		else PRE_LIST_STR("dbdir",             lu_config_dbdir)
		else PRE_LIST_STR("pidfile",           lu_config_pidfile)
		else PRE_LIST_STR("list_host",         lu_config_list_host)
		else PRE_LIST_STR("admin_name",        lu_config_admin_name)
		else PRE_LIST_STR("admin_address",     lu_config_admin_address)
		else PRE_LIST_NUM("cache_cutoff",      lu_config_cache_cutoff)
		else PRE_LIST_NUM("cache_files",       lu_config_cache_files)
		else PRE_LIST_NUM("cache_size",        lu_config_cache_size)
		else PRE_LIST_NUM("cache_search_ttl",  lu_config_cache_search_ttl)
		else PRE_LIST_NUM("cache_message_ttl", lu_config_cache_message_ttl)
		else PRE_LIST_NUM("cache_index_ttl",   lu_config_cache_index_ttl)
		else AFT_LIST_STR("description",       description)
		else AFT_LIST_STR("address",           address)
		else
		{
			fprintf(stderr, "%s:%d: error: unknown keyword '%s'\n",
				my_config_file, lines, &keyword[0]);
			ok = -1;
		}
	}
	
	fclose(c);
	
	DEF_STR("dbdir",         lu_config_dbdir,         DBDIR)
	DEF_STR("pidfile",       lu_config_pidfile,       DEFAULT_PID_FILE)
	DEF_STR("list_host",     lu_config_list_host,     "somewhere.org")
	DEF_STR("admin_name",    lu_config_admin_name,    "unconfigured")
	DEF_STR("admin_address", lu_config_admin_address, "nill@unconfigured.org")
	
	DEF_NUM("cache_cutoff",      lu_config_cache_cutoff,      65536)
	DEF_NUM("cache_files",       lu_config_cache_files,       4096)
	DEF_NUM("cache_size",        lu_config_cache_size,        8388608)
	DEF_NUM("cache_search_ttl",  lu_config_cache_search_ttl,  600)
	DEF_NUM("cache_message_ttl", lu_config_cache_message_ttl, 600)
	DEF_NUM("cache_index_ttl",   lu_config_cache_index_ttl,   604800)
	
	if (ok != 0)
	{
		return ok;
	}
	
	if (my_config_sort_configuration(top, my_config_file) != 0)
	{
		return -1;
	}
	
	if (my_config_open_mboxs() != 0)
	{
		return -1;
	}
	
	return 0;
}

int my_config_sync_mbox()
{
	Lu_Config_List*	list;
	Lu_Config_Mbox*	mbox;
	
	My_Config_Mbox_Eat_Key	key;
	My_Config_Mbox_Eat_Value	val;
	
	DBT	dkey;
	DBT	dval;
	
	int error;
	int got;
	
	char buf[1024];
	
	for (list = lu_config_list; list != lu_config_list + lu_config_lists; list++)
	{
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
		{
			memset(&dkey, 0, sizeof(DBT));
			memset(&dval, 0, sizeof(DBT));
			
			dkey.data = &key;
			dkey.size = sizeof(key);
			
			dval.data = &val;
			dval.ulen = sizeof(val);
			dval.flags = DB_DBT_USERMEM;
			
			key.list = list->id;
			key.mbox = mbox->id;
			
			error = my_config_mbox_db->get(
				my_config_mbox_db, 0, &dkey, &dval, 0);
			
			mbox->length = 0;
			
			if (error == 0)
			{
				got = read(mbox->fd, &buf[0], sizeof(buf));
				if (got < 0)
				{
					fprintf(stderr, "Couldn't read mbox %s: %s\n",
							mbox->path, strerror(errno));
					return -1;
				}
				
				if (got < sizeof(buf) && got < val.offset)
				{	/* They switched mboxs on us -- too small */
					fprintf(stderr, "Mbox id does not match prior version %s\n",
						mbox->path);
					return -1;
				}
				
				if (val.offset < got)
				{	/* Don't compare more than there was */
					got = val.offset;
				}
				
				if (memcmp(&val.front[0], &buf[0], got))
				{	/* The contents are changed */
					fprintf(stderr, "Mbox id does not match prior version %s\n",
						mbox->path);
					return -1;
				}
				
				mbox->length = val.offset;
			}
			else if (error != DB_NOTFOUND)
			{
				fprintf(stderr, "Database failure mbox.btree: %s\n",
					db_strerror(error));
			}
		}
	}
	
	return 0;
}
/*------------------------------------------------ Public component methods */

int lu_config_init(const char* cfg)
{
	my_config_file = cfg;
	
	if (my_config_load_config() != 0)
	{
		return -1;
	}
	
	if (chdir(lu_config_dbdir) != 0)
	{
		perror(lu_config_dbdir);
		return -1;
	}
	
	return 0;
}

int lu_config_open()
{
	int error;
	
	if ((error = db_env_create(&lu_config_env, 0)) != 0)
	{
		fprintf(stderr, "Creating db3 environment: %s\n", 
			db_strerror(error));
		return -1;
	}

#ifdef DEBUG
	lu_config_env->set_errfile(lu_config_env, stderr);
#endif
	
	/* Give ourselves a special and unique shared memory key */
	if ((error = lu_config_env->set_shm_key(
		lu_config_env, 0x4E12DA03UL)) != 0)
	{
		fprintf(stderr, "Setting db3 shared memory key: %s\n", 
			db_strerror(error));
		return -1;
	}
	
	/* Open the environment */
	if ((error = lu_config_env->open(lu_config_env, lu_config_dbdir, 
		DB_INIT_MPOOL |	DB_RECOVER |
		DB_CREATE | DB_SYSTEM_MEM,
		LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening the db3 environment: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&my_config_mbox_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_config_mbox_db->open(my_config_mbox_db, "mbox.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: mbox.btree: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if (my_config_sync_mbox() != 0)
	{
		return -1;
	}
	
	return 0;
}

int lu_config_sync()
{
	int error;
	int fail = 0;
	
	if ((error = my_config_mbox_db->sync(my_config_mbox_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: mbox.btree: %s\n",
			db_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_config_close()
{
	int error;
	int fail = 0;
	
	if ((error = my_config_mbox_db->close(my_config_mbox_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: mbox.btree: %s\n",
			db_strerror(error));
		fail = -1;
	}
	
	if ((error = lu_config_env->close(lu_config_env, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 environment: %s\n",
			db_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_config_quit()
{
	return 0;
}

/*------------------------------------------------- Public accessor methods */

Lu_Config_List* lu_config_find_list(
	lu_word id)
{
	Lu_Config_List search;
	search.id = id;
	
	return bsearch(
		&search, 
		lu_config_list, lu_config_lists, sizeof(Lu_Config_List), 
		&my_config_compare_lists);
}

Lu_Config_Mbox* lu_config_find_mbox(
	Lu_Config_List* list, 
	lu_word id)
{
	Lu_Config_Mbox search;
	search.id = id;
	
	return bsearch(
		&search, 
		list->mbox, list->mboxs, sizeof(Lu_Config_Mbox), 
		&my_config_compare_mboxs);
}

int lu_config_move_mbox_end(
	Lu_Config_Mbox* mbox, 
	Lu_Config_List* list, 
	off_t now)
{
	My_Config_Mbox_Eat_Key		key;
	My_Config_Mbox_Eat_Value	val;
	
	DBT	dkey;
	DBT	dval;
	
	int error;
	
	memset(&dkey, 0, sizeof(DBT));
	memset(&dval, 0, sizeof(DBT));
	
	dkey.data = &key;
	dkey.size = sizeof(key);
	
	dval.data = &val;
	dval.size = sizeof(val);
	
	key.list = list->id;
	key.mbox = mbox->id;
	
	if (lseek(mbox->fd, 0, SEEK_SET) != 0) return -1;
	if (read(mbox->fd, &val.front[0], sizeof(val.front)) <= 0) return -1;
	
	val.offset = now;
	
	error = my_config_mbox_db->put(my_config_mbox_db, 0, &dkey, &dval, 0);
	if (error != 0)
		return -1;
	
	mbox->length = now;
	
	return 0;
}
