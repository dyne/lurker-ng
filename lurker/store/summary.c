/*  $Id: summary.c,v 1.34 2002-07-21 19:26:08 terpstra Exp $
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
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "keyword.h"

#include "config.h"
#include "summary.h"
#include "indexer.h"
#include "decode.h"

#include <ctype.h>
#include <string.h>
#include <netinet/in.h> /* ntohl */

#ifdef DMALLOC
# include <dmalloc.h>
#endif

/*------------------------------------------------ Constant parameters */

#define LU_SQUISHY_MAX	80

/* By default, if two messages with similar subject arrive within 90 days,
 * we consider them the same thread. 
 */
#define LU_THREAD_OVERLAP_TIME	60*60*24*90

/*------------------------------------------------ Private global vars */

static int my_summary_message_fd  = -1;
static int my_summary_variable_fd = -1;

static Kap my_summary_mid_db    = 0;
static Kap my_summary_offset_db = 0;
static Kap my_summary_merge_db  = 0;

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
	char thread_key[LU_SQUISHY_MAX];
	int  error;
	
	/* Format of records is:
	 *   4 bytes last   timestamp
	 *   4 bytes thread head
	 */
	unsigned char	record[4+sizeof(message_id)];
	ssize_t		len;
	
	off_t		lasttime;
	off_t		head;
	
	/* Get the key for where this thread lies */
	my_summary_squishy_subject(subject, &thread_key[0]);
	
	if (!thread_key[0])
	{
		*out = id;
		return 0;
	}
	
	error = kap_btree_read(
		my_summary_merge_db, 
		&thread_key[0],
		&record[0],
		&len);
	
	if (error && error != KAP_NOT_FOUND)
	{
		syslog(LOG_ERR, _("Unable to read thread information: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	/* Decide what to write */
	if (error == KAP_NOT_FOUND)
	{
		head     = id;
	}
	else
	{
		kap_decode_offset(&record[0], &lasttime, 4);
		
		/* Reuse thread? */
		if (lasttime + LU_THREAD_OVERLAP_TIME >= timestamp)
		{
			kap_decode_offset(&record[4], &head, sizeof(message_id));
		}
		else
		{
			head = id;
		}
	}
	
	lasttime = timestamp;
	
	/* Write it out */
	kap_encode_offset(&record[0], lasttime, 4);
	kap_encode_offset(&record[4], head,     sizeof(message_id));
	
	error = kap_btree_rewrite(
		my_summary_merge_db, 
		&thread_key[0],
		&record[0],
		8);
	
	if (error)
	{
		syslog(LOG_ERR, _("Unable to write thread information: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	/*!!! Should probably clean out the database of records that get too
	 * old since they will never be needed again later...
	 */
	
	*out = head;
	return 0;
}

static int my_summary_reply_link(
	message_id src, 
	message_id dest)
{
	off_t			soffset = src;
	off_t			doffset = dest;
	Lu_Summary_Message	ssum;
	Lu_Summary_Message	dsum;
	int out;
	
	out = lu_summary_read_msummary(src, &ssum);
	if (out) return out;
	out = lu_summary_read_msummary(dest, &dsum);
	if (out) return out;
	 
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
	off_t		out;
	int		error;
	unsigned char	buf[sizeof(message_id)];
	ssize_t		len;
	
	error = kap_btree_read(
		my_summary_mid_db, 
		mmessage_id,
		&buf[0],
		&len);
	
	if (error || len != sizeof(message_id))
		return lu_common_minvalid;
	
	kap_decode_offset(&buf[0], &out, sizeof(message_id));
	return out;
}

int lu_summary_read_msummary(message_id mid, Lu_Summary_Message* m)
{
	off_t offset = mid;
	
	offset *= sizeof(Lu_Summary_Message);
	if (lseek(my_summary_message_fd, offset, SEEK_SET) != offset)
	{
		memset(m, 0, sizeof(Lu_Summary_Message));
		return -1;
	}
	
	if (read(my_summary_message_fd, m, sizeof(Lu_Summary_Message)) != 
		sizeof(Lu_Summary_Message))
	{
		memset(m, 0, sizeof(Lu_Summary_Message));
		return -1;
	}
	
	return 0;
}

message_id lu_summary_num_messages(void)
{
	return my_summary_msg_free;
}

message_id lu_summary_find_timestamp(time_t tm)
{
	message_id l, r, m;
	Lu_Summary_Message msg;
	
	/* Goal:
	 *  Find the largest id such that all messages prior to it are prior
	 *  to time tm. AKA: find the first message with timestamp >= tm.
	 * Invariant:
	 *  The first message with timestamp >= tm is in [l, r) or DNE
	 *  r >= l
	 */
	l = 0;
	r = lu_summary_num_messages();
	
	while (r - l > 1)
	{
		m = (r+l)/2;
		
		lu_summary_read_msummary(m-1, &msg);
		
		if (msg.timestamp < tm)
		{	/* We know that m-1 is < tm. Therefore, >= m would
			 * satisfy all messages prior to it are prior to tm.
			 * Thus the answer lies in  [m, inf)
			 * Intersect with invariant -> [m, r)
			 */
			l = m;
		}
		else
		{	/* We know that m-1 is >= tm
			 * Therefore m is not the first thing >= tm.
			 * So, (-inf, m) is only possible.
			 * Intersect to get [l, m)
			 */
			r = m;
		}
	}
	
	if (r == l)
	{	/* No match */
		return lu_common_minvalid;
	}
	
	/* Okay, [l, r) = [l, l+1) = [l, l] is only possible answer.
	 * However, l itself may be < tm.
	 */
	lu_summary_read_msummary(l, &msg);
	
	if (msg.timestamp < tm)
	{	/* Ok, so therefore, everything must be < tm. */
		return lu_common_minvalid;
	}
	else
	{
		return l;
	}
}

int lu_summary_write_variable(
	int (*writefn)(void* arg, const char* str),
	int (*quotefn)(void* arg, const char* str, size_t len),
	int (*qurlfn )(void* arg, const char* str, size_t len),
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
						writefn(arg, "   <mid>");
						break;
					case 1:
						writefn(arg, "   <subject>");
						break;
					case 2:
						writefn(arg, " name=\"");
						break;
					case 3:
						writefn(arg, " address=\"");
						break;
					}
				}
				
				if (nulls == 0)
					qurlfn(arg, s, w-s);
				else
					quotefn(arg, s, w - s);
				
				if (have)
				{
					switch (nulls)
					{
					case 0:
						writefn(arg, "</mid>\n");
						break;
					case 1:
						writefn(arg, "</subject>\n");
						break;
					case 2:
						writefn(arg, "\"");
						break;
					case 3:
						writefn(arg, "\"");
						break;
					}
				}
				
				nulls++;
				s = w+1;
				have = 0;
				
				if (nulls == 2)
					writefn(arg, "   <email");
			}
		}
		
		if (!have && w != s)
		{
			have = 1;
			switch (nulls)
			{
			case 0:
				writefn(arg, "   <mid>");
				break;
			case 1:
				writefn(arg, "   <subject>");
				break;
			case 2:
				writefn(arg, "name=\"");
				break;
			case 3:
				writefn(arg, "address=\"");
				break;
			}
		}
		
		quotefn(arg, s, w - s);
	}
	
	writefn(arg, "/>\n");
	
	close(my_var_fd);
	return 0;
}

