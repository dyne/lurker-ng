/*  $Id: db.c,v 1.10 2002-01-28 08:32:13 terpstra Exp $
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

#include "records.h"
#include "io.h"
#include "globals.h"

#include <string.h>

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

message_id lu_import_message(
	lu_word list, lu_word mbox, lu_addr mbox_offset,
	time_t timestamp, 
	const char* subject,
	const char* author_name,
	const char* author_email)
{
	/* This function has to create the message stub info.
	 * 
	 * It must push the variable information for the message.
	 * Then it must push summary information for the message.
	 * 
	 * Then we have to see if this message is in a thread by checking
	 * if it falls within squishy_subject time window.
	 * Worse still, this message might be the 'missing link' and require
	 * merging two seperate threads.
	 * 
	 * Finally, we push keywords for the list, author, and subject.
	 */
	 
	 printf("HOLA! %s <%s>\n", author_name, author_email);
	 
	 return 0;
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
