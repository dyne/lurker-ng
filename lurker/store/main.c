/*  $Id: main.c,v 1.40 2002-08-30 11:24:02 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"

#include "config.h"
#include "mbox.h"
#include "indexer.h"
#include "summary.h"
#include "search.h"
#include "service.h"
#include "expiry.h"

#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef DMALLOC
# include <dmalloc.h>
#endif

#ifndef LOG_PERROR
# define LOG_PERROR 0
#endif

/*------------------------------------------------ Private helper methods */

static int my_main_init(const char* c)
{
	if (lu_config_init(c)  != 0) return -1;
	if (lu_indexer_init()  != 0) return -1;
	if (lu_search_init()   != 0) return -1;
	if (lu_summary_init()  != 0) return -1;
	if (lu_expiry_init()   != 0) return -1;
	if (lu_mbox_init()     != 0) return -1;
	if (lu_service_init()  != 0) return -1;
	
	return 0;
}

static int my_main_open(void)
{
	if (lu_config_open()   != 0) return -1;
	if (lu_indexer_open()  != 0) return -1;
	if (lu_search_open()   != 0) return -1;
	if (lu_summary_open()  != 0) return -1;
	if (lu_expiry_open()   != 0) return -1;
	if (lu_mbox_open()     != 0) return -1;
	if (lu_service_open()  != 0) return -1;
	
	return 0;
}

static int my_main_sync(void)
{
	int fail = 0;
	
	if (lu_service_sync()  != 0) fail = -1;
	if (lu_mbox_sync()     != 0) fail = -1;
	if (lu_expiry_sync()   != 0) fail = -1;
	if (lu_summary_sync()  != 0) fail = -1; 
	if (lu_search_sync()   != 0) fail = -1;
	if (lu_indexer_sync()  != 0) fail = -1;
	if (lu_config_sync()   != 0) fail = -1; 
	
	return fail;
}

static int my_main_close(void)
{
	int fail = 0;
	
	if (lu_service_close()  != 0) fail = -1;
	if (lu_mbox_close()     != 0) fail = -1;
	if (lu_expiry_close()   != 0) fail = -1;
	if (lu_summary_close()  != 0) fail = -1; 
	if (lu_search_close()   != 0) fail = -1;
	if (lu_indexer_close()  != 0) fail = -1;
	if (lu_config_close()   != 0) fail = -1; 
	
	return fail;
}

static int my_main_quit(void)
{
	int fail = 0;
	
	if (lu_service_quit()  != 0) fail = -1;
	if (lu_mbox_quit()     != 0) fail = -1;
	if (lu_expiry_quit()   != 0) fail = -1;
	if (lu_summary_quit()  != 0) fail = -1; 
	if (lu_search_quit()   != 0) fail = -1;
	if (lu_indexer_quit()  != 0) fail = -1;
	if (lu_config_quit()   != 0) fail = -1; 
	
	return fail;
}

static void* my_main_handle_client(void* arg)
{
	st_netfd_t client = (st_netfd_t)arg;
	lu_service_connection(client);
	st_netfd_close(client);
	
	return 0;
}

#ifdef HAVE_SIGNAL_H

static void* lu_sched_sync(void* die)
{	/* We know that at this point we aren't in the middle of a message
	 * import thanks to the joys of controlled context switching.
	 * 
	 * Sadly, this is no longer true. We might also be in a yield
	 * position my_calc_storage. However, it is harmless to exit at
	 * this point.
	 */
	 
	syslog(LOG_NOTICE, _("syncing databases to disk ...\n"));
	my_main_sync();
	
	if (!die) return 0;
	
	my_main_close();
	my_main_quit();
	
	syslog(LOG_NOTICE, _("shutting down ...\n"));
	exit(0);
}

static void lu_parachute(int sig)
{
	static int once = 0;
	
	if (once)
	{
		syslog(LOG_CRIT, _("Program Fault (signal %d) in fault handler, giving up---your database is hosed"),
			sig);
		exit(0);
	}
	
	once = 1;
	syslog(LOG_CRIT, _("Program Fault (signal %d) - attempting to salvage db"), 
		sig);
	
	lu_sched_sync((void*)1);
}

static void lu_sync_buffers(int sig)
{
	st_thread_create(&lu_sched_sync, (void*)0, 0, 0);
}

static void lu_cleanup_quit(int sig)
{
	st_thread_create(&lu_sched_sync, (void*)1, 0, 0);
}

#endif

void help(const char* name)
{
	fprintf(stderr, _("Usage: %s [OPTIONS]...\n\n"), name);
	
	fputs(_("\t-v\tPrint version information\n"),   stderr);
	fputs(_("\t-n\tDon't run as a daemon\n"),       stderr);
	fputs(_("\t-i\tDon't import new messages\n"),	stderr);
	fputs(_("\t-s\tDon't service requests\n"),	stderr);
	fputs(_("\t-p\tOutput profiling information\n"),stderr);
	fputs(_("\t-c file\tUse settings from file\n"), stderr);
	exit(0);
}

