/*  $Id: mbox.c,v 1.27 2002-05-11 19:24:59 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "prefix.h"

#include "mbox.h"
#include "config.h"
#include "summary.h"
#include "indexer.h"
#include "expiry.h"
#include "md5.h"

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <st.h>
#include <string.h>

/*------------------------------------------------ Constant parameters */

/* Unlock all locked mboxs every 100 messages.
 */
#define LU_MBOX_LOCK_DROP	100

/* How much space do we expect to need for a given message?
 */
#define LU_MBOX_INIT_MSG_SIZE	4096

/* How much memory do we speculatively mmap?
 */
#define LU_MBOX_MMAP_SPECULATE	65536

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

	mbox->locked = 1;
	return 0;
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

	mbox->locked = 0;
	return 0;
}

static int my_mbox_mmap(
	Lu_Config_Mbox*			mbox,
	struct Lu_Config_Message*	msg,
	off_t				off,
	size_t				msize)
{
	size_t ps_bits = getpagesize() - 1;
	size_t speculate_size;
	
	if (LU_MBOX_MMAP_SPECULATE < msize)
		speculate_size = msize;
	else
		speculate_size = LU_MBOX_MMAP_SPECULATE;
	
	if (msg->map.base)
	{
		if (off >= msg->map.off && 
			msg->map.off + msg->map.size > off + msize)
		{	/* Reuse the same mapping */
			return 0;
		}
		
		/* Else, remap it */
		lu_mbox_destroy_map(msg);
	}
	
#ifdef DEBUG
	printf("m"); fflush(stdout);
#endif
	
	/*
	 * Some systems expect the mmap offset to be aligned to the VM
	 * page size, some don't.  Do it just to be safe -- it won't waste
	 * any space.
	 */
	msg->map.off = off & ~ps_bits;
	msg->map.size = (speculate_size + (off - msg->map.off) + ps_bits) & 
			~ps_bits;

	/* Mapping past the EOF is fine. In fact - we like doing this. =>
	 */
	msg->map.base = mmap(NULL, msg->map.size, PROT_READ | PROT_WRITE,
				MAP_PRIVATE, mbox->fd, msg->map.off);
	if (!msg->map.base) return -1;
	
	return 0;
}

static char* my_mbox_strnstr(
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
	return 0;
}

static void my_mbox_swallow(
	struct MD5Context*	ctx, 
	char*			buf, 
	long 			len)
{
	char* e = buf + len;
	char* s;
	
	s = buf;
	while (s != e)
	{
		for (buf = s; s != e && *s >= 33 && *s <= 126; s++);
		MD5Update(ctx, buf, s - buf);
		for (; s != e && (*s < 33 || *s > 126); s++);
	}
}

static void my_mbox_swallow_addr(
	struct MD5Context*	ctx,
	ADDRESS*		addr)
{
	while (addr)
	{
		if (addr->personal)
			my_mbox_swallow(ctx, addr->personal, strlen(addr->personal));
		if (addr->mailbox)
			my_mbox_swallow(ctx, addr->mailbox,  strlen(addr->mailbox));
		if (addr->host)
			my_mbox_swallow(ctx, addr->host,     strlen(addr->host));
		
		addr = addr->next;
	}
}

