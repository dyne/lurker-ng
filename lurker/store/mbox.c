/*  $Id: mbox.c,v 1.5 2002-02-04 07:53:43 terpstra Exp $
 *  
 *  mbox.c - Knows how to follow mboxes for appends and import messages
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
#include "summary.h"
#include "indexer.h"

#include <st.h>
#include <string.h>

/*------------------------------------------------ Constant parameters */

#if defined(HAVE_FCNTL_H) && defined(HAVE_F_GETLK)
#define USE_LOCK_FCNTL
#elif defined(HAVE_FLOCK)
#define USE_LOCK_FLOCK
#elif defined(HAVE_LOCKF)
#define USE_LOCK_LOCKF
#endif

/*------------------------------------------------ Private global vars */

static int my_mbox_stop_watch = 0;
static int my_mbox_skip_watch = 1;

/*------------------------------------------------ Private helper methods */

/* Returns 0 on success.
 */
static int my_mbox_lock_mbox(
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

static int my_mbox_unlock_mbox(
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

static void my_mbox_process_mbox(
	Lu_Config_Mbox* mbox, 
	Lu_Config_List* list, 
	time_t stamp)
{
	off_t		old, new;
	struct msg*	m;
	message_id	id;
	char*		author_name;
	char		author_email[200];
	int		error;
	
	if (my_mbox_lock_mbox(mbox->fd, mbox->path) == -1)
		return;
	
	old = lseek(mbox->fd, 0, SEEK_CUR);
	if (old == -1)
	{
		my_mbox_unlock_mbox(mbox->fd, mbox->path);
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
		
		id = lu_summary_import_message(
			list->id, 
			mbox->id, 
			old, 
			stamp,
			m->env->subject, 
			author_name, 
			&author_email[0]);
		
		if (id == lu_common_minvalid)
		{
			if (lseek(mbox->fd, old, SEEK_SET) != old)
			{
				syslog(LOG_ERR, "Eep - failed to import message and unable to seek back for retry: %s\n",
					strerror(errno));
			}
			
			my_mbox_unlock_mbox(mbox->fd, mbox->path);
			mail_free(m);
			return;
		}
		
		error = lu_indexer_import(
			m, 
			list->id,
			mbox->id,
			stamp,
			id);
		
		if (error != 0)
		{
			if (lseek(mbox->fd, old, SEEK_SET) != old)
			{
				syslog(LOG_ERR, "Eep - failed to import message and unable to seek back for retry: %s\n",
					strerror(errno));
			}
			
			my_mbox_unlock_mbox(mbox->fd, mbox->path);
			mail_free(m);
			return;
		}
		
		lu_summary_reply_to_resolution(
			id,
			m->env->message_id,
			m->env->in_reply_to);
		
		mail_free(m);
	}
	
	/* even if we can't parse it, we want to use what the parser said the
	 * new offset should be. -> get past junk
	 */
	new = lseek(mbox->fd, 0, SEEK_CUR);
	if (new == -1)
	{
		syslog(LOG_ERR, "Unable to locate position in mbox\n");
		my_mbox_unlock_mbox(mbox->fd, mbox->path);
	}
	
	if (new < old)
	{
		new = old;
		syslog(LOG_ERR, "Somehow our file pointer was rewound.\n");
	}
	
	lu_config_move_mbox_end(mbox, list, new);
	my_mbox_unlock_mbox(mbox->fd, mbox->path);
}

static time_t my_mbox_convert_date_mbox(
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

static time_t my_mbox_convert_date_field(
	char* d)
{
	MESSAGECACHE elt;
	
	if (mail_parse_date(&elt, d) != T)
		return 0;
	
	return mail_longdate(&elt);
}

static time_t my_mbox_extract_timestamp(
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
	
	if (my_mbox_lock_mbox(mbox->fd, mbox->path) == -1)
		return 0;
	
	old = lseek(mbox->fd, 0, SEEK_CUR);
	if (old == -1)
	{
		syslog(LOG_ERR, "Unable to read mbox offset(%s): %s\n",
			mbox->path, strerror(errno));
		my_mbox_unlock_mbox(mbox->fd, mbox->path);
		return 0;
	}
	
	got = read(mbox->fd, &buf[0], sizeof(buf)-1);
	if (got <= 0)
	{	/* hit eof, that's fine */
		my_mbox_unlock_mbox(mbox->fd, mbox->path);
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
		my_mbox_unlock_mbox(mbox->fd, mbox->path);
		return 0;
	}
	
	if (lseek(mbox->fd, old, SEEK_SET) != old)
	{
		syslog(LOG_ERR, "Unable to rewind mbox(%s): %s\n", 
			mbox->path, strerror(errno));
		return 0;
	}
	
	arrival_timestamp = my_mbox_convert_date_mbox(&timestamp[0]);
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
			client_timestamp = my_mbox_convert_date_field(&timestamp[0]);
		}
	}
	
	my_mbox_unlock_mbox(mbox->fd, mbox->path);
	return lu_summary_timestamp_heuristic(arrival_timestamp, client_timestamp);
}

static void* my_mbox_watch(
	void* arg)
{
	Lu_Config_List*	scan_list;
	Lu_Config_Mbox*	scan_mbox;
	time_t		lowest_timestamp;
	time_t		candidate;
	Lu_Config_List*	lowest_list = 0;
	Lu_Config_Mbox*	lowest_mbox = 0;
	
	while (!my_mbox_stop_watch)
	{
		if (my_mbox_skip_watch)
		{	/* The mailboxes aren't ready */
			st_sleep(1);
			continue;
		}
		
		lowest_timestamp = 0;
		
		for (	scan_list = lu_config_list; 
			scan_list != lu_config_list + lu_config_lists; 
			scan_list++)
		{
			for (	scan_mbox = scan_list->mbox;
				scan_mbox != scan_list->mbox + scan_list->mboxs;
				scan_mbox++)
			{
				candidate = my_mbox_extract_timestamp(
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
			my_mbox_process_mbox(
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

/*------------------------------------------------- Public component methods */

int lu_mbox_init()
{
	my_mbox_stop_watch = 0;
	my_mbox_skip_watch = 1;
	st_thread_create(&my_mbox_watch, 0, 0, 0);
	
	return 0;
}

int lu_mbox_open()
{
	my_mbox_skip_watch = 0;
	return 0;
}

int lu_mbox_sync()
{
	return 0;
}

int lu_mbox_close()
{
	my_mbox_skip_watch = 1;
	return 0;
}

int lu_mbox_quit()
{
	my_mbox_stop_watch = 1;
	return 0;
}
