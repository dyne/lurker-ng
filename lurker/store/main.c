/*  $Id: main.c,v 1.19 2002-02-03 03:10:53 terpstra Exp $
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

#define _GNU_SOURCE
// #define DEBUG 1

#include "common.h"
#include "io.h"
#include "message.h"

#include "config.h"
#include "flatfile.h"
#include "wbuffer.h"
#include "indexer.h"
#include "summary.h"
#include "breader.h"
#include "search.h"
#include "service.h"

#include <sys/types.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <string.h>
#include <ctype.h>

#include <st.h>

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

/*------------------------------------------------ Constant parameters */

#if defined(HAVE_FLOCK)
#define USE_LOCK_FLOCK
#elif defined(HAVE_FCNTL_H) && defined(HAVE_F_GETLK)
#define USE_LOCK_FCNTL
#elif defined(HAVE_LOCKF)
#define USE_LOCK_LOCKF
#endif

/*------------------------------------------------ Private helper methods */

static int my_main_init()
{
	if (lu_common_init() != 0) return -1;
	if (lu_config_init() != 0) return -1;
	if (lu_flatfile_init() != 0) return -1;
	if (lu_wbuffer_init()  != 0) return -1;
	if (lu_indexer_init()  != 0) return -1;
	if (lu_summary_init()  != 0) return -1;
	if (lu_breader_init()  != 0) return -1;
	if (lu_search_init()   != 0) return -1;
	if (lu_service_init()  != 0) return -1;
	
	return 0;
}

static int my_main_open(const char* c)
{
	if (lu_common_open()   != 0) return -1;
	if (lu_config_open(c)  != 0) return -1;
	if (lu_flatfile_open() != 0) return -1;
	if (lu_wbuffer_open()  != 0) return -1;
	if (lu_indexer_open()  != 0) return -1;
	if (lu_summary_open()  != 0) return -1;
	if (lu_breader_open()  != 0) return -1;
	if (lu_search_open()   != 0) return -1;
	if (lu_service_open()  != 0) return -1;
	
	return 0;
}

static int my_main_sync()
{
	int fail = 0;
	
	if (lu_service_sync()  != 0) fail = -1;
	if (lu_search_sync()   != 0) fail = -1;
	if (lu_breader_sync()  != 0) fail = -1;
	if (lu_summary_sync()  != 0) fail = -1; 
	if (lu_indexer_sync()  != 0) fail = -1;
	if (lu_wbuffer_sync()  != 0) fail = -1;
	if (lu_flatfile_sync() != 0) fail = -1;
	if (lu_config_sync()   != 0) fail = -1; 
	if (lu_common_sync()   != 0) fail = -1;
	
	return fail;
}

static int my_main_close()
{
	int fail = 0;
	
	if (lu_service_close()  != 0) fail = -1;
	if (lu_search_close()   != 0) fail = -1;
	if (lu_breader_close()  != 0) fail = -1;
	if (lu_summary_close()  != 0) fail = -1; 
	if (lu_indexer_close()  != 0) fail = -1;
	if (lu_wbuffer_close()  != 0) fail = -1;
	if (lu_flatfile_close() != 0) fail = -1;
	if (lu_config_close()   != 0) fail = -1; 
	if (lu_common_close()   != 0) fail = -1;
	
	return fail;
}

static int my_main_quit()
{
	int fail = 0;
	
	if (lu_service_quit()  != 0) fail = -1;
	if (lu_search_quit()   != 0) fail = -1;
	if (lu_breader_quit()  != 0) fail = -1;
	if (lu_summary_quit()  != 0) fail = -1; 
	if (lu_indexer_quit()  != 0) fail = -1;
	if (lu_wbuffer_quit()  != 0) fail = -1;
	if (lu_flatfile_quit() != 0) fail = -1;
	if (lu_config_quit()   != 0) fail = -1; 
	if (lu_common_quit()   != 0) fail = -1;
	
	return fail;
}

/* Returns 0 on success.
 */
