/*  $Id: config.c,v 1.21 2002-07-21 19:26:08 terpstra Exp $
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
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "md5.h"
#include "avl.h"

#include "config.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef DMALLOC
# include <dmalloc.h>
#endif

/*------------------------------------------------ Private global vars */

static Kap		my_config_mbox_db;
static const char*	my_config_file;

/*------------------------------------------------ Public global vars */

Lu_Config_File*	lu_config_file    = 0;
Kap		lu_config_keyword = 0;

/*------------------------------------------------ Private helper methods */

#define	LU_INVALID	0xFFFFU

static int my_config_compare_lists(
	const void* pa, 
	const void* pb)
{
	const Lu_Config_List* a = pa;
	const Lu_Config_List* b = pb;
	
	return strcmp(a->name, b->name);
}

static int my_config_compare_mboxs(
	const void* pa, 
	const void* pb)
{
	const Lu_Config_Mbox* a = pa;
	const Lu_Config_Mbox* b = pb;
	
	/* not using subtraction b/c it's unsigned */
	return strcmp(a->name, b->name);
}

inline int my_config_compare_id(
	lu_word a,
	lu_word b)
{
	if (a < b) return -1;
	if (a > b) return 1;
	return 0;
}

AVL_DEFINE_INSERT(list, lu_word, LU_INVALID, Lu_Config_List, my_config_compare_id)
AVL_DEFINE_INSERT(mbox, lu_word, LU_INVALID, Lu_Config_Mbox, my_config_compare_id)

static int my_config_sort_mboxs(
	Lu_Config_List* list, 
	const char* cfg)
{
	Lu_Config_Mbox* scan;
	int ok = 0;
	
	/* Now, sort them */
	qsort(list->mbox, list->mboxs, sizeof(Lu_Config_Mbox), 
		&my_config_compare_mboxs);
	
	/* Make sure we have no clones */
	for (scan = list->mbox; scan != list->mbox + list->mboxs; scan++)
	{
		if (scan != list->mbox)
		{
			if (!strcmp(scan->name, (scan-1)->name))
			{
				fprintf(stderr, 
					_("%s: error: duplicated mbox id: %s (%s)\n"), 
					cfg, scan->name, list->name);
				ok = -1;
			}
		}
	}
	
	return ok;
}

static int my_config_sort_configuration(
	Lu_Config_File* file, 
	const char* cfg)
{
	Lu_Config_List* scan;
	int ok = 0;
	
	if (file->lists == 0)
	{
		fprintf(stderr, _("%s: error: no lists defined\n"), cfg);
		return -1;
	}
	
	/* Now, sort them */
	qsort(file->list, file->lists, sizeof(Lu_Config_List), 
		&my_config_compare_lists);
	
	/* Make sure we have no clones */
	for (scan = file->list; scan != file->list + file->lists; scan++)
	{
		my_config_sort_mboxs(scan, cfg);
		if (scan != file->list)
		{
			if (!strcmp(scan->name, (scan-1)->name))
			{
				fprintf(stderr, 
					_("%s: error: duplicated list id: %s\n"), 
					cfg, scan->name);
				ok = -1;
			}
		}
	}
	
	return ok;
}

static char* my_config_find_after(
	char* str, 
	const char* cfg, 
	int lines)
{
	char* k;
	
	while (*str)
	{
		if (*str++ == '=')
			break;
	}
	
	if (!*str)
	{
		fprintf(stderr, _("%s:%d: error: missing '='\n"), cfg, lines);
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
		fprintf(stderr, _("%s:%d: error: missing value after '='\n"), cfg, lines);
		return 0;
	}
	
	if (strlen(str) == 0)
	{
		fprintf(stderr, _("%s:%d: error: empty value\n"), cfg, lines);
		return 0;
	}
	
	for (k = str + strlen(str) - 1; k != str; k--)
		if (!isspace(*k))
			break;
	*++k = 0;
	
	if (strlen(str) == 0)
	{
		fprintf(stderr, _("%s:%d: error: empty value\n"), cfg, lines);
		return 0;
	}
	
	return str;
}

static long my_config_number_after(
	char* str,
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
			fprintf(stderr, 
				_("%s:%d: error: invalid value - must be positive integer\n"), 
				cfg, lines);
		}
	}
	
	return out;
}

