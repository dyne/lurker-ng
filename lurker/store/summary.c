/*  $Id: summary.c,v 1.22 2002-06-14 11:16:59 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "keyword.h"

#include "config.h"
#include "breader.h"
#include "summary.h"
#include "indexer.h"

#include <ctype.h>
#include <string.h>
#include <netinet/in.h> /* ntohl */

/*------------------------------------------------ Constant parameters */

#define LU_SQUISHY_MAX	80

/* By default, if two messages with similar subject arrive within 14 days
 * on the same mailing list, we consider them the same thread. If they have
 * the same squishy subject and a reply-to linkage, we may merge threads
 * at that time.
 */
#define LU_THREAD_OVERLAP_TIME	60*60*24*30

/*------------------------------------------------ Private types */

/* The squishy merge db has keys like:
 *  <squishy subject><timestamp>
 * And vals like:
 */
typedef struct My_Summary_SquishyVal_T
{
	message_id	thread;		/* thread head */
	lu_quad		end_time;	/* ending timestamp */
} My_Summary_SquishyVal;
#define LU_THREAD_KEY_MAX (LU_SQUISHY_MAX + sizeof(lu_quad))

/* The key for the offset db is simply the message id.
 */

/* The list of offsets of this message.
 */
typedef struct My_Summary_Offest_T
{
	lu_quad		list;
	message_id	offset;
} My_Summary_Offset;

/*------------------------------------------------ Private global vars */

static int my_summary_message_fd  = -1;
static int my_summary_variable_fd = -1;
static DB* my_summary_merge_db    = 0;
static DB* my_summary_mid_db      = 0;
static DB* my_summary_offset_db   = 0;

static message_id	my_summary_msg_free;
static time_t		my_summary_last_time;

/*------------------------------------------------ Private helper methods */