static int my_main_lock_mbox(
	int fd, 
	const char* path)
{
#ifdef USE_LOCK_FCNTL
	struct flock lck;
	
	memset (&lck, 0, sizeof (struct flock));
	lck.l_type = F_RDLCK;
	lck.l_whence = SEEK_CUR;
	/* leave range as current to eof */
	
	if (fcntl(fd, F_SETLK, &lck) == -1)
		return -1;
#endif

#if defined(USE_LOCK_FLOCK)
	if (flock(fd, LOCK_SH) == -1)
		return -1;
#elif defined(USE_LOCK_LOCKF)
	if (lockf(fd, F_TLOCK, 0) == -1)
		return -1;
#endif
	
	return 0;
}

static int my_main_unlock_mbox(
	int fd, 
	const char* path)
{
#ifdef USE_LOCK_FCNTL
	struct flock lck;
	lck.l_type = F_UNLCK;
	fcntl(fd, F_SETLK, &lck);
#endif

#if defined(USE_LOCK_FLOCK)
	flock(fd, LOCK_UN);
#elif defined(USE_LOCK_LOCKF)
	lockf(fd, F_ULOCK, 0);
#endif

	return 0;
}

static void my_main_process_mbox(
	Lu_Config_Mbox* mbox, 
	Lu_Config_List* list, 
	time_t stamp)
{
	off_t old, new;
	struct msg* m;
	message_id id;
	char* author_name;
	char author_email[200];
	
	if (my_main_lock_mbox(mbox->fd, mbox->path) == -1)
		return;
	
	old = lseek(mbox->fd, 0, SEEK_CUR);
	if (old == -1)
	{
		my_main_unlock_mbox(mbox->fd, mbox->path);
		return;
	}
	
	m = mail_parse(mbox->fd, old);
	if (m)
	{
		author_name = "";
		author_email[0] = 0;
		
		if (m->env->reply_to)
		{
			if (m->env->reply_to->personal)
				author_name = m->env->reply_to->personal;
			if (m->env->reply_to->mailbox && m->env->reply_to->host)
			{
				snprintf(&author_email[0], sizeof(author_email),
					"%s@%s", m->env->reply_to->mailbox, m->env->reply_to->host);
			}
		}
		else if (m->env->from)
		{
			if (m->env->from->personal)
				author_name = m->env->from->personal;
			if (m->env->from->mailbox && m->env->from->host)
			{
				snprintf(&author_email[0], sizeof(author_email),
					"%s@%s", m->env->from->mailbox, m->env->from->host);
			}
		}
		else if (m->env->sender)
		{
			if (m->env->sender->personal)
				author_name = m->env->sender->personal;
			if (m->env->sender->mailbox && m->env->sender->host)
			{
				snprintf(&author_email[0], sizeof(author_email),
					"%s@%s", m->env->sender->mailbox, m->env->sender->host);
			}
		}
		
		id = lu_summary_import_message(list->id, mbox->id, old, stamp,
			m->env->subject, author_name, &author_email[0]);
		if (id == lu_common_minvalid)
		{
			if (lseek(mbox->fd, old, SEEK_SET) != old)
			{
				syslog(LOG_ERR, "Eep - failed to import message and unable to seek back for retry: %s\n",
					strerror(errno));
			}
			
			my_main_unlock_mbox(mbox->fd, mbox->path);
			return;
		}
		
		lu_summary_reply_to_resolution(id,
			m->env->message_id,
			m->env->in_reply_to);
		
		mail_free(m);
	}
	
	/* even if we can't parse it, we want to use what the parser said the
	 * new offset should be. -> get past junk
	 */
	new = lseek(mbox->fd, 0, SEEK_CUR);
	if (new != -1)
		lu_config_move_mbox_end(mbox, list, new);
	
	my_main_unlock_mbox(mbox->fd, mbox->path);
}