static int my_config_sync_mbox(Lu_Config_File* file)
{
	char		key[80];
	char		key2[80];
	unsigned char	buf[sizeof(lu_word) + sizeof(off_t)];
	int		error, dir;
	off_t		tmp;
	ssize_t		len;
	lu_word		maxid;
	
	Lu_Config_List*	list;
	Lu_Config_Mbox*	mbox;
	
	strcpy(key, "");
	maxid = 0;
	
	/* Ok, here follows an ugly routine which will read in the mbox state.
	 * The database has two types of records:
	 * 	'list'      -> list id
	 *	'list:mbox' -> mbox id, mbox offset
	 * 
	 * The intention is to walk the database, record the information into
	 * our config block, note new config file entries, and abort if we
	 * ever see an mbox that has no configured entry.
	 * 
	 * Note: both the in memory data, and disk data are sorted.
	 */
	
	for (list = file->list; list != file->list + file->lists; list++)
	{
		dir = -1;
		
		/* Find the record which tells us about this list id
		 */
		while (dir < 0)
		{
			error = kap_btree_read_next(
				my_config_mbox_db, key, buf, &len);
			if (error && error != KAP_NOT_FOUND)
			{
				fprintf(stderr, _("Database failure mbox.btree: %s\n"),
					kap_strerror(error));
				return error;
			}
			
			if (error != KAP_NOT_FOUND)
				dir = strcmp(key, list->name);
			else
				dir = 1;
			
			/* Handle the case that there is stuff we have seen
			 * that is no longer configured. Only complain if an
			 * mbox went missing.
			 */
			if (dir < 0)
			{	/* skip past a list with no mboxs */
				if (strchr(key, ':'))
				{	/* is an mbox */
					fprintf(stderr, _("Lurker database references an unconfigured mbox: %s\n"),
						key);
					return -1;
				}
			}
		}
		
		if (dir > 0)
		{	/* new list */
			list->key = 0;
			strcpy(key, list->name); /* reseek to this point */
		}
		else if (dir == 0)
		{	/* This tells us the id of the key */
			assert (len == sizeof(lu_word));
	 		kap_decode_offset(&buf[0], &tmp, sizeof(lu_word));
			list->key = tmp;
			
			if (list->key > maxid) maxid = list->key;
		}
		
		/* Start reading mbox configuration */
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
		{
			error = kap_btree_read_next(
				my_config_mbox_db, key, buf, &len);
			if (error && error != KAP_NOT_FOUND)
			{
				fprintf(stderr, _("Database failure mbox.btree: %s\n"),
					kap_strerror(error));
				return error;
			}
			
			snprintf(&key2[0], sizeof(key2), "%s:%s", 
				list->name, mbox->name);
			
			if (error != KAP_NOT_FOUND)
				dir = strcmp(key, key2);
			else
				dir = 1;
			
			if (dir < 0)
			{
				fprintf(stderr, _("Lurker database references an unconfigured mbox: %s\n"),
					&key[0]);
			}
			else if (dir > 0)
			{	/* new mbox */
				mbox->key = 0;
				strcpy(key, key2); /* reseek to this point */
			}
			else
			{
				assert (len == sizeof(buf));
				
				kap_decode_offset(&buf[0], &tmp, sizeof(lu_word));
				mbox->key = tmp;
				
				kap_decode_offset(&buf[sizeof(lu_word)], &tmp, sizeof(off_t));
				mbox->length = tmp;
			}
		}
	}
	
	/* Everything we know about is now marked new or configured.
	 * However, check to see if there are yet more lists with possibly
	 * unreferenced mboxs
	 */
	while (1)
	{
		error = kap_btree_read_next(
			my_config_mbox_db, key, buf, &len);
		if (error && error != KAP_NOT_FOUND)
		{
			fprintf(stderr, _("Database failure mbox.btree: %s\n"),
				kap_strerror(error));
			return error;
		}
		
		if (error == KAP_NOT_FOUND) break;
		
		/* skip past a list with no mboxs */
		if (strchr(key, ':'))
		{	/* is an mbox */
			fprintf(stderr, _("Lurker database references an unconfigured mbox: %s\n"),
				key);
			return -1;
		}
	}
	
	/* Now, assign ids to all the new things */
	
	/** NOTE: We do _not_ write out information about mboxs.
	 *  This is done so that if they don't import anything, they can
	 *  be removed from the config file.
	 *  However, we must still assign them an id at this time.
	 */
	
	for (list = file->list; list != file->list + file->lists; list++)
	{
		if (list->key != 0) continue;
		
		list->key = ++maxid;
		kap_encode_offset(&buf[0], list->key, sizeof(lu_word));
		
		error = kap_btree_write(
			my_config_mbox_db, list->name, buf, sizeof(lu_word));
		
		if (error)
		{
			fprintf(stderr, _("Database failure mbox.btree: %s\n"),
				kap_strerror(error));
			return error;
		}
	}
	
	for (list = file->list; list != file->list + file->lists; list++)
	{
		maxid = 0;
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
			if (mbox->key > maxid) maxid = mbox->key;
		
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
		{
			if (mbox->key != 0) continue;
			mbox->key = ++maxid;
		}
	}
	
	return 0;
}

