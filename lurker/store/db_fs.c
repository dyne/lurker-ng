/*  $Id: db_fs.c,v 1.2 2002-06-24 11:06:25 terpstra Exp $
 *  
 *  db_fs.c - A flatfile database wrapper that just uses the filesystem
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

#include "config.h"
#include "flatfile.h"
#include "wbuffer.h"

#include <string.h>
#include <stdlib.h>

/*------------------------------------------------ Private helper methods */

const char* my_fs_filename(const char* keyword)
{
	static char buf[300];
	char* w;
	
	assert(*keyword);
	
	strcpy(&buf[0], "kw/");
	
	for (w = &buf[3]; *keyword; keyword++)
	{
		if (*keyword >= 'a' && *keyword <= 'z')
		{
			*w++ = *keyword;
		}
		else
		{
			sprintf(w, "%02X", (int)*keyword);
			w += 2;
		}
	}
	*w = 0;
	
	return &buf[0];
}

/*------------------------------------------------ Public access methods */

message_id lu_flatfile_write_keyword_block(
	const char*	keyword, 
	message_id*	buf,
	message_id	count)
{
	off_t	where;
	ssize_t	out;
	int	fd;
	
	fd = open(my_fs_filename(keyword),
		O_WRONLY | O_APPEND | O_CREAT | O_BINARY, LU_S_READ | LU_S_WRITE);
	if (fd == -1)
	{
		syslog(LOG_ERR, _("Open append keyword %s failed: %s"), 
			my_fs_filename(keyword),
			strerror(errno));
		return lu_common_minvalid;
	}
	
	out = write(fd, buf, count * sizeof(message_id));
	if (out != count * sizeof(message_id))
	{
		syslog(LOG_ERR, _("Write failed to complete: %s"), strerror(errno));
		close(fd);
		return lu_common_minvalid;
	}
	
	where = lseek(fd, 0, SEEK_CUR);
	if (where == (off_t)-1)
	{
		syslog(LOG_ERR, _("lseek failed to answer correctly: %s"), strerror(errno));
	}
	close(fd);
	
	return where / sizeof(message_id);
}

Lu_Flatfile_Handle lu_flatfile_open_handle(
	const char* keyword)
{
	int	fd;
	
	if (lu_wbuffer_flush(keyword) != 0)
		return 0;
	
	fd = open(my_fs_filename(keyword), O_BINARY | O_RDONLY);
	if (fd == -1 && errno == ENOENT) return (Lu_Flatfile_Handle)fd;
	if (fd == -1)
	{
		syslog(LOG_ERR, _("Open handle keyword %s failed: %s"), 
			my_fs_filename(keyword),
			strerror(errno));
		return 0;
	}
	
	return (Lu_Flatfile_Handle)fd;
}

message_id lu_flatfile_records(
	const char* keyword)
{
	message_id out;
	Lu_Flatfile_Handle h = lu_flatfile_open_handle(keyword);
	
	if (!h) return 0;
	out = lu_flatfile_handle_records(h);
	lu_flatfile_close_handle(h);
	
	return out;
}

message_id lu_flatfile_handle_records(
	Lu_Flatfile_Handle h)
{
	int fd = (int)h;
	off_t	where;
	
	if (fd == -1) return 0;
	
	where = lseek(fd, 0, SEEK_END);
	if (where == (off_t)-1)
	{
		syslog(LOG_ERR, _("lseek failed to answer correctly: %s"), strerror(errno));
	}
	
	return where / sizeof(message_id);
}

int lu_flatfile_handle_read(
	Lu_Flatfile_Handle h,
	message_id index, 
	message_id* buf, 
	message_id count)
{
	int fd = (int)h;
	
	lseek(fd, index * sizeof(message_id), SEEK_SET);
	if (read(fd, buf, count * sizeof(message_id)) != count * sizeof(message_id))
	{
		syslog(LOG_ERR, _("read failed: %s"), strerror(errno));
	}
	
	return 0;
}

void lu_flatfile_close_handle(
	Lu_Flatfile_Handle h)
{
	int fd = (int)h;
	if (fd != -1) close(fd);
}

/*------------------------------------------------ Public component methods */

int lu_flatfile_init()
{
	return 0;
}

int lu_flatfile_open()
{
	return 0;
}

int lu_flatfile_sync()
{
	return 0;
}

int lu_flatfile_close()
{
	return 0;
}

int lu_flatfile_quit()
{
	return 0;
}