static int my_summary_squishy_subject(
	const char* subject, 
	char* target)
{
	/* Alright, we want to drop 're:', 'fwd:', etc.
	 * Also drop [...] chunks
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
	 * Also, any number of '\[[^\]]{0,16}\]' sequences
	 */
	state = 1;
	
	r = s = subject;
	while (*s)
	{
		if (state == 0)
		{	/* We are scanning a word that could be ...: */
			if (*s == '[' && s == r)
			{
				state = 2;
			}
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
		else if (state == 1)
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
		else
		{	/* We're skipping past a [...] */
			if (*s == ']')
			{
				state = 1;
			}
			else if (s - r > 16)
			{
				break;
			}
			
			s++;
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
				tolower(*(r-1)) == 's' &&
				tolower(*(r-2)) == 'a' &&
				tolower(*(r-3)) == 'w')
			{	/* Nasty colons! */
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

/* This function uses the given data to try and find a thread which the given
 * message would belong in. I initially thought that I would need to merge
 * threads. However, because we have guaranteed import order, a missing link
 * is an impossibility. It can only possibly connect to the previous thread.
 * 
 * The caller is responsible for setting his thread to the returned id.
 * Also, he must insert himself into the record for the thread.
 */
int my_summary_find_thread(
	message_id	id,
	const char*	subject, 
	time_t		timestamp,
	message_id*	out)
{
	char			thread_key    [LU_THREAD_KEY_MAX];
	char			thread_key_bak[LU_THREAD_KEY_MAX];
	int			thread_key_len;
	My_Summary_SquishyVal	squishy_val;
	
	lu_quad		tm;
	DBC*		cursor;
	DBT		key;
	DBT		val;
	int		error;
	lu_quad		prior_time;
	
	if (!subject || !*subject)
	{
		*out = id;
		return 0;
	}
	
	/* Get the key for where this thread lies */
	tm = timestamp;
	tm = htonl(tm);
	thread_key_len = 0;
	thread_key_len += my_summary_squishy_subject(subject, &thread_key[thread_key_len]);
	memcpy(&thread_key[thread_key_len], &tm, sizeof(lu_quad));
	thread_key_len += sizeof(lu_quad);
	
	memcpy(&thread_key_bak[0], &thread_key[0], thread_key_len);
	
	/* Find the smallest entry >= our thread in the thread btree */
	error = my_summary_merge_db->cursor(my_summary_merge_db, 0, &cursor, 0);
	if (error)
	{
		syslog(LOG_ERR, _("Unable to create thread DB cursor: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	key.ulen = sizeof(thread_key);
	key.flags = DB_DBT_USERMEM;
	val.ulen = sizeof(My_Summary_SquishyVal);
	val.flags = DB_DBT_USERMEM;
	
	key.data = &thread_key[0];
	key.size = thread_key_len;
	val.data = &squishy_val;
	
	error = cursor->c_get(cursor, &key, &val, DB_SET_RANGE);
	if (error && error != DB_NOTFOUND)
	{
		syslog(LOG_ERR, _("Unable to find next thread record: %s\n"),
			db_strerror(error));
		cursor->c_close(cursor);
		return -1;
	}
	
	if (error == DB_NOTFOUND)
		error = cursor->c_get(cursor, &key, &val, DB_LAST);
	else if (thread_key_len != key.size || 
	         memcmp(&thread_key[0], &thread_key_bak[0], key.size - sizeof(lu_quad)))
		error = cursor->c_get(cursor, &key, &val, DB_PREV);
	
	if (error && error != DB_NOTFOUND)
	{
		syslog(LOG_ERR, _("Unable to find prev thread record: %s\n"),
			db_strerror(error));
		cursor->c_close(cursor);
		return -1;
	}
	
	if (error == DB_NOTFOUND ||
	    thread_key_len != key.size ||
	    memcmp(&thread_key[0], &thread_key_bak[0], 
	           key.size - sizeof(lu_quad)))
	{	/* There is no prior thread */
		prior_time = 0;
	}
	else
	{
		memcpy(&prior_time, &thread_key[key.size-sizeof(lu_quad)], 
			sizeof(lu_quad));
		prior_time = ntohl(prior_time);
		
		/* Ok, we now have the data loaded, do a quick consistency check */
		if (prior_time > squishy_val.end_time || 
		     timestamp < squishy_val.end_time ||
		     val.size != sizeof(My_Summary_SquishyVal))
		{
			syslog(LOG_ERR, _("Corrupt thread database - prior (%d %d %d)\n"),
				prior_time, squishy_val.end_time, (int)timestamp);
			cursor->c_close(cursor);
			return -1;
		}
	}
	
	error = cursor->c_close(cursor);
	if (error)
	{
		syslog(LOG_ERR, _("Unable to close thread cursor: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	/* Now will the new message require a new thread? */
	if (prior_time == 0 || 
	    squishy_val.end_time + LU_THREAD_OVERLAP_TIME < timestamp)
	{
		memcpy(&thread_key[0], &thread_key_bak[0], thread_key_len);
		squishy_val.thread = id;
	}
	
	squishy_val.end_time = timestamp;
	val.size = sizeof(My_Summary_SquishyVal);
	key.size = thread_key_len;
	
	error = my_summary_merge_db->put(
		my_summary_merge_db, 0, &key, &val, 0);
	if (error)
	{
		syslog(LOG_ERR, _("Unable to write message squishy: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	/*!!! Should probably clean out the database of records that get too
	 * old since they will never be needed again later...
	 */
	
	/* Record the thread this is a member of */
	memcpy(out, &squishy_val.thread, sizeof(message_id));
	return 0;
}

static int my_summary_reply_link(
	message_id src, 
	message_id dest)
{
	off_t			soffset = src;
	off_t			doffset = dest;
	Lu_Summary_Message	ssum = lu_summary_read_msummary(src);
	Lu_Summary_Message	dsum = lu_summary_read_msummary(dest);
	
	if (ssum.timestamp == 0 || dsum.timestamp == 0)
	{
		return -1;
	}
	
	ssum.in_reply_to = dest;
	dsum.replies++;
	
	soffset *= sizeof(Lu_Summary_Message);
	if (lseek(my_summary_message_fd, soffset, SEEK_SET) != soffset ||
	    write(my_summary_message_fd, &ssum, sizeof(Lu_Summary_Message)) !=
	    	sizeof(Lu_Summary_Message))
	{
		syslog(LOG_WARNING, 
			_("Could not store in-reply-to link: %d -> %d\n"),
			src, dest);
		return -1;
	}
	
	doffset *= sizeof(Lu_Summary_Message);
	if (lseek(my_summary_message_fd, doffset, SEEK_SET) != doffset ||
	    write(my_summary_message_fd, &dsum, sizeof(Lu_Summary_Message)) !=
	    	sizeof(Lu_Summary_Message))
	{
		syslog(LOG_WARNING, 
			_("Could increment replies: %d -> %d\n"),
			src, dest);
		return -1;
	}
	
	return 0;
}

/*------------------------------------------------ Public access methods */

message_id lu_summary_lookup_mid(
	const char* mmessage_id)
{
	message_id out = lu_common_minvalid;
	DBT key;
	DBT val;
	
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	key.data = (void*)mmessage_id;
	key.size = strlen(mmessage_id);
	val.data = &out;
	val.ulen = sizeof(out);
	val.flags = DB_DBT_USERMEM;
	
	my_summary_mid_db->get(
		my_summary_mid_db, 0, &key, &val, 0);
	
	return out;
}

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

int lu_summary_write_variable(
	int (*write)(void* arg, const char* str),
	int (*quote)(void* arg, const char* str, size_t len),
	int (*qurl )(void* arg, const char* str, size_t len),
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
				
				if (nulls == 0)
					qurl(arg, s, w-s);
				else
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

int lu_summary_write_lists(
	int (*write)(void* arg, lu_word list, message_id offset),
	void* arg,
	message_id id)
{
	DBT			key;
	DBT			val;
	int			error;
	
	My_Summary_Offset	offsets[20];
	int			i;
	
	/* Check to see if we were already loaded */
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	key.data = &id;
	key.size = sizeof(message_id);
	val.data = &offsets[0];
	val.size = 0;
	val.ulen = sizeof(offsets);
	val.flags = DB_DBT_USERMEM;
	
	error = my_summary_offset_db->get(
		my_summary_offset_db, 0, &key, &val, 0);
	if (error && error != DB_NOTFOUND)
	{
		syslog(LOG_ERR, _("Could not read occurance db: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	for (i = 0; i < val.size/sizeof(My_Summary_Offset); i++)
		(*write)(arg, offsets[i].list, offsets[i].offset);
	
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

/*------------------------------------------------- Import message methods */

int lu_summary_import_message(
	lu_word		list, 
	lu_word		mbox, 
	lu_addr		mbox_offset,
	time_t		timestamp, 
	const char*	mmessage_id,
	const char*	subject,
	const char*	author_name,
	const char*	author_email,
	message_id*	out)
{
	/* This function has to create the message stub info:
	 * 
	 * Detect previously imported message.
	 * Find what thread to use.
	 * Then write out location keywords.
	 *
	 * If new message:
	 *  We must push the variable information for the message.
	 *  Then we must push summary information for the message.
	 */
	 
	off_t		sum_off;
	off_t		var_off;
	lu_addr		high_bits;
	size_t		mid_len, sub_len, aun_len, aue_len;
	
	message_id		id;
	Lu_Summary_Message	sum;
	message_id		thread_id;
	message_id		loff;
	
	DBT			key;
	DBT			val;
	int			error;
	
	My_Summary_Offset	offsets[20];
	int			i;
	
	if (!subject) subject = "";
	
	id = my_summary_msg_free;
	
	/* Check to see if we were already loaded */
	memset(&key, 0, sizeof(DBT));
	memset(&val, 0, sizeof(DBT));
	
	key.data = (void*)mmessage_id;
	key.size = strlen(mmessage_id);
	val.data = &id;
	val.size = sizeof(id);
	
	error = my_summary_mid_db->put(
		my_summary_mid_db, 0, &key, &val, DB_NOOVERWRITE);
	if (error == DB_KEYEXIST)
	{	/* Already imported this message */
		val.ulen = sizeof(id);
		val.flags = DB_DBT_USERMEM;
		
		error = my_summary_mid_db->get(
			my_summary_mid_db, 0, &key, &val, 0);
	}
	if (error)
	{
		syslog(LOG_ERR, _("Checking message-id db for duplicate: %s\n"),
			db_strerror(errno));
		goto lu_summary_import_message_error0;
	}
	
	/* Write out the list occurance info */
	key.data = &id;
	key.size = sizeof(message_id);
	val.data = &offsets[0];
	val.size = 0;
	val.ulen = sizeof(offsets);
	val.flags = DB_DBT_USERMEM;
	
	error = my_summary_offset_db->get(
		my_summary_offset_db, 0, &key, &val, 0);
	if (error && error != DB_NOTFOUND)
	{
		syslog(LOG_ERR, _("Could not read occurance db: %s\n"),
			db_strerror(error));
		goto lu_summary_import_message_error0;
	}
	
	for (i = 0; i < val.size/sizeof(My_Summary_Offset); i++)
		if (offsets[i].list == list)
			break;
	
	if (i == sizeof(offsets)/sizeof(My_Summary_Offset))
	{
		syslog(LOG_WARNING, _("Ridiculous number of list occurances in message: %s (%d)"),
			mmessage_id, id);
	}
	else if (i == val.size/sizeof(My_Summary_Offset))
	{
		/* Write out location keywords -- but only do this if
		 * the message has not already been pushed.
		 */
		lu_indexer_location(list, mbox, &loff);
		
		val.size += sizeof(My_Summary_Offset);
		offsets[i].list   = list;
		offsets[i].offset = loff;
		
		error = my_summary_offset_db->put(
			my_summary_offset_db, 0, &key, &val, 0);
		
		if (error)
		{
			syslog(LOG_ERR, _("Could not write occurance db: %s\n"),
				db_strerror(error));
			goto lu_summary_import_message_error0;
		}
	}
	
	if (id < my_summary_msg_free)
	{	/* This is an old message; we're done. */
		*out = id;
		return 1;
	}
	
	/* What thread to put it in? */
	if (my_summary_find_thread(id, subject, timestamp, &thread_id) != 0)
	{
		goto lu_summary_import_message_error0;
	}
	
	lu_indexer_threading(
		list,
		thread_id,
		thread_id == id);
	
	/* Start writing variable length data */
	var_off = lseek(my_summary_variable_fd, 0, SEEK_END);
	if (var_off == -1)
	{
		syslog(LOG_ERR, _("Could not seek to end of variable.flat: %s\n"),
			strerror(errno));
		goto lu_summary_import_message_error0;
	}
	
	mid_len = strlen(mmessage_id) + 1;
	if (write(my_summary_variable_fd, mmessage_id, mid_len) != mid_len)
	{
		syslog(LOG_ERR, _("Could not write message id to end of variable.flat: %s\n"),
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	sub_len = strlen(subject) + 1;
	if (write(my_summary_variable_fd, subject, sub_len) != sub_len)
	{
		syslog(LOG_ERR, _("Could not write subject to end of variable.flat: %s\n"),
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	aun_len = strlen(author_name) + 1;
	if (write(my_summary_variable_fd, author_name, aun_len) != aun_len)
	{
		syslog(LOG_ERR, _("Could not write author name to end of variable.flat: %s\n"),
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	aue_len = strlen(author_email) + 1;
	if (write(my_summary_variable_fd, author_email, aue_len) != aue_len)
	{
		syslog(LOG_ERR, _("Could not write author email to end of variable.flat: %s\n"),
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	sum.mbox_offset   = mbox_offset;
 	sum.flat_offset   = var_off;
	sum.timestamp     = timestamp;
	sum.replies       = 0;
	sum.in_reply_to   = lu_common_minvalid;	/* nothing yet */
	sum.thread        = thread_id;		/* what thread are we? */
	
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
		syslog(LOG_ERR, _("Seeking for write of message summary: %s\n"),
			strerror(errno));
		goto lu_summary_import_message_error1;
	}
	
	if (write(my_summary_message_fd, &sum, sizeof(Lu_Summary_Message)) != 
		sizeof(Lu_Summary_Message))
	{
		syslog(LOG_ERR, _("Writing message summary: %s\n"),
			strerror(errno));
		goto lu_summary_import_message_error2;
	}
	
	my_summary_msg_free++;
	my_summary_last_time = timestamp;
	
	*out = id;
	return 0;

lu_summary_import_message_error2:
	sum_off = id;
	sum_off *= sizeof(Lu_Summary_Message);
	if (ftruncate(my_summary_message_fd, sum_off) != 0)
	{
		syslog(LOG_ERR, _("Unable to reclaim space in summary.flat on failed lu_summary_import_message: %s\n"),
			strerror(errno));
	}
	
lu_summary_import_message_error1:
	if (ftruncate(my_summary_variable_fd, var_off) != 0)
	{
		syslog(LOG_ERR, _("Unable to reclaim space in variable.flat on failed lu_summary_import_message: %s\n"),
			strerror(errno));
	}
	
lu_summary_import_message_error0:
	*out = lu_common_minvalid;
	return -1;
}

int lu_summary_reply_to_resolution(
	message_id id,
	const char* msg_id,
	const char* reply_to_msg_id)
{
	/* Then comes reply-to resolution:
	 * 
	 * Has something replied to us?  We run a query to see if any
	 * messages are in reply-to our message-id. For each of them, we
	 * update their reply-to link in the summary.
	 * 
	 * Have we replied to something?  We do a query to see if it exists.
	 * If it does, we update our reply-to summary informationn.
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
	 			{
	 				if (my_summary_reply_link(buf[i], id) != 0)
	 				{
	 					lu_breader_free(h);
	 					return -1;
	 				}
	 			}
	 			
	 			ind   += get;
	 			count -= get;
	 		}
	 		
	 		lu_breader_free(h);
	 	}
	 }
	 
	 if (*reply_to_msg_id)
	 {	/* Do we reply to anything? */
	 	buf[0] = lu_summary_lookup_mid(reply_to_msg_id);
	 	if (buf[0] != lu_common_minvalid)
	 	{
 			if (my_summary_reply_link(id, buf[0]) != 0)
 			{
 				return -1;
 			}
 		}
	 }
	 
	 return 0;
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
	
	if ((error = db_create(&my_summary_merge_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, _("Creating a db3 database: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&my_summary_mid_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, _("Creating a db3 database: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	if ((error = db_create(&my_summary_offset_db, lu_config_env, 0)) != 0)
	{
		fprintf(stderr, _("Creating a db3 database: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_summary_merge_db->open(
		my_summary_merge_db, "merge.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, _("Opening db3 database: merge.btree: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_summary_mid_db->open(
		my_summary_mid_db, "mid.hash", 0,
		DB_HASH, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, _("Opening db3 database: mid.hash: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	if ((error = my_summary_offset_db->open(
		my_summary_offset_db, "offset.hash", 0,
		DB_HASH, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, _("Opening db3 database: offset.hash: %s\n"),
			db_strerror(error));
		return -1;
	}
	
	if ((offset = lseek(my_summary_message_fd, 0, SEEK_END)) == -1)
	{
		fprintf(stderr, _("Determining last message id: merge.btree: %s\n"),
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
			perror(_("Grabbing last summary block"));
			return -1;
		}
		
		my_summary_last_time = sum.timestamp;
	}
	
	return 0;
}

int lu_summary_sync()
{
	int error;
	
	if ((error = my_summary_merge_db->sync(
		my_summary_merge_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Syncing db3 database: merge.btree: %s\n"),
			db_strerror(error));
	}
	
	if ((error = my_summary_mid_db->sync(
		my_summary_mid_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Syncing db3 database: mid.hash: %s\n"),
			db_strerror(error));
	}
	
	if ((error = my_summary_offset_db->sync(
		my_summary_offset_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Syncing db3 database: offset.hash: %s\n"),
			db_strerror(error));
	}
	
	return 0;
}

int lu_summary_close()
{
	int error;
	int fail = 0;
	
	if ((error = my_summary_merge_db->close(
		my_summary_merge_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Closing db3 database: merge.btree: %s\n"),
			db_strerror(error));
		fail = -1;
	}
	
	if ((error = my_summary_mid_db->close(
		my_summary_mid_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Closing db3 database: mid.hash: %s\n"),
			db_strerror(error));
		fail = -1;
	}
	
	if ((error = my_summary_offset_db->close(
		my_summary_offset_db, 0)) != 0)
	{
		syslog(LOG_ERR, _("Closing db3 database: offset.hash: %s\n"),
			db_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_summary_quit()
{
	return 0;
}