static int my_config_build_trees(Lu_Config_File* file)
{
	lu_word	list, mbox;
	int ok = 0;
	
	file->list_root = LU_INVALID;
	
	for (list = 0; list < file->lists; list++)
	{
		file->list_root = my_avl_list_insert(
			file->list, file->list_root, list);
			
		if (file->list_root == LU_INVALID)
		{
			fprintf(stderr, _("Duplicate list identifier: %d"),
				file->list[list].key);
			ok = -1;
		}
		
		file->list[list].mbox_root = LU_INVALID;
		for (mbox = 0; mbox < file->list[list].mboxs; mbox++)
		{
			file->list[list].mbox_root = my_avl_mbox_insert(
				file->list[list].mbox,
				file->list[list].mbox_root,
				mbox);
			
			if (file->list[list].mbox_root == LU_INVALID)
			{
				fprintf(stderr, _("Duplicate mbox identifier: %d"),
					file->list[list].mbox[mbox].key);
				ok = -1;
			}
		}
	}
	
	return ok;
}

static int my_config_open_mboxs(
	Lu_Config_File*	file)
{
	Lu_Config_List* list;
	Lu_Config_Mbox* mbox;
	
	int ok = 0;
	
	if (chdir(file->maildir) != 0)
	{
		perror(file->maildir);
		return -1;
	}
	
	for (list = file->list; list != file->list + file->lists; list++)
	{
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
		{
			mbox->fd = open(mbox->path, O_RDONLY);
			if (mbox->fd == -1)
			{
				fprintf(stderr, _("opening mailbox for %s: "), list->name);
				perror(mbox->path);
				ok = -1;
			}
			
			mbox->next_message = 0;
			mbox->locked       = 0;
			
			/* We have nothing mapped */
			memset(&mbox->msg, 0, sizeof(struct Lu_Config_Message));
		}
	}
	
	if (chdir(file->dbdir) != 0)
	{
		perror(file->dbdir);
		return -1;
	}
	
	return ok;
}

#define STR_LEN(key) \
if (!strcmp(&keyword[0], key)) \
{ \
	tmp = my_config_find_after(&line[0], my_config_file, lines); \
	file->strings += strlen(tmp) + 1; \
}

#define NUM_NOOP(key) \
if (!strcmp(&keyword[0], key)) \
{ \
	/* noop */ \
}

#define PRE_LIST_STR(key, var) \
if (!strcmp(&keyword[0], key)) \
{ \
	if (list != file->list) \
	{ \
		fprintf(stderr, _("%s:%d: error: %s in list scope\n"), \
			my_config_file, lines, key); \
		ok = -1; \
		continue; \
	} \
	\
	if (file->var) \
	{ \
		fprintf(stderr, _("%s:%d: warning: %s reset\n"), \
			my_config_file, lines, key); \
	} \
	\
	tmp = my_config_find_after(&line[0], my_config_file, lines); \
	if (tmp) \
	{ \
		file->var = string; \
		strcpy(string, tmp); \
		string += strlen(tmp) + 1; \
	} \
	else ok = -1; \
}

#define PRE_LIST_NUM(key, var) \
if (!strcmp(&keyword[0], key)) \
{ \
	if (list != file->list) \
	{ \
		fprintf(stderr, _("%s:%d: error: %s in list scope\n"), \
			my_config_file, lines, key); \
		ok = -1; \
		continue; \
	} \
	\
	if (file->var) \
	{ \
		fprintf(stderr, _("%s:%d: warning: %s reset\n"), \
			my_config_file, lines, key); \
	} \
	\
	file->var = my_config_number_after(&line[0], my_config_file, lines); \
	if (!file->var) ok = -1; \
}

