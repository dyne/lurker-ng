/*  $Id: service.c,v 1.4 2002-02-04 08:05:31 terpstra Exp $
 *  
 *  service.c - Knows how to deal with request from the cgi
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
#include "protocol.h"

#include "config.h"
#include "summary.h"
#include "search.h"
#include "breader.h"
#include "service.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*------------------------------------------------- Private helper methods */

static int my_service_getmsg(st_netfd_t fd, const char* request)
{
	const char*	scan;
	const char*	last;
	char*		eptr;
	message_id	id;
	lu_addr		bits;
	off_t		off = 0;
	char		buf[4096];
	ssize_t		got;
	char*		eom;
	int		mbox_fd = -1;
	int		seen;
	int		had_seen;
	
	Lu_Proto_Message	out;
	Lu_Summary_Message	msg;
	
	Lu_Config_List*		list;
	Lu_Config_Mbox*		mbox = 0;
	
	id = strtoul(request, &eptr, 0);
	if (eptr == request || (*eptr && !isspace(*eptr)))
	{	/* There was nothing valid, or it did not end in whitespace
		 * or a null.
		 */
		return -1;
	}
	
	/* invalid message */
	memset(&out, 0, sizeof(out));
	
	msg = lu_summary_read_msummary(id);
	if (msg.timestamp != 0)
	{
		out.timestamp   = msg.timestamp;
		out.in_reply_to = msg.in_reply_to;
		out.thread      = msg.thread_parent;
		
		bits = msg.flat_offset;
		bits >>= (sizeof(lu_addr)*8) - 16;
		out.list = bits;
		
		bits = msg.mbox_offset;
		bits >>= (sizeof(lu_addr)*8) - 16;
		out.mbox = bits;
		
		bits = 0xFFFFUL;
		bits <<= (sizeof(lu_addr)*8) - 16;
		msg.mbox_offset &= ~bits;
		
		off = msg.mbox_offset;
		
		list = lu_config_find_list(out.list);
		if (list) mbox = lu_config_find_mbox(list, out.mbox);
		
		if (list)
		{
			strncpy(&out.list_name[0], list->name,
				sizeof(out.list_name));
			strncpy(&out.list_address[0], list->address,
				sizeof(out.list_address));
			strncpy(&out.list_desc[0], list->description,
				sizeof(out.list_desc));
			
			out.list_name   [sizeof(out.list_name   )-1] = 0;
			out.list_address[sizeof(out.list_address)-1] = 0;
			out.list_desc   [sizeof(out.list_desc   )-1] = 0;
		}
		
		if (mbox)
		{
			/* Find the last component of the path */
			last = scan = mbox->path;
			for (; *scan; scan++)
			{
				if (*scan == '/')
				{
					last = scan + 1;
				}
			}
			
			strncpy(&out.mbox_name[0], last,
				sizeof(out.mbox_name));
			
			out.mbox_name[sizeof(out.mbox_name)-1] = 0;
			
			/* We need a copy of this because while we are doing
			 * our st_writes, we could get context switched and
			 * the file closed. Also, we need an independent
			 * read position, so dup() is out.
			 */
			mbox_fd = open(mbox->path, O_RDONLY);
		}
	}
	
	if (st_write(fd, &out, sizeof(out), 5000000) != sizeof(out))
	{	/* Couldn't service their request */
		close(mbox_fd);
		return -1;
	}
	
	if (mbox_fd != -1)
	{
		if (lseek(mbox_fd, off, SEEK_SET) != off)
		{
			close(mbox_fd);
			return -1;
		}
		
		had_seen = seen = 0;
		while (seen != 6)
		{
			got = read(mbox_fd, &buf[0], sizeof(buf));
			if (got <= 0)
			{	/* Push whatever we defered. */
				st_write(fd, "\nFrom ", had_seen, 5000000);
				break;
			}
			
			had_seen = seen;
			for (eom = &buf[0]; seen != 6 && eom != &buf[got]; eom++)
			{
				switch (seen)
				{
				case 0: if (*eom == '\n') seen++; break;
				case 1: if (*eom == 'F') seen++; else seen = 0; break;
				case 2: if (*eom == 'r') seen++; else seen = 0; break;
				case 3: if (*eom == 'o') seen++; else seen = 0; break;
				case 4: if (*eom == 'm') seen++; else seen = 0; break;
				case 5: if (*eom == ' ') seen++; else seen = 0; break;
				}
			}
			
			/* If the stuff rolled over from last time was not
			 * part of the next message.
			 */
			if (eom - &buf[0] + had_seen != 6-1)
			{	/* Push whatever we defered. */
				st_write(fd, "\nFrom ", had_seen, 5000000);
			}
			
			/* Write all of what we got that wasn't potentially
			 * the start of the next message
			 */
			if (st_write(fd, &buf[0], eom - &buf[0] - seen + 1, 5000000) 
				!= eom - &buf[0] - seen + 1)
			{
				break;
			}
		}
		
		close(mbox_fd);
	}
	
	return -1;
}

static int my_service_digest_request(st_netfd_t fd, const char* request)
{
	/* Determine what to do with the request */
	if (memcmp(request, LU_PROTO_GETMSG, sizeof(LU_PROTO_GETMSG))) 
		return my_service_getmsg(fd, request+sizeof(LU_PROTO_GETMSG)-1);
	
	return -1;
}

/*------------------------------------------------- Public component methods */

int lu_service_init()
{
	return 0;
}

int lu_service_open()
{
	return 0;
}

int lu_service_sync()
{
	return 0;
}

int lu_service_close()
{
	return 0;
}

int lu_service_quit()
{
	return 0;
}

/*------------------------------------------------- Service methods */

extern int lu_service_connection(st_netfd_t fd)
{
	char	buf[200];
	char*	f;
	char*	s;
	char*	e;
	int	off;
	ssize_t	got;
	
	off = 0;
	
	while (1)
	{
		got = st_read(fd, 
			&buf[off], sizeof(buf) - off, 
			5000000); /* They have 5 seconds to talk */
		if (got <= 0)
		{	/* We're done - they closed link, timed out, or else
			 * something else went wrong.
			 */
			return -1;
		}
		
		/* Look for an end of request */
		f = &buf[0];
		e = &buf[off+got];
		
		for (s = &buf[off]; s != e; s++)
		{
			if (*s == LU_PROTO_ENDREQ)
			{
				*s = 0;
				if (my_service_digest_request(fd, f) != 0)
				{	/* Service the requst */
					return -1;
				}
				
				f = s+1;
			}
		}
		
		off = e - f;
		if (f != &buf[0])
		{	/* Move the remaining data to the start */
			memmove(&buf[0], f, off);
		}
		
		if (off == sizeof(buf))
		{	/* They filled our buffer without making a complete
			 * request.
			 */
			return -1;
		}
	}
	
	return 0;
}
