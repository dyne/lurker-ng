/*  $Id: db.c,v 1.12 2002-01-31 05:42:40 terpstra Exp $
 *  
 *  db.c - manage the databases
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

#include "keyword.h"
#include "globals.h"
#include "io.h"

#include <string.h>
#include <ctype.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#else
#define syslog(x, y, ...)
#endif

int	lu_summary_fd;
int	lu_variable_fd;
DB_ENV*	lu_db_env;
DB*	lu_thread_db;
DB*	lu_merge_db;
DB*	lu_mbox_db;

static message_id	lu_msg_free;
static time_t		lu_last_time;

MessageSummary lu_read_msummary(message_id mid)
{
	MessageSummary	out;
	off_t		offset = mid;
	
	offset *= sizeof(MessageSummary);
	if (lseek(lu_summary_fd, offset, SEEK_SET) != offset)
	{
		memset(&out, 0, sizeof(MessageSummary));
		return out;
	}
	
	if (read(lu_summary_fd, &out, sizeof(MessageSummary)) != 
		sizeof(MessageSummary))
	{
		memset(&out, 0, sizeof(MessageSummary));
		return out;
	}
	
	return out;
}

static int lu_path_compress(message_id* mid)
{
	message_id	bak;
	off_t		offset;
	
	/* First, follow the union-find trail and path compress as we do so. */
	MessageSummary	sum = lu_read_msummary(*mid);
	
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
	if (lu_path_compress(&sum.thread_parent) != 0)
	{	/* Something f*ked up, pass the error down */
		return -1;
	}
	
	/* Was the path compressed? Update if so. */
	if (sum.thread_parent != bak)
	{
		offset = *mid;
		offset *= sizeof(MessageSummary);
		
		/* If we can't rewrite, no big deal, we don't HAVE to 
		 * do the path compression.
		 */
		if (lseek(lu_summary_fd, offset, SEEK_SET) != offset ||
		    write(lu_summary_fd, &sum, sizeof(MessageSummary)) !=
		    	sizeof(MessageSummary))
		{
			syslog(LOG_WARNING, 
				"Could not compress thread path for message %d\n",
				*mid);
		}
	}
	
	*mid = sum.thread_parent;
	return 0;
}

ThreadSummary lu_read_tsummary(message_id tid)
{
	ThreadSummary out;
	
	DBT key;
	DBT val;
	
	int error;
	
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	/* Find the actual thread head.
	 */
	if (lu_path_compress(&tid) != 0)
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
	if ((error = lu_thread_db->get(lu_thread_db, 0, &key, &val, 0)) != 0)
	{
		syslog(LOG_ERR, "Thread-head %d does not have summary record\n",
			tid);
		memset(&out, 0, sizeof(out));
	}
	
	return out;
}

