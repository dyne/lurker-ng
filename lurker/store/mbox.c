/*  $Id: mbox.c,v 1.11 2002-02-13 16:32:07 cbond Exp $
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

/*!!! Don't forget to turn off in final release */
#define DEBUG 1

#include "common.h"
#include "io.h"

#include "mbox.h"
#include "config.h"
#include "summary.h"
#include "indexer.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
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
;
/* Returns 0 on success.
 */
static int my_mbox_lock_mbox(
	Lu_Config_Mbox* mbox)
{
#ifdef USE_LOCK_FCNTL
	struct flock lck;
#endif
	
	if (mbox->locked)
		return 0;
#ifdef DEBUG
	printf("l"); fflush(stdout);
#endif

#ifdef USE_LOCK_FCNTL	
	memset (&lck, 0, sizeof (struct flock));
	lck.l_type = F_RDLCK;
	lck.l_whence = SEEK_CUR;
	/* leave range as current to eof */
	
	if (fcntl(mbox->fd, F_SETLK, &lck) == -1)
		return -1;
#endif

#if defined(USE_LOCK_FLOCK)
	if (flock(mbox->fd, LOCK_SH) == -1)
		return -1;
#elif defined(USE_LOCK_LOCKF)
	if (lockf(mbox->fd, F_TLOCK, 0) == -1)
		return -1;
#endif

	memset(&mbox->map, 0, sizeof(mbox->map));
	mbox->locked = 1;
	return 0;
}

static int my_mbox_mmap(
	Lu_Config_Mbox* mbox,
	off_t off,
	size_t msize)
{
	off_t round;

	/*
	 * Some systems expect the mmap offset to be aligned to the VM
	 * page size, some don't.  Do it just to be safe -- it won't waste
	 * any space.
	 */
	round = off & ~(getpagesize() - 1);
	mbox->map.off = off;
	mbox->map.size = msize + off - round;

	/*
	 * Don't map past the end of the file.
	 */
	if (mbox->map.size + round > mbox->sb.st_size)
		mbox->map.size = mbox->sb.st_size - round;

	mbox->map.addr = mmap(NULL, mbox->map.size, PROT_READ | PROT_WRITE,
				MAP_PRIVATE, mbox->fd, round);
	if (mbox->map.addr) {
		mbox->map.start = (int8_t *)mbox->map.addr + off - round;
		return 0;
	}
	return 1;
}

static void my_mbox_unmap(
	Lu_Config_Mbox *mbox)
{
	if (mbox->map.addr)
	{
		munmap(mbox->map.addr, mbox->map.size);
		mbox->map.addr = NULL;
	}
}

static int my_mbox_unlock_mbox(
	Lu_Config_Mbox* mbox)
{
#ifdef USE_LOCK_FCNTL
	struct flock lck;
#endif
	
	if (!mbox->locked)
		return 0;
#ifdef DEBUG
	printf("u"); fflush(stdout);
#endif
	
#ifdef USE_LOCK_FCNTL
	lck.l_type = F_UNLCK;
	fcntl(mbox->fd, F_SETLK, &lck);
#endif

#if defined(USE_LOCK_FLOCK)
	flock(mbox->fd, LOCK_UN);
#elif defined(USE_LOCK_LOCKF)
	lockf(mbox->fd, F_ULOCK, 0);
#endif

	if (mbox->map.addr)
		my_mbox_unmap(mbox);

	mbox->locked = 0;
	return 0;
}

static char *local_strnstr(
	const char* s,
	const char* d,
	ssize_t slen)
{
	const char* orig;
	size_t dlen;

	dlen = strlen(d);
	for (orig = s; slen > s - orig; ++s)
		if (*s == *d && strncmp(s + 1, d + 1, dlen - 1) == 0)
			return (char *)s;
	return NULL;
}

/*
 * Calculate the actual offset of a message.
 */
static int message_offset(
	char *buffer,
	size_t *offset)
{
	char *calc;

	if ((calc = strstr(buffer, "\r\n\r\n")) != NULL)
		calc += 4;
	else
		if ((calc = strstr(buffer, "\n\n")) != NULL)
			calc += 2;
	*offset = (size_t)(calc - buffer);
	return (calc == NULL);
}