static time_t my_main_convert_date_mbox(
	char* d)
{	/* Format: www mmm dd hh:mm:ss yyyy */
	struct tm t;
	char mon[4];
	
	if (sscanf(d, "%*3s %3s %2d %02d:%02d:%02d %d",
		&mon[0], &t.tm_mday, &t.tm_hour, &t.tm_min, &t.tm_sec, &t.tm_year)
		!= 6)
	{
		return 0;
	}
	
	t.tm_year -= 1900;
	
	if      (!strcmp(&mon[0], "Jan")) t.tm_mon = 0;
	else if (!strcmp(&mon[0], "Feb")) t.tm_mon = 1;
	else if (!strcmp(&mon[0], "Mar")) t.tm_mon = 2;
	else if (!strcmp(&mon[0], "Apr")) t.tm_mon = 3;
	else if (!strcmp(&mon[0], "May")) t.tm_mon = 4;
	else if (!strcmp(&mon[0], "Jun")) t.tm_mon = 5;
	else if (!strcmp(&mon[0], "Jul")) t.tm_mon = 6;
	else if (!strcmp(&mon[0], "Aug")) t.tm_mon = 7;
	else if (!strcmp(&mon[0], "Sep")) t.tm_mon = 8;
	else if (!strcmp(&mon[0], "Oct")) t.tm_mon = 9;
	else if (!strcmp(&mon[0], "Nov")) t.tm_mon = 10;
	else if (!strcmp(&mon[0], "Dec")) t.tm_mon = 11;
	else return 0;
	
	return mktime(&t);
}

static time_t my_main_convert_date_field(
	char* d)
{
	MESSAGECACHE elt;
	
	if (mail_parse_date(&elt, d) != T)
		return 0;
	
	return mail_longdate(&elt);
}

static time_t my_main_extract_timestamp(
	Lu_Config_Mbox* mbox, 
	Lu_Config_List* list)
{
	time_t	arrival_timestamp;
	time_t	client_timestamp;
	char	buf[4096];	/* hope headers fit in here */
	char	timestamp[60];
	char*	s;
	char*	t1;
	char*	t2;
	off_t	old;
	ssize_t	got;
	
	if (my_main_lock_mbox(mbox->fd, mbox->path) == -1)
		return 0;
	
	old = lseek(mbox->fd, 0, SEEK_CUR);
	if (old == -1)
	{
		syslog(LOG_ERR, "Unable to read mbox offset(%s): %s\n",
			mbox->path, strerror(errno));
		my_main_unlock_mbox(mbox->fd, mbox->path);
		return 0;
	}
	
	got = read(mbox->fd, &buf[0], sizeof(buf)-1);
	if (got <= 0)
	{	/* hit eof, that's fine */
		my_main_unlock_mbox(mbox->fd, mbox->path);
		return 0;
	}
	
	/* Null terminate that puppy */
	buf[got] = 0;
	
	s = strstr(&buf[0], "From ");
	if (s == 0 || (s != &buf[0] && *(s-1) != '\n') ||
		sscanf(s, "From %*s %58[^\n]\n", &timestamp[0]) != 1)
	{	/* There's no From anywhere...? 
		 * Must be continued mail. Fawk. Well, nothing we can do
		 * about it now.
		 */
		
		syslog(LOG_ERR, "Discovered more to a message after we had already processed it.\n");
		old = lseek(mbox->fd, 0, SEEK_CUR);
		if (old != -1)
			lu_config_move_mbox_end(mbox, list, old);
		my_main_unlock_mbox(mbox->fd, mbox->path);
		return 0;
	}
	
	if (lseek(mbox->fd, old, SEEK_SET) != old)
	{
		syslog(LOG_ERR, "Unable to rewind mbox(%s): %s\n", 
			mbox->path, strerror(errno));
		return 0;
	}
	
	arrival_timestamp = my_main_convert_date_mbox(&timestamp[0]);
	/* Now, see if we can find a date field too */
	t1 = strstr(s, "\n\n");
	t2 = strstr(s, "\r\r");
	s  = strstr(s, "\nDate: ");
	
	/* If we saw 'Date:' before the end of the headers
	 */
	client_timestamp = 0;
	if (s && (!t1 || s < t1) && (!t2 || s < t2))
	{
		if (sscanf(s, "\nDate: %58[^\n]\n", &timestamp[0]) == 1)
		{
			client_timestamp = my_main_convert_date_field(&timestamp[0]);
		}
	}
	
	my_main_unlock_mbox(mbox->fd, mbox->path);
	return lu_summary_timestamp_heuristic(arrival_timestamp, client_timestamp);
}

