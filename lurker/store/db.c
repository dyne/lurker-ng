/*  $Id: db.c,v 1.2 2002-01-21 01:18:00 terpstra Exp $
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
	val.size = sizeof(out);
	
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

int lu_open_db()
{
	int	error;
	char	buf[400];
	off_t	offset;
	
	snprintf(&buf[0], sizeof(buf), "%s/summary.flat", lu_dbdir);
	lu_summary_fd = open(&buf[0], 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (lu_summary_fd == -1)
	{
		perror(&buf[0]);
		return -1;
	}
	
	snprintf(&buf[0], sizeof(buf), "%s/variable.flat", lu_dbdir);
	lu_variable_fd = open(&buf[0], 
		O_RDWR | O_BINARY | O_CREAT,
		LU_S_READ | LU_S_WRITE);
	if (lu_variable_fd == -1)
	{
		perror(&buf[0]);
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
	
	if ((error = lu_thread_db->open(lu_thread_db, "thread.hash", 0,
		DB_HASH, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: thread.hash: %s\n",
			db_strerror(error));
		return -1;
	}
	
	if ((error = lu_thread_db->open(lu_merge_db, "merge.btree", 0,
		DB_BTREE, DB_CREATE, LU_S_READ | LU_S_WRITE)) != 0)
	{
		fprintf(stderr, "Opening db3 database: merge.btree: %s\n",
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