#define AFT_LIST_STR(key, var) \
if (!strcmp(&keyword[0], key)) \
{ \
	if (list == file->list) \
	{ \
		fprintf(stderr, _("%s:%d: error: %s outside list scope\n"), \
			my_config_file, lines, key); \
		ok = -1; \
		continue; \
	} \
	\
	if ((list-1)->var) \
	{ \
		fprintf(stderr, _("%s:%d: warning: %s reset\n"), \
			my_config_file, lines, key); \
	} \
	\
	tmp = my_config_find_after(&line[0], my_config_file, lines); \
	if (tmp) \
	{ \
		(list-1)->var = string; \
		strcpy(string, tmp); \
		string += strlen(tmp) + 1; \
	} \
	else ok = -1; \
}

#define DEF_STR(key, var, val) \
if (!var) \
{ \
	fprintf(stderr, _("%s: warning: %s not declared - using: %s\n"), \
		my_config_file, key, val); \
	var = val; \
}

#define DEF_NUM(key, var, val) \
if (!var) \
{ \
	fprintf(stderr, _("%s: warning: %s not declared - using: %d\n"), \
		my_config_file, key, val); \
	var = val; \
}

static void my_config_free_config(Lu_Config_File* file)
{
	Lu_Config_Mbox* scan;
	
	if (file->mbox)
	{
		for (scan =  file->mbox; 
		     scan != file->mbox + file->mboxs; 
		     scan++)
		{
			if (scan->fd != -1) close(scan->fd);
		}
	}
	
	if (file->list)   free(file->list);
	if (file->mbox)   free(file->mbox);
	if (file->string) free(file->string);
	free(file);
}