static void my_mbox_process_mbox(
	Lu_Config_Mbox* mbox, 
	Lu_Config_List* list, 
	time_t stamp)
{
	off_t		old, new;
	message_id	id;
	struct msg	m;
	char*		author_name;
	char*		end;
	char		author_email[200];
	int		error;
	size_t		off;

	assert(mbox->locked);
	
	assert(mbox->locked);
	
	old = lseek(mbox->fd, 0, SEEK_CUR);
	if (old == -1)
	{
		return;
	}

	lseek(mbox->fd, 0, SEEK_SET);
	if (fstat(mbox->fd, &mbox->sb))
		return;
	lseek(mbox->fd, old, SEEK_SET);

	/*
	 * If this offset extends past the currently mapped region, or
	 * we have less than 1K of this message, remap it starting at
	 * `old.'  Otherwise if no previous map exists, allocate 128K
	 * as a starting point.
	 */
	if (mbox->map.addr)
		if (mbox->map.off + mbox->map.size - old < 1024)
			my_mbox_unmap(mbox);
	if (mbox->map.addr == NULL && my_mbox_mmap(mbox, old, 1 << 17))
		return;

	/*
	 * Look for an end to the message.  (This is either an EOF or
	 * an unmangled '\nFrom'.)
	 */
	while (1)
	{
		end = local_strnstr((const char *)map_off(mbox, old),
				"\nFrom ", map_end(mbox) - map_off(mbox, old));
		if (end)
			break;
		if (mbox->map.off + mbox->map.size >= mbox->sb.st_size)
		{
			end = map_end(mbox);
			break;
		}

		my_mbox_unmap(mbox);
		if (my_mbox_mmap(mbox, old, 2 * mbox->map.size))
			return;
	}

	/* Seek to the end of this message for the next call. */
	if (lseek(mbox->fd, end - map_off(mbox, old) + 1, SEEK_CUR) < 0)
		return;

	end[0] = '\0';
	if (message_offset(map_off(mbox, old), &off))
		return;

	/* Parse the message. */
	m.buffer = map_off(mbox, old) + off;
	INIT(&m.bss, mail_string, m.buffer, (size_t)(end - m.buffer));
	rfc822_parse_msg(&m.env, &m.body, map_off(mbox, old), off, &m.bss,
		"localhost", 0);

	author_name = "";
	author_email[0] = 0;
		
	if (m.env->reply_to)
	{
		if (m.env->reply_to->personal)
			author_name = m.env->reply_to->personal;
		if (m.env->reply_to->mailbox && m.env->reply_to->host)
		{
			snprintf(&author_email[0], sizeof(author_email),
				"%s@%s", m.env->reply_to->mailbox,
				m.env->reply_to->host);
		}
	}
	else if (m.env->from)
	{
		if (m.env->from->personal)
			author_name = m.env->from->personal;
		if (m.env->from->mailbox && m.env->from->host)
		{
			snprintf(&author_email[0], sizeof(author_email),
				"%s@%s", m.env->from->mailbox,
				m.env->from->host);
		}
	}
	else if (m.env->sender)
	{
		if (m.env->sender->personal)
			author_name = m.env->sender->personal;
		if (m.env->sender->mailbox && m.env->sender->host)
		{
			snprintf(&author_email[0], sizeof(author_email),
				"%s@%s", m.env->sender->mailbox,
				m.env->sender->host);
		}
	}
		
	id = lu_summary_import_message(
		list->id, 
		mbox->id, 
		old, 
		stamp,
		m.env->subject, 
		author_name, 
		&author_email[0]);

	if (id == lu_common_minvalid)
	{
		if (lseek(mbox->fd, old, SEEK_SET) != old)
		{
			syslog(LOG_ERR, "Eep - failed to import message and unable to seek back for retry: %s\n",
				strerror(errno));
		}

		mail_free_body(&m.body);
		mail_free_envelope(&m.env);
		return;
	}
		
	error = lu_indexer_import(
		&m, 
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
		
		mail_free_body(&m.body);
		mail_free_envelope(&m.env);
		return;
	}

	/*!!! Unwind the indexer pushes if reply_to fails */
	lu_summary_reply_to_resolution(
		id,
		m.env->message_id,
		m.env->in_reply_to);
	
	/*!!! don't forget to remove this in release */
	printf("."); fflush(stdout);
		
	/* even if we can't parse it, we want to use what the parser said the
	 * new offset should be. -> get past junk
	 *
	 * !!! I don't think this test is necessary any longer.
	 */
	new = lseek(mbox->fd, 0, SEEK_CUR);
	if (new == -1)
	{
		syslog(LOG_ERR, "Unable to locate position in mbox\n");
	}
	
	if (new < old)
	{
		new = old;
		syslog(LOG_ERR, "Somehow our file pointer was rewound.\n");
	}
	
	lu_config_move_mbox_end(mbox, list, new);
	mail_free_body(&m.body);
	mail_free_envelope(&m.env);
}