static int my_mbox_process_mbox(
	Lu_Config_Mbox* mbox, 
	Lu_Config_List* list, 
	time_t stamp)
{
	message_id		id;
	struct Lu_Mbox_Message	m;
	char			decode_subj [200];
	char			author_name [100];
	char			author_email[100];
	char			message_id[LU_KEYWORD_LEN+1];
	char			reply_to  [LU_KEYWORD_LEN+1];
	int			error;
	int			new_message;
	struct MD5Context	md5c;
	unsigned char		digest[16];
	
	if (lu_mbox_map_message(mbox, &mbox->msg, mbox->length) != 0)
		return -1;
	
	if (lu_mbox_parse_message(&mbox->msg, &m) != 0)
		return -1;
	
	decode_subj [0] = 0;
	author_name [0] = 0;
	author_email[0] = 0;
	
	/* Ok, we prefer reply-to to from to sender.
	 * However, often we have people with personal name in the from
	 * field even though they specified a reply-to. We pick this in
	 * preference.
	 */
	  	           	 		          
	if (m.env->reply_to)
	{
		if (m.env->reply_to->personal)
		{
			lu_common_decode_header(
				m.env->reply_to->personal,
				&author_name[0],
				sizeof(author_name),
				lu_mbox_find_charset(m.body));
		}
		else if (m.env->from && m.env->from->personal)
		{
			lu_common_decode_header(
				m.env->from->personal,
				&author_name[0],
				sizeof(author_name),
				lu_mbox_find_charset(m.body));
		}
		
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
		{
			lu_common_decode_header(
				m.env->from->personal,
				&author_name[0],
				sizeof(author_name),
				lu_mbox_find_charset(m.body));
		}
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
		{
			lu_common_decode_header(
				m.env->sender->personal,
				&author_name[0],
				sizeof(author_name),
				lu_mbox_find_charset(m.body));
		}
		if (m.env->sender->mailbox && m.env->sender->host)
		{
			snprintf(&author_email[0], sizeof(author_email),
				"%s@%s", m.env->sender->mailbox,
				m.env->sender->host);
		}
	}
	
	if (m.env->subject)
	{
		lu_common_decode_header(
			m.env->subject,
			&decode_subj[0],
			sizeof(decode_subj),
			lu_mbox_find_charset(m.body));
	}
	
	if (!decode_subj[0])
	{
		strcpy(&decode_subj[0], "No subject");
	}
		
	/* Empty fields initially */
	message_id[0] = reply_to[0] = 0;
	
	if (m.env->message_id)
		strcpy(&message_id[0], lu_common_cleanup_id(m.env->message_id));
	if (!strchr(&message_id[0], '@'))
	{	/* We only like message-ids with an '@' in them. */
		message_id[0] = 0;
	}
	
	if (!message_id[0])
	{	/* We need a message-id. Invent one with md5! */
		MD5Init(&md5c);
		my_mbox_swallow_addr(&md5c, m.env->from);
		my_mbox_swallow_addr(&md5c, m.env->sender);
		my_mbox_swallow_addr(&md5c, m.env->reply_to);
		my_mbox_swallow_addr(&md5c, m.env->to);
		my_mbox_swallow_addr(&md5c, m.env->cc);
		my_mbox_swallow(&md5c, m.buffer + m.body->contents.offset, 
			m.body->contents.text.size);
		
		MD5Final(digest, &md5c);
		sprintf(&message_id[0], 
			"%02x%02x%02x%02x%02x%02x%02x%02x"
			"%02x%02x%02x%02x%02x%02x%02x%02x"
			"@lurker.mid",
			digest[0], digest[1], digest[2], digest[3],
			digest[4], digest[5], digest[6], digest[7],
			digest[8], digest[9], digest[10], digest[11],
			digest[12], digest[13], digest[14], digest[15]);
	}
	
	if (!reply_to[0] && m.env->in_reply_to)
	{
		strcpy(&reply_to[0], lu_common_cleanup_id(m.env->in_reply_to));
		if (!strchr(&reply_to[0], '@'))
			reply_to[0] = 0;
	}
	
	if (!reply_to[0] && m.env->followup_to)
	{
		strcpy(&reply_to[0], lu_common_cleanup_id(m.env->followup_to));
		if (!strchr(&reply_to[0], '@'))
			reply_to[0] = 0;
	}
	
	if (!reply_to[0] && m.env->references)
	{
		strcpy(&reply_to[0], lu_common_cleanup_id(m.env->references));
		if (!strchr(&reply_to[0], '@'))
			reply_to[0] = 0;
	}
	
	/* Notify the cache about an import */
	if (lu_expiry_notice_import(list->id) != 0)
	{
		lu_mbox_destroy_message(&m);
		return -1;
	}
	
	/* Prep the keyword indexer buffer */
	lu_indexer_prep();
	
	/* Store the summary information for this message and link the thread 
	 */
	error = lu_summary_import_message(
		list->id, 
		mbox->id, 
		mbox->length, 
		stamp,
		&message_id  [0],
		&decode_subj [0], 
		&author_name [0],
		&author_email[0],
		&id);

	if (error == -1 || id == lu_common_minvalid)
	{
		lu_mbox_destroy_message(&m);
		return -1;
	}
	
	new_message = (error == 0);
	
	error = lu_summary_reply_to_resolution(
		id,
		&message_id[0],
		&reply_to[0]);
	if (error != 0)
	{
		lu_mbox_destroy_message(&m);
		return -1;
	}
	
	if (new_message)
	{
		lu_indexer_message(
			&m, 
			stamp,
			&reply_to[0]);
	}
	
	error = lu_indexer_dump(id);
		
	if (error != 0)
	{
		lu_mbox_destroy_message(&m);
		return -1;
	}
	
#ifdef DEBUG
	printf("."); fflush(stdout);
#endif
	
	lu_config_move_mbox_end(mbox, list, mbox->msg.end);
	
	lu_mbox_destroy_message(&m);
	return 0;
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
	char	timestamp[60];
	char*	w;
	char*	buf;
	char*	e;
	char*	s;
	char*	t1;
	char*	t2;
	off_t	size;

	assert(mbox->locked);
	
	/* Keep trying to find 'From ' until it works. */
	while (1)
	{
		/* How big is the file? 
		 */
		size = lseek(mbox->fd, 0, SEEK_END);
		
		if (mbox->length == size)
		{	/* Ran out of mbox while searching. */
			return 0;
		}
		
		/* Map in enough of the message to read the headers.
		 */
		if (my_mbox_mmap(mbox, &mbox->msg, mbox->length, LU_MBOX_INIT_MSG_SIZE) != 0)
			return 0;
		
		assert (mbox->msg.map.off <= mbox->length);
		assert (mbox->msg.map.size + mbox->msg.map.off >= 
			mbox->length + LU_MBOX_INIT_MSG_SIZE);
		
		/* Convert to a char* pointer
		 */
		buf = mbox->msg.map.base;
		buf += (mbox->length - mbox->msg.map.off);
		
		/* Null terminate the range for the purposes of strstr'n it.
		 */
		if (size - mbox->length < LU_MBOX_INIT_MSG_SIZE)
			e = &buf[size - mbox->length];
		else
			e = &buf[LU_MBOX_INIT_MSG_SIZE-1];
		
		/* Find the first occurance of 'From '. It should in theory be right
		 * at the start of buf... However, we should be more robust.
		 */
		s = my_mbox_strnstr(buf, "From ", e - buf);
		if (s == 0)
		{
			syslog(LOG_ERR, "Discovered >=4k more to a message after we had already processed it.\n");
			lu_config_move_mbox_end(mbox, list, mbox->length + (e - buf));
			
			/* Try again */
			continue;
		}
		
		if (s != buf)
		{
			syslog(LOG_ERR, "Discovered <4k more to a message after we had already processed it.\n");
			lu_config_move_mbox_end(mbox, list, mbox->length + (s - buf));
			
			continue;
		}
		
		s += 5; /* Pass the 'From ' */
		while (s != e && *s != ' ') s++; /* Skip the address */
		while (s != e && *s == ' ') s++; /* Skip the spaces */
		
		/* Now, we should be on the timestamp */
		w = &timestamp[0];
		while (s != e && *s != '\n' && w != &timestamp[sizeof(timestamp)-1])
			*w++ = *s++;
		*w = 0;
		
		if (!timestamp[0])
		{	/* Not a valid From line.
			 */
			syslog(LOG_ERR, "Discovered an invalid From line - must be prior message continued.\n");
			
			lu_config_move_mbox_end(mbox, list, mbox->length + (s - buf) + 1);
			continue;
		}
		
		break;
	}
	
	/* Process the From line */
	arrival_timestamp = my_mbox_convert_date_mbox(&timestamp[0]);
	
	/* Now, see if we can find a date field too */
	t1 = my_mbox_strnstr(s, "\n\n",     e - s);
	t2 = my_mbox_strnstr(s, "\r\n\r\n", e - s);
	s  = my_mbox_strnstr(s, "\nDate: ", e - s);
	
	/* If we saw 'Date:' before the end of the headers
	 */
	client_timestamp = 0;
	if (s && (!t1 || s < t1) && (!t2 || s < t2))
	{
		s += 7; /* Skip '\nDate: ' */
		
		w = &timestamp[0];
		while (s != e && *s != '\n' && w != &timestamp[sizeof(timestamp)-1])
			*w++ = *s++;
		*w = 0;
		
		if (timestamp[0])
		{
			client_timestamp = my_mbox_convert_date_field(&timestamp[0]);
		}
	}
	
	return mbox->next_message = 
		lu_summary_timestamp_heuristic(arrival_timestamp, client_timestamp);
}

