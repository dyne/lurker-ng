/*  $Id: summary.c,v 1.12 2002-05-04 04:39:10 terpstra Exp $
 *  
 *  summary.h - Knows how to manage digested mail information
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
#include "prefix.h"

#include "config.h"
#include "breader.h"
#include "summary.h"

#include <ctype.h>
#include <string.h>

/*------------------------------------------------ Constant parameters */

#define LU_SQUISHY_MAX	80

/*------------------------------------------------ Private types */

/** Thread merging meta-data key - merge.hash
 *  Size - ?
 */
typedef struct Lu_Summary_Thread_Merge_Key_T
{
	lu_word		list_index;
	char		squishy_subject[0];
	/* Followed by:
	 * lu_quad	start;
	 */
} Lu_Summary_Thead_Merge_Key;

/*------------------------------------------------ Private global vars */

static int my_summary_message_fd  = -1;
static int my_summary_variable_fd = -1;
static DB* my_summary_thread_db   = 0;
static DB* my_summary_merge_db    = 0;

static message_id	my_summary_msg_free;
static time_t		my_summary_last_time;

/*------------------------------------------------ Private helper methods */

static int my_summary_path_compress(
	message_id* mid)
{
	message_id	bak;
	off_t		offset;
	
	/* First, follow the union-find trail and path compress as we do so. */
	Lu_Summary_Message sum = lu_summary_read_msummary(*mid);
	
	if (sum.timestamp == 0)
	{	/* Invalid messages are their own thread */
		return -1;
	}
	
	/* Is this the top of a thread? */
	if (sum.thread_parent == *mid)
	{
		return 0;
	}
	
	/* Nope, find out who is the top. */
	bak = sum.thread_parent;
	if (my_summary_path_compress(&sum.thread_parent) != 0)
	{	/* Something f*ked up, pass the error down */
		return -1;
	}
	
	/* Was the path compressed? Update if so. */
	if (sum.thread_parent != bak)
	{
		offset = *mid;
		offset *= sizeof(Lu_Summary_Message);
		
		/* If we can't rewrite, no big deal, we don't HAVE to 
		 * do the path compression.
		 */
		if (lseek(my_summary_message_fd, offset, SEEK_SET) != offset ||
		    write(my_summary_message_fd, &sum, sizeof(Lu_Summary_Message)) !=
		    	sizeof(Lu_Summary_Message))
		{
			syslog(LOG_WARNING, 
				"Could not compress thread path for message %d\n",
				*mid);
		}
	}
	
	*mid = sum.thread_parent;
	return 0;
}

static int my_summary_squishy_subject(
	const char* subject, 
	char* target)
{
	/* Alright, we want to drop 're:', 'fwd:', etc.
	 * Also, we don't want to be confused by fluctuations in whitespace.
	 * Anything after a 'was:' should be cut.
	 * Changes in case shouldn't be confusing.
	 * Punctuation is disregarded.
	 * We want a maximum length. (LU_SQUISHY_MAX)
	 */
	
	const char* r;
	const char* s;
	char* e;
	char* w;
	int state, ws;
	
	if (!subject)
	{
		*target = 0;
		return 0;
	}
	
	/* Skip past any number of: ' *[^ :]{0, 8}:' sequences
	 */
	state = 1;
	
	r = s = subject;
	while (*s)
	{
		if (state == 0)
		{
			if (*s == ':')
			{
				state = 1;
			}
			else if (*s == ' ' || s - r > 8)
			{
				break;
			}
			
			s++;
		}
		else
		{	/* We're skiping past whitespace */
			if (isspace(*s))
			{
				s++;
			}
			else
			{
				r = s;
				state = 0;
			}
		}
	}
	
	/* Ok, begin writing the string out to target.
	 * We compress whitespace to a single space.
	 * Change everything to lower case.
	 * Drop punctuation on the floor.
	 * Stop on a ':' and discard any word that preceded it. (was:)
	 */
	ws = 0;
	for (	w = target, e = w + LU_SQUISHY_MAX-1; 
		*r && w != e; r++)
	{
		if (isspace(*r))
		{
			ws = 1;
		}
		else
		{
			if (*r == ':' && (w - target) > 8 &&
				tolower(*(r-1)) == 'w' &&
				tolower(*(r-2)) == 'a' &&
				tolower(*(r-3)) == 's')
			{	/* Nasty colons! */
				if (ws) *w++ = ' ';
				break;
			}
			
			if (isalnum(*r))
			{
				if (ws)
				{
					*w++ = ' ';
					ws = 0;
					
					/* Need to retest since we are doing a double
					 * write
					 */
					if (w == e) break;
				}
				
				*w++ = tolower(*r);
			}
		}
	}
	
	if (*r == ':')
	{	/* Rewind 'w' by one word */
		if (w != target) w--;
		
		for (; w != target; w--)
			if (isspace(*w))
				break;
	}
	
	*w = 0;
	return w - target;
}

