/*  $Id: db.c,v 1.1.1.1 2002-01-21 00:03:07 terpstra Exp $
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

int	lu_summary_fd;
int	lu_variable_fd;
DB_ENV*	lu_db_env;
DB*	lu_thread_db;
DB*	lu_merge_db;

static message_id	lu_msg_free;
static time_t		lu_last_time;

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
	
	return 0;
}