/* We need to avoid excessive locking. Experimentation shows that this is one 
 * of the programs bottlenecks (esp. over nfs to the spool).
 * Also, we want to keep the mmaps open as long as possible too.
 *
 * Plan: keep everything mmap'd indefinitely. Keep mailbox locks open to
 * all mboxes, but release all locks every LU_MBOX_LOCK_DROP messages.
 *
 * Don't hold locks while delaying between mail checks.
 */
static time_t my_mbox_find_lowest(
	Lu_Config_List** list, 
	Lu_Config_Mbox** mbox)
{
	Lu_Config_List*	scan_list;
	Lu_Config_Mbox*	scan_mbox;
	time_t		lowest_timestamp;
	time_t		candidate;
	off_t		size;
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
				size = lseek(scan_mbox->fd, 0, SEEK_END);
				if (scan_mbox->length != size)
				{	/* The mbox is bigger now, so we should
					 * check for new messages
					 */
					 
					if (my_mbox_lock_mbox(scan_mbox) != 0)
						continue;
						
					/* Can't reuse size b/c could have grown
					 * while we acquired the lock.
					 */
					candidate = my_mbox_extract_timestamp(
						scan_mbox, scan_list);
				}
				else
				{	/* The mbox hasn't grown; we have no
				 	 * more data yet.
				 	 */
					candidate = 0;
				}
			}
			
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
	
	*list = lowest_list;
	*mbox = lowest_mbox;
	
	return lowest_timestamp;
}