/* Unlink the thread from the linked list of threads in the hash.
 * The passed thread is assumed to be the correct record. It is not
 * rewritten; it is merely used to locate neighbours.
 */
static int pop_thread(
	message_id thread_id, 
	Lu_Summary_Thread* thread)
{
	int		error;
	DBT		key;
	DBT		val;
	Lu_Summary_Thread	left;
	Lu_Summary_Thread	right;
	
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	key.size = sizeof(message_id);
	val.ulen = sizeof(left);
	val.flags = DB_DBT_USERMEM;
	
	if (thread->prev_thread != lu_common_minvalid)
	{
		key.data = &thread->prev_thread;
		val.data = &left;
	
		if ((error = my_summary_thread_db->get(
			my_summary_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Thread-linkage corrupt - left missing for %d: %d: %s\n",
				thread_id, thread->prev_thread, db_strerror(error));
			return -1;
		}
		
		left.next_thread = thread->next_thread;
	}
	
	if (thread->next_thread != lu_common_minvalid)
	{
		key.data = &thread->next_thread;
		val.data = &right;
		
		if ((error = my_summary_thread_db->get(
			my_summary_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Thread-linkage corrupt - right missing for %d: %d: %s\n",
				thread_id, thread->next_thread, db_strerror(error));
			return -1;
		}
		
		right.prev_thread = thread->prev_thread;
	}
	
	if (thread->prev_thread != lu_common_minvalid)
	{
		key.data = &thread->prev_thread;
		val.data = &left;
	
		if ((error = my_summary_thread_db->put(
			my_summary_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Unable to rewrite left link for %d\n",
				thread_id);
			return -1;
		}
	}
	
	if (thread->next_thread != lu_common_minvalid)
	{
		key.data = &thread->next_thread;
		val.data = &right;
		
		if ((error = my_summary_thread_db->put(
			my_summary_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Thread-linkage corrupt - right missing for %d\n",
				thread_id);
			
			if (thread->prev_thread != lu_common_minvalid)
			{
				left.next_thread = thread_id;
				key.data = &thread->prev_thread;
				val.data = &left;
				
				if ((error = my_summary_thread_db->put(
					my_summary_thread_db, 0, &key, &val, 0)) != 0)
				{
					syslog(LOG_ERR, "Unable to restore left link for %d - broken linkage!\n",
						thread_id);
				}
			}
			
			return -1;
		}
	}
	
	thread->prev_thread = lu_common_minvalid;
	thread->next_thread = lu_common_minvalid;
	
	return 0;
}

/* Append the thread to the linked list of threads in the hash.
 * The passed thread is assumed to be the correct record and is rewritten with
 * appropriate link information.
 */
static int my_summary_push_thread(
	lu_word list, 
	message_id last_thread, 
	message_id thread_id, 
	Lu_Summary_Thread* thread)
{
	char		thread_key[sizeof(lu_word) + 1];
	int		error;
	DBT		key;
	DBT		val;
	DBT		ind;
	Lu_Summary_Thread	last;
	
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	memset(&ind, 0, sizeof(DBT));
	
	if (last_thread != lu_common_minvalid)
	{
		key.data = &last_thread;
		key.size = sizeof(message_id);
		val.data = &last;
		val.ulen = sizeof(last);
		val.flags = DB_DBT_USERMEM;
		
		error = my_summary_thread_db->get(
			my_summary_thread_db, 0, &key, &val, 0);
		if (error != 0)
		{
			syslog(LOG_ERR, "Pulling last thread summary for %d: %s\n",
				last_thread, db_strerror(error));
			goto my_summary_push_thread_error0;
		}
	}
	
	memcpy(&thread_key[0], &list, sizeof(lu_word));
	thread_key[sizeof(lu_word)] = 0xFF; /* No record is this large */
	
	key.data = &thread_key[0];
	key.size = sizeof(lu_word) + 1;
	ind.data = &thread_id;
	ind.size = sizeof(message_id);
	
	error = my_summary_merge_db->put(
		my_summary_merge_db, 0, &key, &ind, 0);
	if (error)
	{
		syslog(LOG_ERR, "Setting the last thread: %s\n",
			db_strerror(error));
		goto my_summary_push_thread_error0;
	}
	
	last.next_thread = thread_id;
	thread->prev_thread = last_thread;
	thread->next_thread = lu_common_minvalid;
	
	if (last_thread != lu_common_minvalid)
	{
		key.data = &last_thread;
		key.size = sizeof(message_id);
		val.data = &last;
		val.size = sizeof(last);
		
		error = my_summary_thread_db->put(
			my_summary_thread_db, 0, &key, &val, 0);
		if (error != 0)
		{
			syslog(LOG_ERR, "Pushing last thread summary for %d: %s\n",
				last_thread, db_strerror(error));
			goto my_summary_push_thread_error1;
		}
	}
	
	key.data = &thread_id;
	key.size = sizeof(message_id);
	val.data = thread;
	val.size = sizeof(Lu_Summary_Thread);
		
	if ((error = my_summary_thread_db->put(
		my_summary_thread_db, 0, &key, &val, 0)) != 0)
	{
		syslog(LOG_ERR, "Storing thread summary for %d: %s\n",
			thread_id, db_strerror(error));
		goto my_summary_push_thread_error2;
	}
	
	return 0;

my_summary_push_thread_error2:
	if (last_thread != lu_common_minvalid)
	{
		key.data = &last_thread;
		key.size = sizeof(message_id);
		val.data = &last;
		val.size = sizeof(last);
		
		last.next_thread = lu_common_minvalid;
		
		error = my_summary_thread_db->put(
			my_summary_thread_db, 0, &key, &val, 0);
		if (error != 0)
		{
			syslog(LOG_ERR, "Restoring last thread next link for %d: %s\n",
				last_thread, db_strerror(error));
			goto my_summary_push_thread_error1;
		}
	}

my_summary_push_thread_error1:
	key.data = &thread_key[0];
	key.size = sizeof(lu_word) + 1;
	ind.data = &last_thread;
	ind.size = sizeof(message_id);
	
	error = my_summary_merge_db->put(
		my_summary_merge_db, 0, &key, &ind, 0);
	if (error)
	{
		syslog(LOG_ERR, "Restoring the last thread: %s\n",
			db_strerror(error));
	}
	
my_summary_push_thread_error0:
	return -1;
}

/* By default, if two messages with similar subject arrive within 14 days
 * on the same mailing list, we consider them the same thread. If they have
 * the same squishy subject and a reply-to linkage, we may merge threads
 * at that time.
 */
#define LU_THREAD_OVERLAP_TIME	60*60*24*14

/* This function uses the given data to try and find a thread which the given
 * message would belong in. I initially thought that I would need to merge
 * threads. However, because we have guaranteed import order, a missing link
 * is an impossibility. It can only possibly connect to the previous thread.
 * 
 * lu_summary_minvalid -> no thread existed, make your own.
 * 
 * The caller is responsible for either setting his thread_parent to the
 * returned id if != lu_common_minvalid. Also, he must insert himself into
 * the linked list for the thread. The parent summary is returned as a 
 * convenience (less disk hits). message_count++ is a good move too.
 */
static int my_summary_find_thread(
	lu_word list, 
	const char* subject, 
	time_t timestamp,
	Lu_Summary_Thread* parent, 
	message_id* out)
{
	char		thread_key[sizeof(lu_word) + LU_SQUISHY_MAX + sizeof(lu_quad)];
	int		thread_key_len;
	char		squishy[LU_SQUISHY_MAX];
	int		squishy_len;
	lu_quad		tm;
	DBC*		cursor;
	DBT		key;
	DBT		ind;
	DBT		val;
	int		error;
	Lu_Summary_Thread	prior;
	message_id	prior_id;
	lu_quad		prior_time;
	
	/* Get the key for where this thread lies */
	tm = timestamp;
	thread_key_len = sizeof(lu_word);
	memcpy(&thread_key[0], &list, thread_key_len);
	thread_key_len += my_summary_squishy_subject(subject, &thread_key[thread_key_len]);
	memcpy(&thread_key[thread_key_len], &tm, sizeof(lu_quad));
	thread_key_len += sizeof(lu_quad);
	
	squishy_len = my_summary_squishy_subject(subject, &squishy[0]);
	
	/* Find the smallest entry >= our thread in the thread btree */
	error = my_summary_merge_db->cursor(my_summary_merge_db, 0, &cursor, 0);
	if (error)
	{
		syslog(LOG_ERR, "Unable to create thread DB cursor: %s\n",
			db_strerror(error));
		return -1;
	}
	
	memset(&key, 0, sizeof(DBT));
	memset(&ind, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	key.ulen = sizeof(thread_key);
	key.flags = DB_DBT_USERMEM;
	ind.ulen = sizeof(message_id);
	ind.flags = DB_DBT_USERMEM;
	val.ulen = sizeof(Lu_Summary_Thread);
	val.flags = DB_DBT_USERMEM;
	
	key.data = &thread_key[0];
	key.size = thread_key_len;
	ind.data = &prior_id;
	val.data = &prior;
	
	error = cursor->c_get(cursor, &key, &ind, DB_SET_RANGE);
	if (error && error != DB_NOTFOUND)
	{
		syslog(LOG_ERR, "Unable to find next thread record: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if (error == DB_NOTFOUND)
		error = cursor->c_get(cursor, &key, &ind, DB_LAST);
	else
		error = cursor->c_get(cursor, &key, &ind, DB_PREV);
	
	if (error && error != DB_NOTFOUND)
	{
		syslog(LOG_ERR, "Unable to find prev thread record: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if (error == DB_NOTFOUND ||
	    squishy_len != key.size - sizeof(lu_word) - sizeof(lu_quad) || 
	    memcmp(&squishy[0], &thread_key[sizeof(lu_word)], squishy_len))
	{	/* There is no prior thread */
		prior_time = 0;
	}
	else
	{
		memcpy(&prior_time, &thread_key[key.size-sizeof(lu_quad)], 
			sizeof(lu_quad));
			
		error = my_summary_thread_db->get(
			my_summary_thread_db, 0, &ind, &val, 0);
		if (error)
		{
			syslog(LOG_ERR, "Unable to retrieve thread summary: %s\n",
				db_strerror(error));
			return -1;
		}
	}
	
	error = cursor->c_close(cursor);
	if (error)
	{
		syslog(LOG_ERR, "Unable to close thread cursor: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if (prior_time == 0)
	{
		*out = lu_common_minvalid;
		return 0;
	}
	
	/* Ok, we now have the data loaded, do a quick consistency check */
	if (prior_time != prior.start || prior.start > prior.end)
	{
		syslog(LOG_ERR, "Corrupt thread database - prior (%d %d %d\n",
			prior_time, prior.start, prior.end);
		return -1;
	}
	
	/* Now will the new message land in the prior thread? */
	if (timestamp + LU_THREAD_OVERLAP_TIME > prior.start &&
	    prior.end + LU_THREAD_OVERLAP_TIME > timestamp)
	{	/* Ok, merge with the prior thread and not the later */
		memcpy(parent, &prior, sizeof(Lu_Summary_Thread));
		*out = prior_id;
		return 0;
	}
	
	/* New thread for this record */
	*out = lu_common_minvalid;
	return 0;
}

static int my_summary_reply_link(
	message_id src, 
	message_id dest)
{
	off_t			offset = src;
	Lu_Summary_Message	sum = lu_summary_read_msummary(src);
	
	if (sum.timestamp == 0)
	{
		return -1;
	}
	
	sum.in_reply_to = dest;
	
	offset *= sizeof(Lu_Summary_Message);
	if (lseek(my_summary_message_fd, offset, SEEK_SET) != offset ||
	    write(my_summary_message_fd, &sum, sizeof(Lu_Summary_Message)) !=
	    	sizeof(Lu_Summary_Message))
	{
		syslog(LOG_WARNING, 
			"Could not store in-reply-to link: %d -> %d\n",
			src, dest);
	}
	
	return 0;
}

/*------------------------------------------------ Public access methods */

Lu_Summary_Message lu_summary_read_msummary(
	message_id mid)
{
	Lu_Summary_Message	out;
	off_t			offset = mid;
	
	offset *= sizeof(Lu_Summary_Message);
	if (lseek(my_summary_message_fd, offset, SEEK_SET) != offset)
	{
		memset(&out, 0, sizeof(Lu_Summary_Message));
		return out;
	}
	
	if (read(my_summary_message_fd, &out, sizeof(Lu_Summary_Message)) != 
		sizeof(Lu_Summary_Message))
	{
		memset(&out, 0, sizeof(Lu_Summary_Message));
		return out;
	}
	
	return out;
}

Lu_Summary_Thread lu_summary_read_tsummary(
	message_id tid)
{
	Lu_Summary_Thread out;
	
	DBT key;
	DBT val;
	
	int error;
	
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	/* Find the actual thread head.
	 */
	if (my_summary_path_compress(&tid) != 0)
	{	/* Something messed up, propogate error */
		memset(&out, 0, sizeof(out));
		return out;
	}
	
	key.data = &tid;
	key.size = sizeof(tid);
	val.data = &out;
	val.ulen = sizeof(out);
	val.flags = DB_DBT_USERMEM;
	
	/* Retrieve it from the database.
	 */
	if ((error = my_summary_thread_db->get(
		my_summary_thread_db, 0, &key, &val, 0)) != 0)
	{
		syslog(LOG_ERR, "Thread-head %d does not have summary record\n",
			tid);
		memset(&out, 0, sizeof(out));
	}
	
	return out;
}

int lu_summary_write_variable(
	int (*write)(void* arg, const char* str),
	int (*quote)(void* arg, const char* str, size_t len),
	void* arg,
	lu_addr flat_offset)
{
	char	buf[4096];
	char*	w;
	char*	e;
	char*	s;
	int	nulls;
	int	got;
	int	have;
	int	my_var_fd;
	
	lu_addr		mask;
	off_t		offset;
	
	mask = 0xFFFFU;
	mask <<= (sizeof(lu_addr)-2)*8;
	
	/* We need a duplicate handle b/c the other thread could be 
	 * importing while we are doing this.
	 */
	my_var_fd = open("variable.flat", O_RDONLY | O_BINARY);
		
	if (my_var_fd == -1)
	{
		return -1;
	}
	
	offset = flat_offset & ~mask;
	
	if (lseek(my_var_fd, offset, SEEK_SET) != offset)
	{
		close(my_var_fd);
		return -1;
	}
	
	nulls = 0;
	have  = 0;
	while (nulls < 4)
	{
		got = read(my_var_fd, &buf[0], sizeof(buf));
		if (got <= 0) break;
		e = &buf[got];
		
		for (s = w = &buf[0]; w != e && nulls != 4; w++)
		{
			if (!*w)
			{
				if (!have && w != s)
				{
					have = 1;
					switch (nulls)
					{
					case 0:
						write(arg, "   <mid>");
						break;
					case 1:
						write(arg, "   <subject>");
						break;
					case 2:
						write(arg, " name=\"");
						break;
					case 3:
						write(arg, " address=\"");
						break;
					}
				}
				
				quote(arg, s, w - s);
				
				if (have)
				{
					switch (nulls)
					{
					case 0:
						write(arg, "</mid>\n");
						break;
					case 1:
						write(arg, "</subject>\n");
						break;
					case 2:
						write(arg, "\"");
						break;
					case 3:
						write(arg, "\"");
						break;
					}
				}
				
				nulls++;
				s = w+1;
				have = 0;
				
				if (nulls == 2)
					write(arg, "   <email");
			}
		}
		
		if (!have && w != s)
		{
			have = 1;
			switch (nulls)
			{
			case 0:
				write(arg, "   <mid>");
				break;
			case 1:
				write(arg, "   <subject>");
				break;
			case 2:
				write(arg, "name=\"");
				break;
			case 3:
				write(arg, "address=\"");
				break;
			}
		}
		
		quote(arg, s, w - s);
	}
	
	write(arg, "/>\n");
	
	close(my_var_fd);
	return 0;
}

/** Here we generally assume that the server is smarter than the client when
 *  it comes to timestamps. However, due to mail delays it is conceivable that
 *  that the client message could arrive up to 3 days after it was sent.
 *  
 *  So, if the client timestamp is with (server - 3d, server], we believe it.
 *  Otherwise we use the servers time. However, we also know that the mboxes
 *  are fed to us in chronological order. So, if this mechanism says that we're
 *  going back in time, it is wrong. We choose to use same time as the last input
 *  message in this case.
 */
time_t lu_summary_timestamp_heuristic(
	time_t server, 
	time_t client)
{
	time_t out;
	
	if (client > server || server - 3*60*60*24 > client)
		out = server;
	else
		out = client;
	
	/* Before one we've already archived? Not possible. */
	if (out < my_summary_last_time)
	{
		out = my_summary_last_time;
	}
	
	return out;
}

/* Find the last thread in the mailing list.
 */
int lu_summary_last_thread(
	lu_word list, 
	message_id* out)
{
	char		thread_key[sizeof(lu_word) + 1];
	int		error;
	DBT		key;
	DBT		ind;
	
	memcpy(&thread_key[0], &list, sizeof(lu_word));
	thread_key[sizeof(lu_word)] = 0xFF; /* No record is this large */
	
	memset(&key, 0, sizeof(DBT));
	memset(&ind, 0, sizeof(DBT));
	
	key.data = &thread_key[0];
	key.size = sizeof(lu_word) + 1;
	ind.data = out;
	ind.ulen = sizeof(message_id);
	ind.flags = DB_DBT_USERMEM;
	
	error = my_summary_merge_db->get(
		my_summary_merge_db, 0, &key, &ind, 0);
	if (error && error != DB_NOTFOUND)
	{
		syslog(LOG_ERR, "Getting the last thread: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if (error == DB_NOTFOUND)
	{
		*out = lu_common_minvalid;
	}
	
	return 0;
}

/*------------------------------------------------- Import message methods */

message_id lu_summary_import_message(
	lu_word list, 
	lu_word mbox, 
	lu_addr mbox_offset,
	time_t timestamp, 
	const char* mmessage_id,
	const char* subject,
	const char* author_name,
	const char* author_email)
{
	/* This function has to create the message stub info:
	 * 
	 * First, find what thread to plop it in.
	 * 
	 * We must push the variable information for the message.
	 * Then we must push summary information for the message.
	 * Then we link it up with it's thread buddies.
	 */
	 
	off_t		sum_off;
	off_t		var_off;
	lu_addr		high_bits;
	size_t		mid_len, sub_len, aun_len, aue_len;
	
	message_id		id;
	Lu_Summary_Message	sum;
	message_id		prev_id;
	Lu_Summary_Message	prev;
	message_id		thread_id;
	Lu_Summary_Thread	thread;
	message_id		last_thread;
	
	char		thread_key[sizeof(lu_word) + LU_SQUISHY_MAX + sizeof(lu_quad)];
	int		thread_key_len;
	lu_quad		tm;
	DBT		key;
	DBT		ind;
	int		error;
	
	id = my_summary_msg_free;
	
	/* Check to see if we were already loaded */
	if (id > 0)
	{
		sum_off = id - 1;
		sum_off *= sizeof(Lu_Summary_Message);
		if (lseek(my_summary_message_fd, sum_off, SEEK_SET) != sum_off)
		{
			syslog(LOG_ERR, "Seeking for check of message summary: %s\n",
				strerror(errno));
			goto lu_summary_import_message_error0;
		}
		
		if (read(my_summary_message_fd, &sum, sizeof(Lu_Summary_Message))
			!= sizeof(Lu_Summary_Message))
		{
			syslog(LOG_ERR, "Checking for dup message summary: %s\n",
				strerror(errno));
			goto lu_summary_import_message_error0;
		}
		
		high_bits = mbox;
		high_bits <<= (sizeof(lu_addr)-2)*8;
		high_bits |= mbox_offset;
		sum.flat_offset >>= (sizeof(lu_addr)-2)*8;
		
		/* So, have we been imported already? */
		if (sum.mbox_offset == high_bits && sum.flat_offset == list)
		{
			return id;
		}
	}
	
	if (subject)
	{
		/* Start the import... What thread to put it in? */
		if (my_summary_find_thread(list, subject, timestamp,
			&thread, &thread_id) != 0)
		{
			goto lu_summary_import_message_error0;
		}
	}
	else
	{
		thread_id = lu_common_minvalid;
	}
	
	/* Start writing variable length data */
	var_off = lseek(my_summary_variable_fd, 0, SEEK_END);
	if (var_off == -1)
	{
		syslog(LOG_ERR, "Could not seek to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_summary_import_message_error0;
	}
	
	mid_len = strlen(mmessage_id) + 1;
	if (write(my_summary_variable_fd, mmessage_id, mid_len) != mid_len)
	{
		syslog(LOG_ERR, "Could not write message id to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	if (!subject) subject = "";
	sub_len = strlen(subject) + 1;
	if (write(my_summary_variable_fd, subject, sub_len) != sub_len)
	{
		syslog(LOG_ERR, "Could not write subject to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	aun_len = strlen(author_name) + 1;
	if (write(my_summary_variable_fd, author_name, aun_len) != aun_len)
	{
		syslog(LOG_ERR, "Could not write author name to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	aue_len = strlen(author_email) + 1;
	if (write(my_summary_variable_fd, author_email, aue_len) != aue_len)
	{
		syslog(LOG_ERR, "Could not write author email to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	sum.mbox_offset   = mbox_offset;
 	sum.flat_offset   = var_off;
	sum.timestamp     = timestamp;
	sum.in_reply_to   = lu_common_minvalid;	/* nothing yet */
	sum.thread_next   = lu_common_minvalid;	/* we are at the end */
	sum.thread_parent = (thread_id==lu_common_minvalid)?id:thread_id;
	
	high_bits = mbox;
	high_bits <<= (sizeof(lu_addr)-2)*8;
	sum.mbox_offset |= high_bits;
	
	high_bits = list;
	high_bits <<= (sizeof(lu_addr)-2)*8;
	sum.flat_offset |= high_bits;
	
	sum_off = id;
	sum_off *= sizeof(Lu_Summary_Message);
	if (lseek(my_summary_message_fd, sum_off, SEEK_SET) != sum_off)
	{
		syslog(LOG_ERR, "Seeking for write of message summary: %s\n",
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	if (write(my_summary_message_fd, &sum, sizeof(Lu_Summary_Message)) != 
		sizeof(Lu_Summary_Message))
	{
		syslog(LOG_ERR, "Writing message summary: %s\n",
			strerror(errno));
		goto lu_summary_import_message_error2;
	}
	
	if (thread_id == lu_common_minvalid)
	{
		prev_id   		= lu_common_minvalid;
		thread.thread_end	= id; /* we are the last record too */
		
		thread.start = timestamp;
		thread.end   = timestamp;
		thread.message_count = 1;
		
		thread_id = id;
		
		/* Get the key for where this thread lies */
		tm = timestamp;
		thread_key_len = sizeof(lu_word);
		memcpy(&thread_key[0], &list, thread_key_len);
		thread_key_len += my_summary_squishy_subject(subject, &thread_key[thread_key_len]);
		memcpy(&thread_key[thread_key_len], &tm, sizeof(lu_quad));
		thread_key_len += sizeof(lu_quad);
		
		memset(&key, 0, sizeof(DBT));
		memset(&ind, 0, sizeof(DBT));
		
		key.data = &thread_key[0];
		key.size = thread_key_len;
		ind.data = &id;
		ind.size = sizeof(message_id);
		
		if (lu_summary_last_thread(list, &last_thread) != 0)
		{
			goto lu_summary_import_message_error2;
		}
		
		error = my_summary_merge_db->put(
			my_summary_merge_db, 0, &key, &ind, 0);
		if (error)
		{
			syslog(LOG_ERR, "Failed to push indirect thread reference: %s\n",
				db_strerror(errno));
			goto lu_summary_import_message_error2;
		}
		
		/* Push the thread to the end of the list, simultaneously
		 * writing out our changes.
		 */
		if (my_summary_push_thread(list, last_thread, thread_id, &thread) != 0)
			goto lu_summary_import_message_error3;
	}
	else
	{
		/* Try to retarget the thread next pointer of the other guy
		 */
		prev_id = thread.thread_end;
		sum_off = prev_id;
		sum_off *= sizeof(Lu_Summary_Message);
		if (lseek(my_summary_message_fd, sum_off, SEEK_SET) != sum_off)
		{
			syslog(LOG_ERR, "Seeking for read of prior link summary: %s\n",
				strerror(errno));
			goto lu_summary_import_message_error2;
		}
		
		if (read(my_summary_message_fd, &prev, sizeof(Lu_Summary_Message)) != sizeof(Lu_Summary_Message))
		{
			syslog(LOG_ERR, "Reading of prior link summary: %s\n",
				strerror(errno));
			goto lu_summary_import_message_error2;
		}
		
		if (lseek(my_summary_message_fd, sum_off, SEEK_SET) != sum_off)
		{
			syslog(LOG_ERR, "Seeking for write of prior link summary: %s\n",
				strerror(errno));
			goto lu_summary_import_message_error2;
		}
		
		prev.thread_next = id;
		
		if (write(my_summary_message_fd, &prev, sizeof(Lu_Summary_Message)) != sizeof(Lu_Summary_Message))
		{
			syslog(LOG_ERR, "Writing of prior link summary: %s\n",
				strerror(errno));
			goto lu_summary_import_message_error3;
		}
		
		/* Ok, now we need to update the threading record itself.
		 */
		
		thread.thread_end = id;
		thread.message_count++;
		
		/* we import in order so we are the newest message in this
		 * thread and this thread is the newest in the mbox
		 */
		thread.end = timestamp;
		
		if (lu_summary_last_thread(list, &last_thread) != 0)
		{
			goto lu_summary_import_message_error3;
		}
		
		if (last_thread == thread_id)
		{	/* We're being popped, it's the guy before us */
			last_thread = thread.prev_thread;
		}
		
		/* Pop the thread's current location */
		if (pop_thread(thread_id, &thread) != 0)
			goto lu_summary_import_message_error3;
			
		/* Push the thread to the end of the list, simultaneously
		 * writing out our changes.
		 */
		if (my_summary_push_thread(list, last_thread, thread_id, &thread) != 0)
			goto lu_summary_import_message_error4;
	}
	
	my_summary_msg_free++;
	my_summary_last_time = timestamp;
	
	return id;

lu_summary_import_message_error4:
	/* somehow repush the thread into it's context
	 * not sure how to do this. not sure that it matters.
	 * when the message finally is imported, this thread will be pushed
	 * to the end at that point. till then it's missing, but whatever
	 * !!!
	 */

lu_summary_import_message_error3:
	/* Well, we could change the thread_next ptr on prev_id at this point.
	 * Indeed, I will restore it, but it's not critical; the thread_end
	 * record still remains unmoved and we should be terminating loops
	 * with that anyways: it would continue to work.
	 * Still: consistency is good.
	 */
	
	if (prev_id != lu_common_minvalid)
	{
		prev.thread_next = lu_common_minvalid;
		
		sum_off = prev_id;
		sum_off *= sizeof(Lu_Summary_Message);
		if (lseek(my_summary_message_fd, sum_off, SEEK_SET) != sum_off)
		{
			syslog(LOG_WARNING, "Unable to seek to restore next thread entry: %s\n",
				strerror(errno));
		}
		else
		{
			if (write(my_summary_message_fd, &prev, sizeof(Lu_Summary_Thread)) != 
				sizeof(Lu_Summary_Thread))
			{
				syslog(LOG_WARNING, "Unable to write to restore next thread entry: %s\n",
					strerror(errno));
			}
		}
	}
	else
	{
	
	/* The other case is that we need to yank out a record which we placed
	 * in the btree to locate the thread hash entry.
	 */
		error = my_summary_merge_db->del(
			my_summary_merge_db, 0, &key, 0);
		if (error)
		{
			syslog(LOG_ERR, "Failed to remove indirect thread reference: %s\n",
				db_strerror(errno));
		}
	}
	
lu_summary_import_message_error2:
	sum_off = id;
	sum_off *= sizeof(Lu_Summary_Message);
	if (ftruncate(my_summary_message_fd, sum_off) != 0)
	{
		syslog(LOG_ERR, "Unable to reclaim space in summary.flat on failed lu_summary_import_message: %s\n",
			strerror(errno));
	}
	
lu_summary_import_message_error1:
	if (ftruncate(my_summary_variable_fd, var_off) != 0)
	{
		syslog(LOG_ERR, "Unable to reclaim space in variable.flat on failed lu_summary_import_message: %s\n",
			strerror(errno));
	}
	
lu_summary_import_message_error0:
	return lu_common_minvalid;
}

int lu_summary_reply_to_resolution(
	message_id id,
	const char* msg_id,
	const char* reply_to_msg_id)
{
	/* Then comes reply-to resolution:
	 * 
	 * Has something replied to us? We push a keyword for our own
	 * message id. Then we run a query to see if an messages are in
	 * reply-to our message-id. For each of them, we update their 
	 * reply-to link in the summary.
	 * 
	 * Have we replied to something? We push a keyword for what we
	 * reply-to. Then we so a query to see if it exists. If it does,
	 * we update our reply-to summary informationn.
	 */
	 
	 char			key[LU_KEYWORD_LEN+1];
	 message_id		buf[1024];
	 message_id		count, ind, get;
	 int			i;
	 Lu_Breader_Handle	h;
	 
	 if (*msg_id)
	 { 	/* Does anything already imported reply to us? */
	 	snprintf(&key[0], sizeof(key), "%s%s",
	 		LU_KEYWORD_REPLY_TO,
	 		msg_id);
	 	
	 	h = lu_breader_new(&key[0]);
	 	if (h != 0)
	 	{
	 		count = lu_breader_records(h);
	 		ind = 0;
	 		
	 		while (count)
	 		{
	 			get = count;
	 			if (get > sizeof(buf)/sizeof(message_id))
	 				get = sizeof(buf)/sizeof(message_id);
	 			
	 			if (lu_breader_read(h, ind, get, &buf[0]) != 0)
	 			{
	 				lu_breader_free(h);
	 				return -1;
	 			}
	 			
	 			for (i = 0; i < get; i++)
	 				my_summary_reply_link(buf[i], id);
	 			
	 			ind   += get;
	 			count -= get;
	 		}
	 		
	 		lu_breader_free(h);
	 	}
	 }
	 
	 if (*reply_to_msg_id)
	 {	/* Do we reply to anything? */
	 	snprintf(&key[0], sizeof(key), "%s%s",
	 		LU_KEYWORD_MESSAGE_ID,
	 		reply_to_msg_id);
	 	
	 	h = lu_breader_new(&key[0]);
	 	if (h != 0)
	 	{
	 		if (lu_breader_records(h) >= 1)
	 		{	/* We will mark ourselves as only replying
	 			 * to the first message with a given message
	 			 * id. -- It DOES happen. *grumble*
	 			 */
	 			 
	 			if (lu_breader_read(h, 0, 1, &buf[0]) != 0)
	 			{
	 				lu_breader_free(h);
	 				return -1;
	 			}
	 			
	 			my_summary_reply_link(id, buf[0]);
	 		}
	 		
	 		lu_breader_free(h);
	 	}
	 }
	 
	 /*!!! If we have the same squishy subject as the target, then
	  * all threads with that squishy subject and ourselves should
	  * be merged. Same for stuff targetting us.
	  */
	 
	 return -1;
}

/*------------------------------------------------ Public component methods */

int lu_summary_init()
{
	return 0;
}

int lu_summary_open()
{
	int	error;
	off_t	offset;
	
	my_summary_message_fd = open("summary.flat", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (my_summary_message_fd == -1)
	{
		perror("summary.flat");
		return -1;
	}
	
	my_summary_variable_fd = open("variable.flat", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (my_summary_variable_fd == -1)
	{
		perror("variable.flat");
		return -1;
	}
	
	if ((error = db_create(&my_summary_thread_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&my_summary_merge_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_summary_thread_db->open(
		my_summary_thread_db, "thread.hash", 0,
		DB_HASH, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: thread.hash: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_summary_merge_db->open(
		my_summary_merge_db, "merge.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: merge.btree: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((offset = lseek(my_summary_message_fd, 0, SEEK_END)) == -1)
	{
		fprintf(stderr, "Determining last message id: merge.btree: %s\n",
			db_strerror(error));
		return -1;
	}
	
	offset /= sizeof(Lu_Summary_Message);
	my_summary_msg_free  = offset;
	
	/* lookup lu_last_time from summary info */
	if (my_summary_msg_free == 0)
	{	/* We use zero as an error, so start at 1971 for epoch */
		my_summary_last_time = 31536000;
	}
	else
	{
		Lu_Summary_Message sum = lu_summary_read_msummary(
			my_summary_msg_free - 1);
		if (sum.timestamp == 0)
		{
			perror("Grabbing last summary block");
			return -1;
		}
		
		my_summary_last_time = sum.timestamp;
	}
	
	return 0;
}

int lu_summary_sync()
{
	int error;
	
	if ((error = my_summary_thread_db->sync(
		my_summary_thread_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: thread.hash: %s\n",
			db_strerror(error));
	}
	
	if ((error = my_summary_merge_db->sync(
		my_summary_merge_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: merge.btree: %s\n",
			db_strerror(error));
	}
	
	return 0;
}

int lu_summary_close()
{
	int error;
	int fail = 0;
	
	if ((error = my_summary_thread_db->close(
		my_summary_thread_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: thread.hash: %s\n",
			db_strerror(error));
		fail = -1;
	}
	
	if ((error = my_summary_merge_db->close(
		my_summary_merge_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: merge.btree: %s\n",
			db_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_summary_quit()
{
	return 0;
}