static Lu_Config_File* my_config_load_config(void)
{
	Lu_Config_File* file;
	Lu_Config_List*	list;
	Lu_Config_Mbox* mbox;
	char*		string;
	
	int	ok = 0;
	int	lines = 0;
	char	line[1000];
	char	keyword[20];
	char	id[20];
	char*	s;
	char*	tmp;
	FILE*	c;
	
	file = calloc(1, sizeof(Lu_Config_File));
	if (!file)
	{
		fputs(_("Out of RAM loading config file\n"), stderr);
		return 0;
	}
	/* all zerod */
	
	if ((c = fopen(my_config_file, "r")) == 0)
	{
		perror(my_config_file);
		my_config_free_config(file);
		return 0;
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
			tmp = my_config_find_after(&line[0], my_config_file, lines);
			if (!tmp)
			{
				ok = -1;
				continue;
			}
			
			file->lists++;
			file->strings += strlen(tmp) + 1;
		}
		else if (!strcmp(&keyword[0], "mbox"))
		{
			if (sscanf(&line[0], "%19s %s = ", &keyword[0], &id[0]) != 2)
			{
				fprintf(stderr, 
					_("%s:%d: error: missing indentity tag for mbox\n"),
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
			
			file->mboxs++;
			file->strings += strlen(&id[0]) + 1;
			file->strings += strlen(tmp) + 1;
		}
		else STR_LEN ("dbdir")
		else STR_LEN ("maildir")
		else STR_LEN ("pidfile")
		else STR_LEN ("list_host")
		else STR_LEN ("admin_name")
		else STR_LEN ("admin_address")
		else NUM_NOOP("cache_cutoff")
		else NUM_NOOP("cache_files")
		else NUM_NOOP("cache_size")
		else NUM_NOOP("cache_search_ttl")
		else NUM_NOOP("cache_message_ttl")
		else NUM_NOOP("cache_index_ttl")
		else STR_LEN ("description")
		else STR_LEN ("address")
	}
	
	if (fseek(c, 0, SEEK_SET) != 0)
	{
		fprintf(stderr, _("%s:%d: error: can't reseek config file: %s\n"),
			my_config_file, lines, strerror(errno));
		my_config_free_config(file);
		return 0;
	}
	
	lines = 0;
	list   = file->list   = calloc(file->lists,   sizeof(Lu_Config_List));
	mbox   = file->mbox   = calloc(file->mboxs,   sizeof(Lu_Config_Mbox));
	string = file->string = calloc(file->strings, 1);
	
	if (!file->list || !file->mbox || !file->string)
	{
		fputs(_("%s: out of memory loading config\n"), stderr);
		my_config_free_config(file);
		return 0;
	}
	
	for (mbox =  file->mbox;
	     mbox != file->mbox + file->mboxs;
	     mbox++)
	{
		mbox->fd = -1;
	}
	
	mbox = file->mbox;
	
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
			tmp = my_config_find_after(&line[0], my_config_file, lines);
			if (!tmp)
			{
				ok = -1;
				continue;
			}
			
			list->name = string;
			list->mbox = mbox;
			strcpy(string, tmp);
			
			while (*string)
			{
				if (*string >= 'A' && *string <= 'Z')
					*string += 'a' - 'A';
				string++;
			}
			++string;
			
			list++;
		}
		else if (!strcmp(&keyword[0], "mbox"))
		{
			if (!list)
			{
				fprintf(stderr, 
					_("%s:%d: error: mbox outside list scope\n"),
					my_config_file, lines);
				ok = -1;
				continue;
			}
			
			if (sscanf(&line[0], "%19s %s = ", &keyword[0], &id[0]) != 2)
			{
				fprintf(stderr, 
					_("%s:%d: error: missing id tag for mbox\n"),
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
			
			mbox->name = string;
			strcpy(string, &id[0]);
			string += strlen(&id[0]) +1;
			
			mbox->path = string;
			strcpy(string, tmp);
			string += strlen(tmp) + 1;
			
			mbox->fd = -1;
			
			mbox++;
			(list-1)->mboxs++;
		}
		else PRE_LIST_STR("dbdir",             dbdir)
		else PRE_LIST_STR("maildir",           maildir)
		else PRE_LIST_STR("pidfile",           pidfile)
		else PRE_LIST_STR("list_host",         list_host)
		else PRE_LIST_STR("admin_name",        admin_name)
		else PRE_LIST_STR("admin_address",     admin_address)
		else PRE_LIST_NUM("cache_cutoff",      cache_cutoff)
		else PRE_LIST_NUM("cache_files",       cache_files)
		else PRE_LIST_NUM("cache_size",        cache_size)
		else PRE_LIST_NUM("cache_search_ttl",  cache_search_ttl)
		else PRE_LIST_NUM("cache_message_ttl", cache_message_ttl)
		else PRE_LIST_NUM("cache_index_ttl",   cache_index_ttl)
		else AFT_LIST_STR("description",       description)
		else AFT_LIST_STR("address",           address)
		else
		{
			fprintf(stderr, _("%s:%d: error: unknown keyword '%s'\n"),
				my_config_file, lines, &keyword[0]);
			ok = -1;
		}
	}
	
	fclose(c);
	
	assert (string <= file->string + file->strings);
	assert (mbox   <= file->mbox   + file->mboxs);
	assert (list   <= file->list   + file->lists);
	
	DEF_STR("dbdir",         file->dbdir,         DBDIR)
	DEF_STR("maildir",       file->maildir,       file->dbdir)
	DEF_STR("pidfile",       file->pidfile,       DEFAULT_PID_FILE)
	
	DEF_STR("list_host",     file->list_host,     "somewhere.org")
	DEF_STR("admin_name",    file->admin_name,    "unconfigured")
	DEF_STR("admin_address", file->admin_address, "nill@unconfigured.org")
	
	DEF_NUM("cache_cutoff",      file->cache_cutoff,      65536)
	DEF_NUM("cache_files",       file->cache_files,       4096)
	DEF_NUM("cache_size",        file->cache_size,        8388608)
	DEF_NUM("cache_search_ttl",  file->cache_search_ttl,  600)
	DEF_NUM("cache_message_ttl", file->cache_message_ttl, 600)
	DEF_NUM("cache_index_ttl",   file->cache_index_ttl,   604800)
	
	if (ok != 0)
	{
		my_config_free_config(file);
		return 0;
	}
	
	if (my_config_sort_configuration(file, my_config_file) != 0)
	{
		my_config_free_config(file);
		return 0;
	}
	
	return file;
}

/*------------------------------------------------ Public component methods */

int lu_config_init(const char* cfg)
{
	Lu_Config_File* file;
	my_config_file = cfg;
	
	if ((file = my_config_load_config()) == 0)
	{
		return -1;
	}
	
	lu_config_file = file;
	
	if (chdir(file->dbdir) != 0)
	{
		perror(file->dbdir);
		return -1;
	}
	
	return 0;
}

int lu_config_open(void)
{
	int error;
	
	if ((error = kap_create(&my_config_mbox_db, KAP_BTREE)) != 0 ||
	    (error = kap_btree_set_maxkeysize(my_config_mbox_db, 80)) != 0 ||
	    (error = kap_btree_set_leafsize  (my_config_mbox_db, sizeof(lu_word)+sizeof(off_t))) != 0 ||
	    (error = kap_btree_set_sectorsize(my_config_mbox_db, 4096)) != 0)
	{
		fprintf(stderr, _("Creating a kap database: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_create(&lu_config_keyword, KAP_FAST)) != 0)
	{
		fprintf(stderr, _("Creating kap database: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_open(my_config_mbox_db, ".", "mbox")) != 0)
	{
		fprintf(stderr, _("Opening kap database: mbox.btree: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_open(lu_config_keyword, ".", "keyword")) != 0)
	{
		fprintf(stderr, _("Opening kap database: keyword: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if (my_config_sync_mbox(lu_config_file) != 0)
		return -1;
	
	if (my_config_build_trees(lu_config_file) != 0)
		return -1;
	
	if (my_config_open_mboxs(lu_config_file) != 0)
		return -1;
	
	return 0;
}

int lu_config_sync(void)
{
	int error;
	int fail = 0;
	
	if ((error = kap_sync(my_config_mbox_db)) != 0)
	{
		syslog(LOG_ERR, _("Syncing kap database: mbox.btree: %s\n"),
			kap_strerror(error));
		fail = -1;
	}
	
	if ((error = kap_sync(lu_config_keyword)) != 0)
	{
		syslog(LOG_ERR, _("Syncing kap database: keyword: %s\n"),
			kap_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_config_close(void)
{
	int error;
	int fail = 0;
	
	if ((error = kap_destroy(my_config_mbox_db)) != 0)
	{
		syslog(LOG_ERR, _("Closing kap database: mbox.btree: %s\n"),
			kap_strerror(error));
		fail = -1;
	}
	
	if ((error = kap_destroy(lu_config_keyword)) != 0)
	{
		syslog(LOG_ERR, _("Destroying kap database: keyword: %s\n"),
			kap_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_config_quit(void)
{
	return 0;
}

/*------------------------------------------------- Public accessor methods */

Lu_Config_List* lu_config_find_listn(
	Lu_Config_File*	file,
	const char*	name)
{
	Lu_Config_List search;
	search.name = name;
	
	return bsearch(
		&search, 
		file->list, file->lists, sizeof(Lu_Config_List), 
		&my_config_compare_lists);
}

Lu_Config_List* lu_config_find_listi(
	Lu_Config_File*	file, 
	lu_word		id)
{
	int	tmp;
	lu_word	ptr = file->list_root;
	
	while (ptr != LU_INVALID)
	{
		tmp = my_config_compare_id(id, file->list[ptr].key);
		if      (tmp < 0) ptr = file->list[ptr].left;
		else if (tmp > 0) ptr = file->list[ptr].right;
		else break;
	}
	
	if (ptr == LU_INVALID) return 0;
	return &file->list[ptr];
}

Lu_Config_Mbox* lu_config_find_mboxn(
	Lu_Config_List* list, 
	const char* name)
{
	Lu_Config_Mbox search;
	search.name = name;
	
	return bsearch(
		&search, 
		list->mbox, list->mboxs, sizeof(Lu_Config_Mbox), 
		&my_config_compare_mboxs);
}

Lu_Config_Mbox* lu_config_find_mboxi(
	Lu_Config_List*	list, 
	lu_word		id)
{
	int	tmp;
	lu_word	ptr = list->mbox_root;
	
	while (ptr != LU_INVALID)
	{
		tmp = my_config_compare_id(id, list->mbox[ptr].key);
		if      (tmp < 0) ptr = list->mbox[ptr].left;
		else if (tmp > 0) ptr = list->mbox[ptr].right;
		else break;
	}
	
	if (ptr == LU_INVALID) return 0;
	return &list->mbox[ptr];
}

int lu_config_move_mbox_end(
	Lu_Config_Mbox* mbox, 
	Lu_Config_List* list, 
	off_t now)
{
	char		key[80];
	unsigned char	buf[sizeof(lu_word) + sizeof(off_t)];
	int		error;
	
	snprintf(&key[0], sizeof(key), "%s:%s", list->name, mbox->name);
	kap_encode_offset(&buf[0],               mbox->key, sizeof(lu_word));
	kap_encode_offset(&buf[sizeof(lu_word)], now,       sizeof(off_t));
	
	error = kap_btree_rewrite(
		my_config_mbox_db,
		&key[0],
		&buf[0],
		sizeof(lu_word)+sizeof(off_t));
	
	if (error != 0)
	{
		syslog(LOG_ERR, _("Could not record mbox eof: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	mbox->length = now;
	
	return 0;
}