char *mbox_select_body(
	struct msg *in,
	BODY *body,
	size_t *len,
	int *nfr)
{
	char *(*output)(unsigned char *, unsigned long, unsigned long *);
	char *input;

	switch ((int)body->encoding)
	{
		case ENCQUOTEDPRINTABLE:
			output = (char *(*)())rfc822_qprint;
			break;
		case ENCBASE64:
			output = (char *(*)())rfc822_base64;
			break;
		default:
			output = NULL;
	}

	input = in->buffer + body->contents.offset;
	if (output)
	{
		nfr[0] = 1;
		return (output((unsigned char *)input, body->contents.text.size,
				(unsigned long *)len));
	}
	nfr[0] = 0;
	len[0] = body->contents.text.size;
	return input;
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

	assert(mbox->locked);
	
	assert(mbox->locked);
	
	old = lseek(mbox->fd, 0, SEEK_CUR);
	if (old == -1)
	{
		syslog(LOG_ERR, "Unable to read mbox offset(%s): %s\n",
			mbox->path, strerror(errno));
		return 0;
	}
	
	got = read(mbox->fd, &buf[0], sizeof(buf)-1);
	if (got <= 0)
	{	/* hit eof, that's fine */
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
	
	return mbox->next_message = 
		lu_summary_timestamp_heuristic(arrival_timestamp, client_timestamp);
}

/* We need to avoid excessive locking. Experimentation shows that this is one 
 * of the programs bottlenecks (esp. over nfs to the spool).
 *
 * Plan: keep the mbox we imported from last locked after import.
 * Remember the timestamp of the next message in each mbox so we don't need
 * to relock/reread them.
 *
 * We release locks whenever a mbox is rejected as a candidate for import.
 * We release the importing mbox lock after 40 imports.
 */

static time_t my_mbox_find_lowest(
	Lu_Config_List** list, 
	Lu_Config_Mbox** mbox)
{
	Lu_Config_List*	scan_list;
	Lu_Config_Mbox*	scan_mbox;
	time_t		lowest_timestamp;
	time_t		candidate;
	Lu_Config_List*	lowest_list = 0;
	Lu_Config_Mbox*	lowest_mbox = 0;
	
	lowest_timestamp = 0;
	
	for (	scan_list = lu_config_list; 
		scan_list != lu_config_list + lu_config_lists; 
		scan_list++)
	{
		for (	scan_mbox = scan_list->mbox;
			scan_mbox != scan_list->mbox + scan_list->mboxs;
			scan_mbox++)
		{
			if (scan_mbox->next_message != 0)
			{
				candidate = scan_mbox->next_message;
			}
			else
			{
				if (my_mbox_lock_mbox(scan_mbox) != 0)
					continue;
				
				candidate = my_mbox_extract_timestamp(
					scan_mbox, scan_list);
			}
			
			if (candidate != 0 &&
				(lowest_timestamp == 0 ||
				 lowest_timestamp > candidate))
			{
				if (lowest_mbox)
				{
					my_mbox_unlock_mbox(lowest_mbox);
				}
				
				lowest_timestamp = candidate;
				lowest_list = scan_list;
				lowest_mbox = scan_mbox;
			}
			else
			{
				my_mbox_unlock_mbox(scan_mbox);
			}
		}
	}
	
	*list = lowest_list;
	*mbox = lowest_mbox;
	
	return lowest_timestamp;
}

static void* my_mbox_watch(
	void* arg)
{
	Lu_Config_List*	list;
	Lu_Config_Mbox*	mbox;
	time_t		stamp;
	int		count;
	
	count = 0;
	while (!my_mbox_stop_watch)
	{
		if (my_mbox_skip_watch)
		{	/* The mailboxes aren't ready */
			st_sleep(1);
			continue;
		}
		
		stamp = my_mbox_find_lowest(&list, &mbox);
		
		if (list != 0 && mbox != 0 && stamp != 0)
		{
			my_mbox_lock_mbox(mbox);
			my_mbox_process_mbox(mbox, list, stamp);
			mbox->next_message = 0;
			
			count++;
			if (count == 80)
			{
				count = 0;
				my_mbox_unlock_mbox(mbox);
			}
			
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