int lu_write_variable(FILE* out, lu_addr flat_offset)
{
	char	buf[4096];
	char*	w;
	char*	e;
	int	nulls;
	int	got;
	
	off_t offset = flat_offset;
	
	if (lseek(lu_variable_fd, offset, SEEK_SET) != offset)
	{
		return -1;
	}
	
	nulls = 0;
	while (nulls < 3)
	{
		got = read(lu_variable_fd, &buf[0], sizeof(buf));
		if (got <= 0) break;
		e = &buf[got];
		
		for (w = &buf[0]; w != e;)
			if (!*w++ && ++nulls == 3)
				break;
		
		fwrite(&buf[0], 1, (w - &buf[0]), out);
	}
	
	if (nulls != 3) return -1;
	
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
time_t lu_timestamp_heuristic(time_t server, time_t client)
{
	time_t out;
	
	if (client > server || server - 3*60*60*24 > client)
		out = server;
	else
		out = client;
	
	/* Before one we've already archived? Not possible. */
	if (out < lu_last_time)
	{
		out = lu_last_time;
	}
	
	return out;
}

#define LU_SQUISHY_MAX	80

static int lu_squishy_subject(const char* subject, char* target)
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
			if (ws)
			{
				*w++ = ' ';
				ws = 0;
				
				/* Need to retest since we are doing a double
				 * write
				 */
				if (target == e) break;
			}
			
			if (*r == ':')
			{	/* Nasty colons! */
				break;
			}
			
			if (isalnum(*r))
			{
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
static int pop_thread(message_id thread_id, ThreadSummary* thread)
{
	int		error;
	DBT		key;
	DBT		val;
	ThreadSummary	left;
	ThreadSummary	right;
	
	key.size = sizeof(message_id);
	val.ulen = sizeof(left);
	val.flags = DB_DBT_USERMEM;
	
	if (thread->prev_thread != lu_kw_invalid)
	{
		key.data = &thread->prev_thread;
		val.data = &left;
	
		if ((error = lu_thread_db->get(lu_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Thread-linkage corrupt - left missing for %d\n",
				thread_id);
			return -1;
		}
		
		left.next_thread = thread->next_thread;
	}
	
	if (thread->next_thread != lu_kw_invalid)
	{
		key.data = &thread->next_thread;
		val.data = &right;
		
		if ((error = lu_thread_db->get(lu_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Thread-linkage corrupt - right missing for %d\n",
				thread_id);
			return -1;
		}
		
		right.prev_thread = thread->prev_thread;
	}
	
	if (thread->prev_thread != lu_kw_invalid)
	{
		key.data = &thread->prev_thread;
		val.data = &left;
	
		if ((error = lu_thread_db->put(lu_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Unable to rewrite left link for %d\n",
				thread_id);
			return -1;
		}
	}
	
	if (thread->next_thread != lu_kw_invalid)
	{
		key.data = &thread->next_thread;
		val.data = &right;
		
		if ((error = lu_thread_db->put(lu_thread_db, 0, &key, &val, 0)) != 0)
		{
			syslog(LOG_ERR, "Thread-linkage corrupt - right missing for %d\n",
				thread_id);
			
			if (thread->prev_thread != lu_kw_invalid)
			{
				left.next_thread = thread_id;
				key.data = &thread->prev_thread;
				val.data = &left;
				
				if ((error = lu_thread_db->put(lu_thread_db, 0, &key, &val, 0)) != 0)
				{
					syslog(LOG_ERR, "Unable to restore left link for %d - broken linkage!\n",
						thread_id);
				}
			}
			
			return -1;
		}
	}
	
	return 0;
}

/* Append the thread to the linked list of threads in the hash.
 * The passed thread is assumed to be the correct record and is rewritten with
 * appropriate link information.
 */
static int push_thread(message_id thread_id, ThreadSummary* thread)
{
	/*!!! how do we find the end?*/
	return 0;
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
 * lu_keyword_invalid -> no thread existed, make your own.
 * 
 * The caller is responsible for either setting his thread_parent to the
 * returned id if != lu_keyword_invalid. Also, he must insert himself into
 * the linked list for the thread. The parent summary is returned as a 
 * convenience (less disk hits). message_count++ is a good move too.
 */
static int lu_find_thread(
	lu_word list, const char* subject, time_t timestamp,
	ThreadSummary* parent, message_id* out)
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
	ThreadSummary	prior;
	message_id	prior_id;
	lu_quad		prior_time;
	
	/* Get the key for where this thread lies */
	tm = timestamp;
	thread_key_len = sizeof(lu_word);
	memcpy(&thread_key[0], &list, thread_key_len);
	thread_key_len += lu_squishy_subject(subject, &thread_key[thread_key_len]);
	memcpy(&thread_key[thread_key_len], &tm, sizeof(lu_quad));
	thread_key_len += sizeof(lu_quad);
	
	squishy_len = lu_squishy_subject(subject, &squishy[0]);
	
	/* Find the smallest entry >= our thread in the thread btree */
	error = lu_merge_db->cursor(lu_merge_db, 0, &cursor, 0);
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
	val.ulen = sizeof(ThreadSummary);
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
	
	error = cursor->c_close(cursor);
	if (error)
	{
		syslog(LOG_ERR, "Unable to close thread cursor: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if (error == DB_NOTFOUND ||
	    memcmp(&squishy[0], &thread_key[sizeof(lu_word)], squishy_len))
	{	/* There is no prior thread */
		*out = lu_kw_invalid;
		return 0;
	}
	else
	{
		error = lu_thread_db->get(lu_thread_db, 0, &ind, &val, 0);
		if (error)
		{
			syslog(LOG_ERR, "Unable to retrieve thread summary: %s\n",
				db_strerror(error));
			return -1;
		}
		
		memcpy(&prior_time, &thread_key[key.size-sizeof(lu_quad)], 
			sizeof(lu_quad));
	}
	
	/* Ok, we now have the data loaded, do a quick consistency check */
	if (prior_time != prior.start || prior.start > prior.end)
	{
		syslog(LOG_ERR, "Corrupt thread database - prior\n");
		return -1;
	}
	
	/* Now will the new message land in the prior thread? */
	if (timestamp + LU_THREAD_OVERLAP_TIME > prior.start &&
	    prior.end + LU_THREAD_OVERLAP_TIME > timestamp)
	{	/* Ok, merge with the prior thread and not the later */
		memcpy(parent, &prior, sizeof(ThreadSummary));
		*out = prior_id;
		return 0;
	}
	
	/* New thread for this record */
	*out = lu_kw_invalid;
	return 0;
}

message_id lu_import_message(
	lu_word list, lu_word mbox, lu_addr mbox_offset,
	time_t timestamp, 
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
	size_t		sub_len, aun_len, aue_len;
	message_id	id;
	MessageSummary	sum;
	message_id	prev_id;
	MessageSummary	prev;
	message_id	thread_id;
	ThreadSummary	thread;
	char		thread_key[sizeof(lu_word) + LU_SQUISHY_MAX + sizeof(lu_quad)];
	int		thread_key_len;
	lu_quad		tm;
	DBT		key;
	DBT		ind;
	int		error;
	
	/*!!! check for already loaded status */
	
	id = lu_msg_free;
	
	if (lu_find_thread(list, subject, timestamp,
		&thread, &thread_id) != 0)
	{
		goto lu_import_message_error0;
	}
	
	var_off = lseek(lu_summary_fd, 0, SEEK_END);
	if (var_off == -1)
	{
		syslog(LOG_ERR, "Could not seek to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_import_message_error0;
	}
	
	sub_len = strlen(subject) + 1;
	if (write(lu_summary_fd, subject, sub_len) != sub_len)
	{
		syslog(LOG_ERR, "Could not write subject to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_import_message_error1;
	}
	
	aun_len = strlen(author_name) + 1;
	if (write(lu_summary_fd, author_name, aun_len) != aun_len)
	{
		syslog(LOG_ERR, "Could not write author name to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_import_message_error1;
	}
	
	aue_len = strlen(author_email) + 1;
	if (write(lu_summary_fd, author_email, aue_len) != aue_len)
	{
		syslog(LOG_ERR, "Could not write author email to end of variable.flat: %s\n",
			strerror(errno));
		goto lu_import_message_error1;
	}
	
	sum.mbox_offset   = mbox_offset;
 	sum.flat_offset   = var_off;
	sum.timestamp     = timestamp;
	sum.in_reply_to   = lu_kw_invalid;	/* nothing yet */
	sum.thread_next   = lu_kw_invalid;	/* we are at the end */
	sum.thread_parent = (thread_id==lu_kw_invalid)?id:thread_id;
	
	high_bits = mbox;
	high_bits <<= (sizeof(lu_addr)-2)*8;
	sum.mbox_offset |= high_bits;
	
	high_bits = list;
	high_bits <<= (sizeof(lu_addr)-2)*8;
	sum.flat_offset |= high_bits;
	
	sum_off = id;
	sum_off *= sizeof(MessageSummary);
	if (lseek(lu_summary_fd, sum_off, SEEK_SET) != sum_off)
	{
		syslog(LOG_ERR, "Seeking for write of message summary: %s\n",
			strerror(errno));
		goto lu_import_message_error1;
	}
	
	if (write(lu_summary_fd, &sum, sizeof(MessageSummary)) != 
		sizeof(MessageSummary))
	{
		syslog(LOG_ERR, "Writing message summary: %s\n",
			strerror(errno));
		goto lu_import_message_error2;
	}
	
	if (thread_id == lu_kw_invalid)
	{
		prev_id   = lu_kw_invalid;
		thread.thread_end  = id; /* we are the last record too */
		
		thread.start = timestamp;
		thread.end   = timestamp;
		thread.message_count = 1;
		
		thread_id = id;
		
		/* Get the key for where this thread lies */
		tm = timestamp;
		thread_key_len = sizeof(lu_word);
		memcpy(&thread_key[0], &list, thread_key_len);
		thread_key_len += lu_squishy_subject(subject, &thread_key[thread_key_len]);
		memcpy(&thread_key[thread_key_len], &tm, sizeof(lu_quad));
		thread_key_len += sizeof(lu_quad);
		
		memset(&key, 0, sizeof(DBT));
		memset(&ind, 0, sizeof(DBT));
		
		key.data = &thread_key[0];
		key.size = thread_key_len;
		ind.ulen = sizeof(message_id);
		ind.size = DB_DBT_USERMEM;
		ind.data = &id;
		
		error = lu_merge_db->put(lu_merge_db, 0, &key, &ind, 0);
		if (error)
		{
			syslog(LOG_ERR, "Failed to push indirect thread reference: %s\n",
				db_strerror(errno));
		}
		
		/* Push the thread to the end of the list, simultaneously
		 * writing out our changes.
		 */
		if (push_thread(thread_id, &thread) != 0)
			goto lu_import_message_error3;
	}
	else
	{
		/* Try to retarget the thread next pointer of the other guy
		 */
		prev_id = thread.thread_end;
		sum_off = prev_id;
		sum_off *= sizeof(MessageSummary);
		if (lseek(lu_summary_fd, sum_off, SEEK_SET) != sum_off)
		{
			syslog(LOG_ERR, "Seeking for read of prior link summary: %s\n",
				strerror(errno));
			goto lu_import_message_error2;
		}
		
		if (read(lu_summary_fd, &prev, sizeof(MessageSummary)) != sizeof(MessageSummary))
		{
			syslog(LOG_ERR, "Reading of prior link summary: %s\n",
				strerror(errno));
			goto lu_import_message_error2;
		}
		
		if (lseek(lu_summary_fd, sum_off, SEEK_SET) != sum_off)
		{
			syslog(LOG_ERR, "Seeking for write of prior link summary: %s\n",
				strerror(errno));
			goto lu_import_message_error2;
		}
		
		prev.thread_next = id;
		
		if (write(lu_summary_fd, &prev, sizeof(MessageSummary)) != sizeof(MessageSummary))
		{
			syslog(LOG_ERR, "Writing of prior link summary: %s\n",
				strerror(errno));
			goto lu_import_message_error3;
		}
		
		/* Ok, now we need to update the threading record itself.
		 */
		
		thread.thread_end = id;
		thread.message_count++;
		
		/* we import in order so we are the newest message in this
		 * thread and this thread is the newest in the mbox
		 */
		thread.end = timestamp;
		
		/* Pop the thread's current location */
		if (pop_thread(thread_id, &thread) != 0)
			goto lu_import_message_error3;
		
		/* Push the thread to the end of the list, simultaneously
		 * writing out our changes.
		 */
		if (push_thread(thread_id, &thread) != 0)
			goto lu_import_message_error4;
	}
	
	return id;

lu_import_message_error4:
	/* somehow repush the thread into it's context
	 * not sure how to do this. not sure that it matters.
	 * when the message finally is imported, this thread will be pushed
	 * to the end at that point. till then it's missing, but whatever
	 * !!!
	 */

lu_import_message_error3:
	/* Well, we could change the thread_next ptr on prev_id at this point.
	 * Indeed, I will restore it, but it's not critical; the thread_end
	 * record still remains unmoved and we should be terminating loops
	 * with that anyways: it would continue to work.
	 * Still: consistency is good.
	 */
	
	if (prev_id != lu_kw_invalid)
	{
		prev.thread_next = lu_kw_invalid;
		
		sum_off = prev_id;
		sum_off *= sizeof(MessageSummary);
		if (lseek(lu_summary_fd, sum_off, SEEK_SET) != sum_off)
		{
			syslog(LOG_WARNING, "Unable to seek to restore next thread entry: %s\n",
				strerror(errno));
		}
		else
		{
			if (write(lu_summary_fd, &prev, sizeof(ThreadSummary)) != 
				sizeof(ThreadSummary))
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
		error = lu_merge_db->del(lu_merge_db, 0, &key, 0);
		if (error)
		{
			syslog(LOG_ERR, "Failed to remove indirect thread reference: %s\n",
				db_strerror(errno));
		}
	}
	
lu_import_message_error2:
	sum_off = id;
	sum_off *= sizeof(MessageSummary);
	if (ftruncate(lu_summary_fd, sum_off) != 0)
	{
		syslog(LOG_ERR, "Unable to reclaim space in summary.flat on failed lu_import_message: %s\n",
			strerror(errno));
	}
	
lu_import_message_error1:
	if (ftruncate(lu_variable_fd, var_off) != 0)
	{
		syslog(LOG_ERR, "Unable to reclaim space in variable.flat on failed lu_import_message: %s\n",
			strerror(errno));
	}
	
lu_import_message_error0:
	return lu_kw_invalid;
}

int lu_reply_to_resolution(
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
	 
	 return -1;
}

int lu_open_db()
{
	int	error;
	off_t	offset;
	
	lu_summary_fd = open("summary.flat", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (lu_summary_fd == -1)
	{
		perror("summary.flat");
		return -1;
	}
	
	lu_variable_fd = open("variable.flat", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (lu_variable_fd == -1)
	{
		perror("variable.flat");
		return -1;
	}
	
	lu_keyword_fd = open("keyword.flat", 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (lu_keyword_fd == -1)
	{
		perror("keyword.flat");
		return -1;
	}
	
	if ((error = db_env_create(&lu_db_env, 0)) != 0)
	{
		fprintf(stderr, "Creating db3 environment: %s\n", 
			db_strerror(error));
		return -1;
	}
	
	/* Give ourselves a special and unique shared memory key */
	if ((error = lu_db_env->set_shm_key(lu_db_env, 0x4E12DA03UL)) != 0)
	{
		fprintf(stderr, "Setting db3 shared memory key: %s\n", 
			db_strerror(error));
		return -1;
	}
	
	/* Open the environment */
	if ((error = lu_db_env->open(lu_db_env, lu_dbdir, 
		DB_INIT_MPOOL |	DB_RECOVER |
		DB_CREATE | DB_SYSTEM_MEM,
		LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening the db3 environment: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&lu_thread_db, lu_db_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&lu_merge_db, lu_db_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&lu_mbox_db, lu_db_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&lu_keyword_db, lu_db_env, 0)) != 0)
	{
		fprintf(stderr, "Creating a db3 database: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = lu_thread_db->open(lu_thread_db, "thread.hash", 0,
		DB_HASH, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: thread.hash: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = lu_merge_db->open(lu_merge_db, "merge.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: merge.btree: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = lu_mbox_db->open(lu_mbox_db, "mbox.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: mbox.btree: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = lu_keyword_db->open(lu_keyword_db, "keyword.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: keyword.btree: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((offset = lseek(lu_summary_fd, 0, SEEK_END)) == -1)
	{
		fprintf(stderr, "Determining last message id: merge.btree: %s\n",
			db_strerror(error));
		return -1;
	}
	
	offset /= sizeof(MessageSummary);
	lu_msg_free  = offset;
	
	/* lookup lu_last_time from summary info */
	if (lu_msg_free == 0)
	{	/* We use zero as an error, so start at 1971 for epoch */
		lu_last_time = 31536000;
	}
	else
	{
		MessageSummary sum = lu_read_msummary(lu_msg_free - 1);
		if (sum.timestamp == 0)
		{
			perror("Grabbing last summary block");
			return -1;
		}
		
		lu_last_time = sum.timestamp;
	}
	
	return 0;
}

int lu_sync_db()
{
	int error;
	
	if ((error = lu_thread_db->sync(lu_thread_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: thread.hash: %s\n",
			db_strerror(error));
	}
	
	if ((error = lu_merge_db->sync(lu_merge_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: merge.btree: %s\n",
			db_strerror(error));
	}
	
	if ((error = lu_mbox_db->sync(lu_mbox_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: mbox.btree: %s\n",
			db_strerror(error));
	}
	
	if ((error = lu_keyword_db->sync(lu_keyword_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Syncing db3 database: keyword.btree: %s\n",
			db_strerror(error));
	}
	
	return 0;
}

int lu_close_db()
{
	int error;
	
	if ((error = lu_thread_db->close(lu_thread_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: thread.hash: %s\n",
			db_strerror(error));
	}
	
	if ((error = lu_merge_db->close(lu_merge_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: merge.btree: %s\n",
			db_strerror(error));
	}
	
	if ((error = lu_mbox_db->close(lu_mbox_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: mbox.btree: %s\n",
			db_strerror(error));
	}
	
	if ((error = lu_keyword_db->close(lu_keyword_db, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 database: keyword.btree: %s\n",
			db_strerror(error));
	}
	
	if ((error = lu_db_env->close(lu_db_env, 0)) != 0)
	{
		syslog(LOG_ERR, "Closing db3 environment: %s\n",
			db_strerror(error));
	}
	
	return 0;
}

int lu_sync_mbox()
{
	List*	list;
	Mbox*	mbox;
	
	MboxEatKey	key;
	MboxEatValue	val;
	
	DBT	dkey;
	DBT	dval;
	
	int error;
	int got;
	
	char buf[1024];
	off_t offset;
	
	for (list = lu_list; list != lu_list + lu_lists; list++)
	{
		for (mbox = list->mbox; mbox != list->mbox + list->mboxs; mbox++)
		{
			memset(&dkey, 0, sizeof(DBT));
			memset(&dval, 0, sizeof(DBT));
			
			dkey.data = &key;
			dkey.size = sizeof(key);
			
			dval.data = &val;
			dval.ulen = sizeof(val);
			dval.flags = DB_DBT_USERMEM;
			
			key.list = list->id;
			key.mbox = mbox->id;
			
			error = lu_mbox_db->get(lu_mbox_db, 0, &dkey, &dval, 0);
			
			if (error == 0)
			{
				got = read(mbox->fd, &buf[0], sizeof(buf));
				if (got < 0)
				{
					fprintf(stderr, "Couldn't read mbox %s: %s\n",
							mbox->path, strerror(errno));
					return -1;
				}
				
				if (got < sizeof(buf) && got < val.offset)
				{	/* They switched mboxs on us -- too small */
					fprintf(stderr, "Mbox id does not match prior version %s\n",
						mbox->path);
					return -1;
				}
				
				if (val.offset < got)
				{	/* Don't compare more than there was */
					got = val.offset;
				}
				
				if (memcmp(&val.front[0], &buf[0], got))
				{	/* The contents are changed */
					fprintf(stderr, "Mbox id does not match prior version %s\n",
						mbox->path);
					return -1;
				}
				
				offset = val.offset;
				if (lseek(mbox->fd, offset, SEEK_SET) 
					!= offset)
				{
					fprintf(stderr, "Could not seek %s: %s\n",
						mbox->path, strerror(errno));
					return -1;
				}
			}
			else if (error != DB_NOTFOUND)
			{
				fprintf(stderr, "Database failure mbox.btree: %s\n",
					db_strerror(error));
			}
		}
	}
	
	return 0;
}
