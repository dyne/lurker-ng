/*  $Id: main.c,v 1.3 2002-01-22 23:10:59 terpstra Exp $
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

#include "records.h"
#include "globals.h"
#include "io.h"

#include <sys/un.h>
#include <sys/socket.h>
#include <popt.h>
#include <unistd.h>
#include <st.h>

static void* watch_mboxs(void* arg)
{
	while (1)
	{
		printf("."); fflush(stdout);
		st_sleep(1);
	}
	
	return 0;
}

static void* handle_client(void* arg)
{
	st_netfd_t client = (st_netfd_t)arg;
	printf("c"); fflush(stdout);
	
	st_netfd_close(client);
	return 0;
}

int main(int argc, const char* argv[])
{
	FILE*		pid;
	char		c;
	poptContext	optCon;
	int		detach = 1;
	const char*	config = DEFAULT_CONFIG_FILE;
	
	struct sockaddr_un	sun_addr;
	int			sun_fd;
	int			sun_len;
	st_netfd_t		sun_stfd;
	st_netfd_t		client_fd;
		
	struct poptOption optionsTable[] = {
		{ "config",   'c', POPT_ARG_STRING, &config, 0, "Config file (default: " DEFAULT_CONFIG_FILE ")", "FILE" },
		{ "nodetach", 'n', POPT_ARG_VAL,    &detach, 0, "Do not run as a daemon", 0 },
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
	
	if (chdir(lu_dbdir) != 0)
	{
		perror(lu_dbdir);
		return 1;
	}
	
	if (lu_open_db() != 0)
		return 1;
	
	if (lu_sync_mbox() != 0)
		return 1;
	
	if (st_init() != 0)
	{
		fprintf(stderr, "Could not initialize st threading library\n");
		return 1;
	}
	
	if ((sun_fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		perror("opening domain socket");
		return 1;
	}
	
	memset(&sun_addr, 0, sizeof(sun_addr));
	
	sun_addr.sun_family = PF_UNIX;
	strcpy(&sun_addr.sun_path[1], PACKAGE "_sock"); /* !!! Should somehow allow multiple lurkerd's */
	sun_len = sizeof(sun_addr.sun_family) + strlen(&sun_addr.sun_path[1]) + 1;
	
	if (bind(sun_fd, (struct sockaddr*)&sun_addr, sun_len) < 0)
	{
		perror("Could not bind abstract domain socket");
		return 1;
	}
	
	if (listen(sun_fd, 10) < 0)
	{
		perror("Could not listen on abstract domain socket");
		return 1;
	}
	
	if ((sun_stfd = st_netfd_open(sun_fd)) == 0)
	{
		perror("Passing domain socket to libst");
		return 1;
	}
	
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
	
	/* Launch the thread which watches all the mboxes for data
	 */
	st_thread_create(&watch_mboxs, 0, 0, 0);
	
	/* Here is the main loop, we watch the domain socket for incoming
	 * requests and spin them off in a service thread.
	 */
	while (1)
	{
		st_sleep(100);
		sun_len = sizeof(sun_addr);
		client_fd = st_accept(sun_stfd, (struct sockaddr*)&sun_addr, &sun_len, 1000000);
		if (client_fd != 0)
		{
			st_thread_create(&handle_client, client_fd, 0, 0);
		}
	}
	
	return 0;
}