int lu_summary_write_lists(
	int (*writefn)(void* arg, lu_word list, message_id offset),
	void* arg,
	message_id id)
{
	KRecord	kr;
	size_t	i;
	int	error;
	char	key[20];
	
	/* Record format: 
	 *  <4 bytes list>
	 *  <4 bytes offset>
	 */
	unsigned char record[4+sizeof(message_id)];
	off_t	list;
	off_t	offset;
	
	sprintf(&key[0], "%ld", (long)id);
	
	error = kap_kopen(my_summary_offset_db, &kr, &key[0]);
	if (error)
	{
		syslog(LOG_ERR, _("Could not open KRecord in occurance db: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	for (i = 0; i < kr.records; i++)
	{
		error = kap_kread(my_summary_offset_db, &kr, &key[0],
			i, &record[0], 1);
		if (error)
		{
			syslog(LOG_ERR, _("Could access occurance db: %s\n"),
				kap_strerror(error));
			return -1;
		}
		
		kap_decode_offset(&record[0], &list,   4);
		kap_decode_offset(&record[4], &offset, sizeof(message_id));
		
		(*writefn)(arg, list, offset);
	}
	
	error = kap_kclose(my_summary_offset_db, &kr, &key[0]);
	if (error)
	{
		syslog(LOG_ERR, _("Could not close occurance db: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
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
	lu_word		list_id, 
	lu_word		mbox_id, 
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
	
	off_t			id;
	Lu_Summary_Message	sum;
	message_id		thread_id;
	message_id		loff;
	
	ssize_t			len;
	int			error;
	
	char			key[20];
	unsigned char		occurance[4+sizeof(message_id)];
	unsigned char		idrec[sizeof(message_id)];
	
	Lu_Config_List*		list;
	Lu_Config_Mbox*		mbox;
	
	list = lu_config_find_listi(lu_config_file, list_id);
	mbox = lu_config_find_mboxi(list, mbox_id);
	
	assert (list && mbox);
	
	if (!subject) subject = "";
	
	id = my_summary_msg_free;
	
	/* Check to see if we were already loaded */
	kap_encode_offset(&idrec[0], id, sizeof(message_id));
	error = kap_btree_write(
		my_summary_mid_db,
		mmessage_id,
		&idrec[0],
		sizeof(message_id));
		
	if (error == KAP_KEY_EXIST)
	{	/* Already imported this message */
		error = kap_btree_read(
			my_summary_mid_db,
			mmessage_id,
			&idrec[0],
			&len);
		
		kap_decode_offset(&idrec[0], &id, 4);
	}
	if (error)
	{
		syslog(LOG_ERR, _("Checking message-id db for duplicate: %s\n"),
			kap_strerror(errno));
		goto lu_summary_import_message_error0;
	}
	
	/* Write out location keywords.
	 */
	lu_indexer_location(list->name, mbox->name, &loff);
	
	/* Write out the list occurance info */
	sprintf(&key[0], "%ld", (long)id);
	kap_encode_offset(&occurance[0], list->key, 4);
	kap_encode_offset(&occurance[4], loff, sizeof(message_id));
	
	error = kap_append(
		my_summary_offset_db,
		&key[0],
		&occurance[0],
		1);
	if (error)
	{
			syslog(LOG_ERR, _("Could not write occurance db: %s\n"),
				kap_strerror(error));
			goto lu_summary_import_message_error0;
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
		list->name,
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
	
	high_bits = mbox->key;
	high_bits <<= (sizeof(lu_addr)-2)*8;
	sum.mbox_offset |= high_bits;
	
	high_bits = list->key;
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
	int			i, out;
	KRecord 		kr;
	
	if (*msg_id)
	{ 	/* Does anything already imported reply to us? */
		snprintf(&key[0], sizeof(key), "%s%s",
			LU_KEYWORD_REPLY_TO,
			msg_id);
		
		out = kap_kopen(lu_config_keyword, &kr, &key[0]);
		if (out == 0 && kr.records > 0)
		{
			count = kr.records;
			ind = 0;
			
			while (count)
			{
				get = count;
				if (get > sizeof(buf)/sizeof(message_id))
					get = sizeof(buf)/sizeof(message_id);
				
				out = kap_kread(
					lu_config_keyword, &kr,	&key[0],
					ind, &buf[0], get);
				
				if (out != 0)
				{
					syslog(LOG_ERR, _("Failed to read from KRecord for reply-to resolution: %s\n"),
						kap_strerror(out));
					kap_kclose(lu_config_keyword, &kr, &key[0]);
					return -1;
				}
				
				for (i = 0; i < get; i++)
				{
					if (my_summary_reply_link(buf[i], id) != 0)
					{
						kap_kclose(lu_config_keyword, &kr, &key[0]);
						return -1;
					}
				}
				
				ind   += get;
				count -= get;
			}
			
			out = kap_kclose(lu_config_keyword, &kr, &key[0]);
			if (out)
			{
				syslog(LOG_ERR, _("Failed to close KRecord during reply-to resolution: %s\n"),
					kap_strerror(out));
			}
		}
		else
		{
			if (out == 0) kap_kclose(lu_config_keyword, &kr, &key[0]);
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
	
	if ((error = kap_create(&my_summary_merge_db, KAP_BTREE)) != 0 ||
	    (error = kap_btree_set_maxkeysize(my_summary_merge_db, LU_SQUISHY_MAX)) != 0 ||
	    (error = kap_btree_set_leafsize  (my_summary_merge_db, 4+sizeof(message_id))) != 0 ||
	    (error = kap_btree_set_sectorsize(my_summary_merge_db, 2048)) != 0)
	{
		fprintf(stderr, _("Creating a kap database: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_create(&my_summary_mid_db, KAP_BTREE)) != 0 ||
	    (error = kap_btree_set_maxkeysize(my_summary_mid_db, MAX_MESSAGE_ID)) != 0 ||
	    (error = kap_btree_set_leafsize  (my_summary_mid_db, sizeof(message_id))) != 0 ||
	    (error = kap_btree_set_sectorsize(my_summary_mid_db, 2048)) != 0)
	{
		fprintf(stderr, _("Creating a kap database: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_create(&my_summary_offset_db, KAP_SMALL)) != 0 ||
	    (error = kap_btree_set_maxkeysize (my_summary_offset_db, 12)) != 0 ||
	    (error = kap_append_set_recordsize(my_summary_offset_db, 4+sizeof(message_id))) != 0)
	{
		fprintf(stderr, _("Creating a kap database: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_open(my_summary_merge_db, ".", "merge")) != 0)
	{
		fprintf(stderr, _("Opening kap database: merge.btree: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_open(my_summary_mid_db, ".", "mid")) != 0)
	{
		fprintf(stderr, _("Opening kap database: mid.btree: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((error = kap_open(my_summary_offset_db, ".", "offset")) != 0)
	{
		fprintf(stderr, _("Opening kap database: offset.*: %s\n"),
			kap_strerror(error));
		return -1;
	}
	
	if ((offset = lseek(my_summary_message_fd, 0, SEEK_END)) == -1)
	{
		fprintf(stderr, _("Determining last message id: summary.flat: %s\n"),
			strerror(errno));
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
		Lu_Summary_Message sum;
		if (lu_summary_read_msummary(my_summary_msg_free - 1, &sum) != 0)
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
	
	if ((error = kap_sync(my_summary_merge_db)) != 0)
	{
		syslog(LOG_ERR, _("Syncing kap database: merge.btree: %s\n"),
			kap_strerror(error));
	}
	
	if ((error = kap_sync(my_summary_mid_db)) != 0)
	{
		syslog(LOG_ERR, _("Syncing kap database: mid.btree: %s\n"),
			kap_strerror(error));
	}
	
	if ((error = kap_sync(my_summary_offset_db)) != 0)
	{
		syslog(LOG_ERR, _("Syncing kap database: offset.*: %s\n"),
			kap_strerror(error));
	}
	
	return 0;
}

int lu_summary_close()
{
	int error;
	int fail = 0;
	
	if ((error = kap_destroy(my_summary_merge_db)) != 0)
	{
		syslog(LOG_ERR, _("Closing kap database: merge.btree: %s\n"),
			kap_strerror(error));
		fail = -1;
	}
	
	if ((error = kap_destroy(my_summary_mid_db)) != 0)
	{
		syslog(LOG_ERR, _("Closing kap database: mid.btree: %s\n"),
			kap_strerror(error));
		fail = -1;
	}
	
	if ((error = kap_destroy(my_summary_offset_db)) != 0)
	{
		syslog(LOG_ERR, _("Closing kap database: offset.*: %s\n"),
			kap_strerror(error));
		fail = -1;
	}
	
	return fail;
}

int lu_summary_quit()
{
	return 0;
}