static int my_mbox_unlock_all()
{
	Lu_Config_List*	scan_list;
	Lu_Config_Mbox*	scan_mbox;
	
	for (	scan_list = lu_config_list; 
		scan_list != lu_config_list + lu_config_lists; 
		scan_list++)
	{
		for (	scan_mbox = scan_list->mbox;
			scan_mbox != scan_list->mbox + scan_list->mboxs;
			scan_mbox++)
		{
			my_mbox_unlock_mbox(scan_mbox);
		}
	}
	
	return 0;
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
			if (count == LU_MBOX_LOCK_DROP)
			{
				count = 0;
				my_mbox_unlock_all();
			}
			
			/* Don't actually sleep, just reschedual to allow for
			 * requests to be serviced
			 */
			st_sleep(0);
		}
		else
		{	/* Pointless to keep the locks while we rest */
			count = 0;
			my_mbox_unlock_all();
			
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

/*------------------------------------------------- Public accessor methods */

const char* lu_mbox_find_charset(
	struct mail_bodystruct* body)
{
	PARAMETER* scan;
	
	for (scan = body->parameter; scan; scan = scan->next)
	{
		if (	scan->attribute &&
			(scan->attribute[0] == 'c' || scan->attribute[0] == 'C') &&
			(scan->attribute[1] == 'h' || scan->attribute[1] == 'H') &&
			(scan->attribute[2] == 'a' || scan->attribute[2] == 'A') &&
			(scan->attribute[3] == 'r' || scan->attribute[3] == 'R') &&
			(scan->attribute[4] == 's' || scan->attribute[4] == 'S') &&
			(scan->attribute[5] == 'e' || scan->attribute[5] == 'E') &&
			(scan->attribute[6] == 't' || scan->attribute[6] == 'T') &&
			!scan->attribute[7])
		{
			return lu_common_charset_maps(scan->value);
		}
	}
	
	return 0;
}

char* lu_mbox_select_body(
	struct Lu_Mbox_Message* in,
	BODY*	body,
	size_t*	len,
	int*	nfr)
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
		*nfr = 1;
		return (output((unsigned char *)input, body->contents.text.size,
				(unsigned long *)len));
	}
	
	*nfr = 0;
	*len = body->contents.text.size;
	return input;
}

