/*  $Id: config.c,v 1.1.1.1 2002-01-21 00:03:07 terpstra Exp $
 *  
 *  config.c - load the config file
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

#include "records.h"
#include "globals.h"
#include "io.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>

char*	lu_dbdir   = 0;
char*	lu_wwwdir  = 0;
char*	lu_pidfile = 0;

List*	lu_list  = 0;
int	lu_lists = 0;

static int compare_lists(const void* pa, const void* pb)
{
	const List* a = pa;
	const List* b = pb;
	
	/* not using subtraction b/c it's unsigned */
	if (a->id < b->id) return -1;
	if (a->id > b->id) return 1;
	return 0;
}

static int compare_mboxs(const void* pa, const void* pb)
{
	const Mbox* a = pa;
	const Mbox* b = pb;
	
	/* not using subtraction b/c it's unsigned */
	if (a->id < b->id) return -1;
	if (a->id > b->id) return 1;
	return 0;
}

List* lu_find_list(lu_word id)
{
	List search;
	search.id = id;
	
	return bsearch(
		&search, 
		lu_list, lu_lists, sizeof(List), 
		&compare_lists);
}

Mbox* lu_find_mbox(List* list, lu_word id)
{
	Mbox search;
	search.id = id;
	
	return bsearch(
		&search, 
		list->mbox, list->mboxs, sizeof(Mbox), 
		&compare_mboxs);
}