int main(int argc, char** argv)
{
	FILE*		pid;
	int		c;
	int		detach = 1;
	const char*	config = DEFAULT_CONFIG_FILE;
	
	struct sockaddr_un	sun_addr;
	int			sun_fd;
	int			sun_len;
	st_netfd_t		sun_stfd;
	st_netfd_t		client_fd;
	
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	bind_textdomain_codeset(PACKAGE, "utf-8");
	
	while ((c = getopt(argc, (char *const *)argv, "vpnsic:")) != -1) {
		switch ((char)c) {
		case 'c':
			config = optarg;
			break;
		case 'n':
			detach = 0;
			break;
		case 'p':
			lu_mbox_enable_profile = 1;
			break;
		case 's':
			lu_service_disable = 1;
			break;
		case 'i':
			lu_mbox_disable_watch = 1;
			break;
		case 'v':
			fprintf(stderr, STORAGE " v" VERSION "\n");
			return (0);
		default:
			help(argv[0]);
		}
	}
	
	if (optind != argc)
	{
		help(argv[0]);
	}
	
	if (lu_mbox_disable_watch && lu_service_disable)
	{
		fputs(_("What am I supposed to do if not service requests OR import messages?\n"), stderr);
		return 1;
	}
	
	srandom(time(0));
	
	if (st_init() != 0)
	{
		fputs(_("Could not initialize st threading library\n"), stderr);
		return 1;
	}
	
	if (my_main_init(config) != 0)
		return 1;
	
	if ((sun_fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		perror(_("opening domain socket"));
		return 1;
	}
	
	memset(&sun_addr, 0, sizeof(sun_addr));
	
	sun_addr.sun_family = PF_UNIX;
	strcpy(&sun_addr.sun_path[0], PACKAGE ".sock");
	sun_len = sizeof(sun_addr.sun_family) + strlen(&sun_addr.sun_path[0]) + 1;
	
	/* First try connecting to see if lurker is already running */
	if (connect(sun_fd, (struct sockaddr*)&sun_addr, sun_len) == 0)
	{
		fputs(_("Lurker is already running on this db\n"), stderr);
		return 1;
	}
	
	unlink(&sun_addr.sun_path[0]);
	if (bind(sun_fd, (struct sockaddr*)&sun_addr, sun_len) < 0)
	{
		perror(_("Could not bind abstract domain socket"));
		return 1;
	}
	
	if (listen(sun_fd, 10) < 0)
	{
		perror(_("Could not listen on domain socket"));
		return 1;
	}
	
	chmod(&sun_addr.sun_path[0], 0777);
	
	if ((sun_stfd = st_netfd_open(sun_fd)) == 0)
	{
		perror(_("Passing domain socket to libst"));
		return 1;
	}
	
	if (detach)
		openlog(STORAGE, LOG_PID, LOG_MAIL);
	else
		openlog(STORAGE, LOG_PID | LOG_PERROR, LOG_MAIL);

#ifdef HAVE_SIGNAL_H
	/* These signals will trigger a safe db cleanup after the current task
	 * has been completed.
	 */
	signal(SIGHUP,  &lu_sync_buffers);
	signal(SIGINT,  &lu_cleanup_quit);
	signal(SIGQUIT, &lu_cleanup_quit);
	signal(SIGABRT, &lu_cleanup_quit);
	signal(SIGTERM, &lu_cleanup_quit);
	
	/* I am getting sick of starting imports over again during debugging.
	 * Because we could be mid-operation, this is not guaranteed to save
	 * from a corrupt database, however, it is better than nothing.
	 */
	signal(SIGSEGV, &lu_parachute);
	signal(SIGILL,  &lu_parachute);
	signal(SIGBUS,  &lu_parachute);
	signal(SIGABRT, &lu_parachute);
	signal(SIGFPE,  &lu_parachute);
	signal(SIGPIPE, &lu_parachute);
#endif
	
	if (my_main_open() != 0)
		return 1;
	
	if (detach)
	{
		if ((pid = fopen(lu_config_file->pidfile, "w")) == 0)
		{
			perror(lu_config_file->pidfile);
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
			perror(_("Could not detech from terminal"));
			return 1;
		}
#endif
		
		close(0);
		close(1);
		close(2);
	}
	
	/* Here is the main loop, we watch the domain socket for incoming
	 * requests and spin them off in a service thread.
	 */
	while (1)
	{
		sun_len = sizeof(sun_addr);
		client_fd = st_accept(sun_stfd, (struct sockaddr*)&sun_addr, &sun_len, 1000000);
		if (client_fd != 0)
		{
			st_thread_create(&my_main_handle_client, client_fd, 0, 0);
		}
	}
	
	return 0;
}