int lu_mbox_map_message(
	struct Lu_Config_Mbox_T*	mbox,
	struct Lu_Config_Message*	msg,
	off_t				start)
{
	off_t	amt;
	off_t	size;
	char*	buf;
	char*	body;
	char*	end;
	char*	e;
	
	/* Keep trying till we have enough mapped to access the header of the
	 * next message.
	 */
	for (amt = LU_MBOX_INIT_MSG_SIZE; ; amt <<= 1)
	{
		/* How big is the file? 
		 */
		size = lseek(mbox->fd, 0, SEEK_END);
		
		/* We know that there are headers b/c of the scan step */
		assert (start < size);
		
		if (my_mbox_mmap(mbox, msg, start, amt) != 0)
			return -1;
		
		assert (msg->map.off <= start);
		assert (msg->map.size + msg->map.off >= 
			start + LU_MBOX_INIT_MSG_SIZE);
		
		/* Convert to a char* pointer
		 */
		buf = msg->map.base;
		buf += (start - msg->map.off);
		
		/* Get range bounds.
		 */
		if (size - start <= amt)
			e = &buf[size - start];
		else
			e = &buf[amt];
		
		/* Look for an end to the message.  (This is either an EOF or
		 * an unmangled '\nFrom'.)
		 */
		end = my_mbox_strnstr(buf, "\nFrom ", e - buf);
		
		if (end)
		{	/* We found the start of the next message. */
			end++;
			break;
		}
		
		/* Did we conclude that all the way to EOF is a message? */
		if (size - start <= amt)
		{	/* We scanned all the way to EOF */
			end = e;
			break;
		}
		
		/* Try again with twice as much storage */
	}
	
	body = my_mbox_strnstr(buf, "\n\n", end - buf);
	if (!body) body = my_mbox_strnstr(buf, "\r\n\r\n", end - buf);
	if (!body) body = my_mbox_strnstr(buf, "\r\r",     end - buf);
	
	/* Some evil mail has no headers - just body... */
	if (!body) body = buf;
	
	msg->headers = start;
	msg->body    = body - buf + start;
	msg->end     = end  - buf + start;
	
	return 0;
}

int lu_mbox_destroy_map(
	struct Lu_Config_Message* msg)
{
	if (!msg->map.base)
		return 0;
	
	munmap(msg->map.base, msg->map.size);
	msg->map.base = 0;
	return 0;
}

int lu_mbox_parse_message(
	struct Lu_Config_Message*	mmap,
	struct Lu_Mbox_Message*		msg)
{
	char*	buf;
	
	buf = mmap->map.base;
	buf += mmap->headers - mmap->map.off;
	
	msg->buffer = buf;
	msg->buffer += mmap->body - mmap->headers;
	
	/* Parse the message. */
	INIT(	&msg->bss, 
		mail_string, 
		msg->buffer, 
		(size_t)(mmap->end - mmap->body));
		
	rfc822_parse_msg(
		&msg->env, 
		&msg->body, 
		buf, 
		(size_t)(mmap->body - mmap->headers),
		&msg->bss, 
		"localhost", 
		0);
	
	return 0;
}

int lu_mbox_destroy_message(
	struct Lu_Mbox_Message*	m)
{
	mail_free_body(&m->body);
	mail_free_envelope(&m->env);
	return 0;
}