static int sort_mboxs(List* list, const char* cfg)
{
	int	ok = 0;
	Mbox*	scan;
	Mbox*	out;
	
	list->mboxs = 0;
	for (scan = list->mbox_head; scan; scan = scan->next)
		list->mboxs++;
	
	if (list->mboxs == 0)
	{
		list->mbox = 0;
		return 0;
	}
	
	/* Get consolidated memory */
	list->mbox = malloc(sizeof(Mbox) * list->mboxs);
	if (!list->mbox)
	{
		fprintf(stderr, "%s: error: out of memory\n", cfg);
		return -1;
	}
	
	/* Move them into place */
	for (out = list->mbox, scan = list->mbox_head; scan; scan = scan->next, out++)
		memcpy(out, scan, sizeof(Mbox));
	
	/* Free the excess */
	while (list->mbox_head)
	{
		out = list->mbox_head->next;
		free (list->mbox_head);
		list->mbox_head = out;
	}
	
	/* Now, sort them */
	qsort(list->mbox, list->mboxs, sizeof(Mbox), &compare_mboxs);
	
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

static int sort_configuration(List* top, const char* cfg)
{
	int	ok = 0;
	
	List*	scan;
	List*	out;
	
	lu_lists = 0;
	for (scan = top; scan; scan = scan->next)
	{
		if (sort_mboxs(scan, cfg) != 0)
			ok = -1;
		
		lu_lists++;
	}
	
	if (lu_lists == 0)
	{
		fprintf(stderr, "%s: error: no lists defined\n", cfg);
		return -1;
	}
	
	/* Consolidate them in memory */
	lu_list = malloc(sizeof(List) * lu_lists);
	if (!lu_list)
	{
		fprintf(stderr, "%s: error: out of memory\n", cfg);
		return -1;
	}
	
	/* Move them into place */
	for (out = lu_list, scan = top; scan; scan = scan->next, out++)
		memcpy(out, scan, sizeof(List));
	
	/* Now, sort them */
	qsort(lu_list, lu_lists, sizeof(List), &compare_lists);
	
	/* Make sure we have no clones */
	for (scan = lu_list; scan != lu_list + lu_lists; scan++)
	{
		if (scan != lu_list)
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

static char* find_after(const char* str, const char* cfg, int lines)
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

static int open_mboxs()
{
	List* list;
	Mbox* mbox;
	
	int ok = 0;
	
	for (list = lu_list; list != lu_list + lu_lists; list++)
	{
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
		{
			mbox->fd = open(mbox->path, O_RDONLY);
			if (mbox->fd == -1)
			{
				perror(mbox->path);
				ok = -1;
			}
		}
	}
	
	return ok;
}

int lu_load_config(const char* cfg)
{
	List** target_list;
	Mbox** target_mbox;
	List*  what;
	List*  top;
	
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
	
	if ((c = fopen(cfg, "r")) == 0)
	{
		perror(cfg);
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
		
		if (!strcmp(&keyword[0], "dbdir"))
		{
			if (what)
			{
				fprintf(stderr, "%s:%d: error: "
					"dbdir in list scope\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			if (lu_dbdir)
			{
				fprintf(stderr, "%s:%d: warning: "
					"dbdir reset\n",
					cfg, lines);
				free(lu_dbdir);
			}
			
			lu_dbdir = find_after(&line[0], cfg, lines);
			if (!lu_dbdir)
				ok = -1;
		}
		else if (!strcmp(&keyword[0], "wwwdir"))
		{
			if (what)
			{
				fprintf(stderr, "%s:%d: error: "
					"wwwdir in list scope\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			if (lu_wwwdir)
			{
				fprintf(stderr, "%s:%d: warning: "
					"wwwdir reset\n",
					cfg, lines);
				free(lu_wwwdir);
			}
			
			lu_wwwdir = find_after(&line[0], cfg, lines);
			if (!lu_wwwdir)
				ok = -1;
		}
		else if (!strcmp(&keyword[0], "pidfile"))
		{
			if (what)
			{
				fprintf(stderr, "%s:%d: error: "
					"pidfile in list scope\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			if (lu_pidfile)
			{
				fprintf(stderr, "%s:%d: warning: "
					"pidfile reset\n",
					cfg, lines);
				free(lu_pidfile);
			}
			
			lu_pidfile = find_after(&line[0], cfg, lines);
			if (!lu_pidfile)
				ok = -1;
		}
		else if (!strcmp(&keyword[0], "list"))
		{
			if (sscanf(&line[0], "%19s %d = ", &keyword[0], &i) != 2)
			{
				fprintf(stderr, "%s:%d: error: "
					"missing numeric tag for list\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			if (i < 0 || i >= 0xFFFFUL)
			{
				fprintf(stderr, "%s:%d: error: "
					"id is not within bounds\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			tmp = find_after(&line[0], cfg, lines);
			if (!tmp)
			{
				ok = -1;
				continue;
			}
			
			what = *target_list = malloc(sizeof(List));
			if (!*target_list)
			{
				fprintf(stderr, "%s:%d: error: "
					"out of memory\n",
					cfg, lines);
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
			
			target_mbox = &what->mbox_head;
			target_list = &what->next;
		}
		else if (!strcmp(&keyword[0], "description"))
		{
			if (!what)
			{
				fprintf(stderr, "%s:%d: error: "
					"description outside list scope\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			if (what->description)
			{
				fprintf(stderr, "%s:%d: warning: "
					"description reset\n",
					cfg, lines);
				free(what->description);
			}
			
			what->description = find_after(&line[0], cfg, lines);
			if (!what->description)
				ok = -1;
		}
		else if (!strcmp(&keyword[0], "address"))
		{
			if (!what)
			{
				fprintf(stderr, "%s:%d: error: "
					"address outside list scope\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			if (what->address)
			{
				fprintf(stderr, "%s:%d: warning: "
					"address reset\n",
					cfg, lines);
				free(what->address);
			}
			
			what->address = find_after(&line[0], cfg, lines);
			if (!what->address)
				ok = -1;
		}
		else if (!strcmp(&keyword[0], "mbox"))
		{
			if (!what)
			{
				fprintf(stderr, "%s:%d: error: "
					"mbox outside list scope\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			if (i < 0 || i >= 0xFFFFUL)
			{
				fprintf(stderr, "%s:%d: error: "
					"id is not within bounds\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			if (sscanf(&line[0], "%19s %d = ", &keyword[0], &i) != 2)
			{
				fprintf(stderr, "%s:%d: error: "
					"missing numeric tag for mbox\n",
					cfg, lines);
				ok = -1;
				continue;
			}
			
			tmp = find_after(&line[0], cfg, lines);
			if (!tmp)
			{
				ok = -1;
				continue;
			}
			
			*target_mbox = malloc(sizeof(Mbox));
			if (!*target_mbox)
			{
				fprintf(stderr, "%s:%d: error: "
					"out of memory\n",
					cfg, lines);
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
		else
		{
			fprintf(stderr, "%s:%d: error: unknown keyword '%s'\n",
				cfg, lines, &keyword[0]);
			ok = -1;
		}
	}
	
	fclose(c);
	
	if (!lu_pidfile)
	{
		fprintf(stderr, "%s: warning: pidfile not declared - "
			"using: %s\n", cfg, DEFAULT_PID_FILE);
		lu_pidfile = strdup(DEFAULT_PID_FILE);
	}
	
	if (!lu_wwwdir)
	{
		fprintf(stderr, "%s: warning: wwdir not declared - "
			"using: %s\n", cfg, WWWDIR);
		lu_wwwdir = strdup(WWWDIR);
	}
	
	if (!lu_dbdir)
	{
		fprintf(stderr, "%s: warning: dbdir not declared - "
			"using: %s\n", cfg, DBDIR);
		lu_dbdir = strdup(DBDIR);
	}
	
	if (ok != 0)
	{
		return ok;
	}
	
	if (sort_configuration(top, cfg) != 0)
	{
		return -1;
	}
	
	if (open_mboxs() != 0)
	{
		return -1;
	}
	
	return 0;
}
