/*  $Id: main.c,v 1.2 2002-01-21 07:27:35 terpstra Exp $
 *  
 *  main.c - startup the storage daemon
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

#include <popt.h>
#include <unistd.h>

#include "records.h"
#include "globals.h"

int main(int argc, const char* argv[])
{
	FILE*		pid;
	char		c;
	poptContext	optCon;
	int		detach = 1;
	const char*	config = DEFAULT_CONFIG_FILE;
	
	struct poptOption optionsTable[] = {
		{ "config",   'c', POPT_ARG_STRING, &config, 0, "Config file (default: " DEFAULT_CONFIG_FILE ")", "FILE" },
		{ "nodetach", 'n', POPT_ARG_NONE,   &detach, 1, "Do not run as a daemon", 0 },
		{ "version",  'v', 0,               0,      'v',"Print version name", 0 },
		POPT_AUTOHELP
		{ NULL, 0, 0, NULL, 0 }
	};
	
	optCon = poptGetContext(PACKAGE "d", argc, &argv[0], &optionsTable[0], 0);
	poptSetOtherOptionHelp(optCon, "[OPTIONS]*");
	
	while ((c = poptGetNextOpt(optCon)) >= 0) switch(c)
	{
	case 'v':
		puts(STORAGE " v" VERSION);
		return 0;
	}
	
	if (c < -1)
	{
		fprintf(stderr, "%s: %s\n",
			poptBadOption(optCon, POPT_BADOPTION_NOALIAS),
			poptStrerror(c));
		return 1;
	}
	
	if (poptPeekArg(optCon))
	{
		fprintf(stderr, "%s: unknown argument\n", poptGetArg(optCon));
		return 1;
	}
	
	poptFreeContext(optCon);
	
	if (lu_load_config(config) != 0)
		return 1;
	
	if (lu_open_db() != 0)
		return 1;
	
	if (lu_sync_mbox() != 0)
		return 1;
	
	if (detach)
	{
		if ((pid = fopen(lu_pidfile, "w")) == 0)
		{
			perror(lu_pidfile);
			return 1;
		}
		
#ifdef HAVE_FORK
		/* Background the process */
		if (fork() != 0)
		{
			return 0;
		}
#endif
		
		fprintf(pid, "%d\n", getpid());
		fclose(pid);
		
#ifdef HAVE_SETSID
		/* Detach from session */
		if (setsid() == -1)
		{
			perror("Could not detech from terminal");
			return 1;
		}
#endif
		
		close(0);
		close(1);
		close(2);
	}
	
	/* Here is the main loop, we watch the files for more input and the 
	 * domain socket for incoming requests.
	 */
	
	return 0;
}