static void* my_main_watch_mboxs(
	void* arg)
{
	Lu_Config_List*	scan_list;
	Lu_Config_Mbox*	scan_mbox;
	time_t		lowest_timestamp;
	time_t		candidate;
	Lu_Config_List*	lowest_list = 0;
	Lu_Config_Mbox*	lowest_mbox = 0;
	
	while (1)
	{
		lowest_timestamp = 0;
		
		for (	scan_list = lu_config_list; 
			scan_list != lu_config_list + lu_config_lists; 
			scan_list++)
		{
			for (	scan_mbox = scan_list->mbox;
				scan_mbox != scan_list->mbox + scan_list->mboxs;
				scan_mbox++)
			{
				candidate = my_main_extract_timestamp(
					scan_mbox, scan_list);
				
				if (candidate != 0 &&
					(lowest_timestamp == 0 ||
					 lowest_timestamp > candidate))
				{
					lowest_timestamp = candidate;
					lowest_list = scan_list;
					lowest_mbox = scan_mbox;
				}
			}
		}
		
		if (lowest_timestamp != 0)
		{
			my_main_process_mbox(
				lowest_mbox, lowest_list, lowest_timestamp);
			
			/* Don't actually sleep, just reschedual to allow for
			 * requests to be serviced
			 */
			st_sleep(0);
		}
		else
		{
			/* Delay till we next check for new mail */
			st_sleep(1);	
		}
	}
	
	return 0;
}

static void* my_main_handle_client(void* arg)
{
	st_netfd_t client = (st_netfd_t)arg;
	printf("c"); fflush(stdout);
	
	st_netfd_close(client);
	return 0;
}

#ifdef HAVE_SIGNAL_H

static void* lu_sched_sync(void* die)
{	/* We know that at this point we aren't in the middle of a message
	 * import thanks to the joys of controlled context switching.
	 */
	
	my_main_sync();
	
	if (!die) return 0;
	
	my_main_close();
	
	syslog(LOG_NOTICE, "shutting down ...\n");
	exit(0);
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

int main(int argc, const char* argv[])
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

	while ((c = getopt(argc, (char *const *)argv, "vnc:")) != -1) {
		switch ((char)c) {
		case 'c':
			config = optarg;
			break;
		case 'n':
			detach = 0;
			break;
		case 'v':
			fprintf(stderr, STORAGE " v" VERSION "\n");
			return (0);
		default:
			fprintf(stderr, "Usage: %s [OPTIONS]...\n\n", argv[0]);
			fprintf(stderr, "\t-v\tPrint version information\n");
			fprintf(stderr, "\t-c file\tUse settings from file\n");
			fprintf(stderr, "\t-n\tDon't run as a daemon\n");
			return (1);
		}
	}
	
	if (my_main_init() != 0)
		return 1;
	
	if (my_main_open(config) != 0)
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
	strcpy(&sun_addr.sun_path[0], PACKAGE ".sock");
	unlink(&sun_addr.sun_path[0]);
	sun_len = sizeof(sun_addr.sun_family) + strlen(&sun_addr.sun_path[0]) + 1;
	
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
	
	chmod(&sun_addr.sun_path[0], 0777);
	
	if ((sun_stfd = st_netfd_open(sun_fd)) == 0)
	{
		perror("Passing domain socket to libst");
		return 1;
	}
	
	if (detach)
		openlog(STORAGE, LOG_PID, LOG_MAIL);
	else
		openlog(STORAGE, LOG_PID | LOG_PERROR, LOG_MAIL);

#ifdef HAVE_SIGNAL_H
	signal(SIGHUP,  &lu_sync_buffers);
	signal(SIGINT,  &lu_cleanup_quit);
	signal(SIGQUIT, &lu_cleanup_quit);
	signal(SIGABRT, &lu_cleanup_quit);
	signal(SIGTERM, &lu_cleanup_quit);
#endif
	
	if (detach)
	{
		if ((pid = fopen(lu_config_pidfile, "w")) == 0)
		{
			perror(lu_config_pidfile);
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
	st_thread_create(&my_main_watch_mboxs, 0, 0, 0);
	
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
			st_thread_create(&my_main_handle_client, client_fd, 0, 0);
		}
	}
	
	return 0;
}
