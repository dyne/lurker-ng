/*  $Id: service.c,v 1.14 2002-02-11 02:09:50 terpstra Exp $
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
#include "prefix.h"

#include "config.h"
#include "summary.h"
#include "search.h"
#include "breader.h"
#include "service.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>

/*------------------------------------------------- Private global vars */

/* A write buffer for output */
static char my_service_buffer[4096];
static int  my_service_used;

/*------------------------------------------------- Private helper methods */

static int my_service_buffer_writel(
	st_netfd_t fd, 
	const char* str,
	size_t len)
{
	char buf[20];
	
	while (len)
	{
		size_t amt = sizeof(my_service_buffer) - my_service_used;
		if (amt > len)
			amt = len;
		
		memcpy(&my_service_buffer[my_service_used], str, amt);
		
		str += amt;
		len -= amt;
		my_service_used += amt;
		
		if (my_service_used == sizeof(my_service_buffer))
		{
#ifdef DEBUG
			write(1, &my_service_buffer[0], sizeof(my_service_buffer));
#endif
			
			/* Tell the caller how big this chunk is */
			sprintf(&buf[0], "%d\n", sizeof(my_service_buffer));
			if (st_write(fd, &buf[0], 
				strlen(&buf[0]), 5000000) !=
				strlen(&buf[0]))
			{
				return -1;
			}
			
			if (st_write(fd, &my_service_buffer[0],
				sizeof(my_service_buffer), 5000000) !=
				sizeof(my_service_buffer))
			{
				return -1;
			}
			
			my_service_used = 0;
		}
	}
	
	return 0;
}

static int my_service_buffer_write(
	st_netfd_t fd, 
	const char* str)
{
	return my_service_buffer_writel(fd, str, strlen(str));
}

static int my_service_buffer_flush(
	st_netfd_t fd)
{
	char buf[20];
	
#ifdef DEBUG
	write(1, &my_service_buffer[0], my_service_used);
#endif

	/* Tell the caller how big this chunk is */
	sprintf(&buf[0], "%d\n", my_service_used);
	if (st_write(fd, &buf[0],
		strlen(&buf[0]), 5000000) !=
		strlen(&buf[0]))
	{
		return -1;
	}
	
	if (st_write(fd, &my_service_buffer[0], 
		my_service_used, 5000000) !=
		my_service_used)
	{
		return -1;
	}
	
	if (my_service_used != 0)
	{	/* Signal end of reply if we haven't already */
		sprintf(&buf[0], "%d\n", 0);
		if (st_write(fd, &buf[0], 
			strlen(&buf[0]), 5000000) !=
			strlen(&buf[0]))
		{
			return -1;
		}
		
		my_service_used = 0;
	}
	
	return 0;
}

static int my_service_write_strl(
	st_netfd_t fd,
	const char* buf,
	size_t length)
{
	const char* end;
	const char* start;

	/*
	 * Read the buffer, looking for XML symbols that need
	 * to be escaped with predefined entities.  Write in
	 * segments.
	 */
	end = buf + length;
	for (start = buf; buf != end; ++buf)
	{
		if (iscntrl(*buf) || (*buf & 0x80) != 0)
		{
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			
			/*!!! Do some sort of unicode craziness */
			start = buf+1;
		}
		else switch (*buf) 
		{
		case '\'':
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(fd, "&apos;") != 0)
				return -1;
			
			start = buf+1;
			break;
			
		case '<':
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(fd, "&lt;") != 0)
				return -1;
			
			start = buf+1;
			break;
			
		case '>':
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(fd, "&gt;") != 0)
				return -1;
			
			start = buf+1;
			break;
			
		case '"':
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(fd, "&quot;") != 0)
				return -1;
			
			start = buf+1;
			break;
		
		case '&':
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(fd, "&amp;") != 0)
				return -1;
				
			start = buf+1;
			break;
			
		case '\n':
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(fd, "<br/>") != 0)
				return -1;
			
			start = buf+1;
			break;
		}
	}
	
	if (my_service_buffer_writel(fd, start, buf - start) != 0)
		return -1;
	
	return 0;
}

static int my_service_write_str(
	st_netfd_t fd,
	const char* buf)
{
	return my_service_write_strl(fd, buf, strlen(buf));
}

static int my_service_write_int(
	st_netfd_t fd, 
	message_id id)
{
	char buf[20];
	sprintf(&buf[0], "%d", id);
	return my_service_buffer_write(fd, &buf[0]);
}

static int my_service_xml_head(
	st_netfd_t fd)
{
	return my_service_buffer_write(fd, 
		"<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n"
		"<?xml-stylesheet type=\"text/xml\" href=\"render.xslt\"?>\n"
		);
}

static int my_service_error(
	st_netfd_t fd,
	const char* title,
	const char* error,
	const char* detail)
{
	if (my_service_xml_head(fd)                              != 0) return -1;
	if (my_service_buffer_write(fd, "<error>\n <title>")     != 0) return -1;
	if (my_service_write_str(fd, title)                      != 0) return -1;
	if (my_service_buffer_write(fd, "</title>\n <message>")  != 0) return -1;
	if (my_service_write_str(fd, error)                      != 0) return -1;
	if (my_service_buffer_write(fd, "</message>\n <detail>") != 0) return -1;
	if (my_service_write_str(fd, detail)                     != 0) return -1;
	if (my_service_buffer_write(fd, "</detail>\n</error>\n") != 0) return -1;
	
	return 0;
}

static int my_service_list(
	st_netfd_t fd,
	Lu_Config_List* l,
	message_id msgs)
{
	if (my_service_buffer_write(fd, " <list>\n  <id>")     != 0) return -1;
	if (my_service_write_int(fd, l->id)                    != 0) return -1;
	if (my_service_buffer_write(fd, "</id>\n  <messages>") != 0) return -1;
	if (my_service_write_int(fd, msgs)                     != 0) return -1;
	if (my_service_buffer_write(fd, "</messages>\n")       != 0) return -1;
	
	if (l->name)
	{
		if (my_service_buffer_write(fd, "  <name>")  != 0) return -1;
		if (my_service_write_str(fd, l->name)        != 0) return -1;
		if (my_service_buffer_write(fd, "</name>\n") != 0) return -1;
	}
	
	if (l->address)
	{
		if (my_service_buffer_write(fd, "  <address>")  != 0) return -1;
		if (my_service_write_str(fd, l->address)        != 0) return -1;
		if (my_service_buffer_write(fd, "</address>\n") != 0) return -1;
	}
	
	if (l->description)
	{
		if (my_service_buffer_write(fd, "  <desc>")  != 0) return -1;
		if (my_service_write_str(fd, l->description) != 0) return -1;
		if (my_service_buffer_write(fd, "</desc>\n") != 0) return -1;
	}
	
	if (my_service_buffer_write(fd, " </list>\n") != 0) return -1;
	
	return 0;
}

static int my_service_summary(
	st_netfd_t fd,
	message_id id)
{
	Lu_Summary_Message	msg;
	time_t			tm;
	
	msg = lu_summary_read_msummary(id);
	
	tm = msg.timestamp;
	
	if (my_service_buffer_write(fd, " <summary>\n  <id>")     != 0) return -1;
	if (my_service_write_int(fd, id)                          != 0) return -1;
	if (my_service_buffer_write(fd, "</id>\n  <timestamp>")   != 0) return -1;
	if (my_service_write_int(fd, msg.timestamp)               != 0) return -1;
	if (my_service_buffer_write(fd, "</timestamp>\n  <time>") != 0) return -1;
	if (my_service_write_str(fd, ctime(&tm))                  != 0) return -1;
	if (my_service_buffer_write(fd, "</time>\n  <thread>")    != 0) return -1;
	if (my_service_write_int(fd, msg.thread_parent)           != 0) return -1;
	if (my_service_buffer_write(fd, "</thread>\n")            != 0) return -1;
	
	if (lu_summary_write_variable(
		&my_service_buffer_write,
		&my_service_write_strl,
		fd,
		msg.flat_offset) != 0) return -1;
	
	if (my_service_buffer_write(fd, " </summary>\n") != 0) return -1;
	
	return 0;
}

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
			if (list->name)
			{
				strncpy(&out.list_name[0], list->name,
					sizeof(out.list_name));
			}
			
			if (list->address)
			{
				strncpy(&out.list_address[0], list->address,
					sizeof(out.list_address));
			}
			
			if (list->description)
			{
				strncpy(&out.list_desc[0], list->description,
					sizeof(out.list_desc));
			}
			
			out.list_name   [sizeof(out.list_name   )-1] = 0;
			out.list_address[sizeof(out.list_address)-1] = 0;
			out.list_desc   [sizeof(out.list_desc   )-1] = 0;
		}
		
		if (mbox)
		{
			/* Find the last component of the path */
			if (mbox->path)
			{
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
			}
			
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
			
			/* Recall: for loops test at the top. Therefore, if
			 * we hit seen == 6, eom has been moved past the char
			 * which did it already.
			 */
			had_seen = seen;
			for (eom = &buf[0]; seen != 6 && eom != &buf[got]; eom++)
			{
				switch (*eom)
				{
				case '\n':
					seen = 1;
					break;
				case 'F':
					if (seen == 1) seen++; else seen = 0;
					break;
				case 'r':
					if (seen == 2) seen++; else seen = 0;
					break;
				case 'o':
					if (seen == 3) seen++; else seen = 0;
					break;
				case 'm':
					if (seen == 4) seen++; else seen = 0;
					break;
				case ' ':
					if (seen == 5) seen++; else seen = 0;
					break;
				default:
					seen = 0;
					break;
				}
			}
			
			/* There are three cases.
			 * Case 1: we found an end of a message marker that
			 *         crossed our buffer boundaries.
			 * Case 2: we found an end of message marker in this
			 *         message.
			 * Case 3: we did not find an end of message marker,
			 *         but the last 'seen' bytes might be the start
			 *         of one.
			 */
			
			/* Is this case 1? */
			if (eom - &buf[0] + had_seen == 6)
			{	/* Case 1. */
				/* do nothing. */
			}
			else
			{	/* Case 2 / 3. */
				
				/* Push whatever we defered as suspect. */
				st_write(fd, "\nFrom ", had_seen, 5000000);
			
				/* Write all of what we got that wasn't potentially
				 * the start of the next message. This works
				 * for both cases 2&3
				 */
				if (st_write(fd, &buf[0], eom - &buf[0] - seen,
					 5000000) != eom - &buf[0] - seen)
				{
					break;
				}
			}
		}
		
		close(mbox_fd);
	}
	
	return -1;
}

static int my_service_mindex(st_netfd_t fd, const char* request)
{
	Lu_Breader_Handle	h;
	Lu_Config_List*		l;
	message_id		offset;
	int			list;
	char			keyword[40];
	message_id		ids[LU_PROTO_INDEX];
	int			i;
	message_id		count;
	
	if (sscanf(request, "%d %d", &list, &offset) != 2)
	{	/* They did something funny. */
		my_service_error(fd,
			"Malformed request",
			"Lurkerd received an improperly formatted mindex request",
			request);
		
		goto my_service_mindex_error0;
	}
	
	if ((offset % LU_PROTO_INDEX) != 0)
	{	/* Must be a multiple of the index */
		my_service_error(fd,
			"Malformed request",
			"Lurkerd received an unaligned mindex request",
			request);
		
		goto my_service_mindex_error0;
	}
	
	l = lu_config_find_list(list);
	if (l == 0)
	{
		my_service_error(fd,
			"Invalid request",
			"Lurkerd received a request for a non-existant list",
			request);
		
		goto my_service_mindex_error0;
	}
	
	sprintf(&keyword[0], "%s%d", LU_KEYWORD_LIST, list);
	h = lu_breader_new(&keyword[0]);
	if (h == 0)
	{
		my_service_error(fd,
			"Internal error",
			"Lurkerd failed to access the keyword database",
			"server failure - see log files");
		
		goto my_service_mindex_error0;
	}
	
	if (offset >= lu_breader_records(h))
	{
		my_service_error(fd,
			"Invalid request",
			"Lurkerd received a mindex request past the end of the list",
			request);
		
		goto my_service_mindex_error1;
	}
	
	count = lu_breader_records(h) - offset;
	if (count > LU_PROTO_INDEX)
		count = LU_PROTO_INDEX;
	
	if (lu_breader_read(h, offset, count, &ids[0]) != 0)
	{
		my_service_error(fd,
			"Internal error",
			"Lurkerd failed to retrieve the records from the keyword file",
			"server failure - see log files");
		
		goto my_service_mindex_error1;
	}
	
	if (my_service_xml_head(fd)                            != 0) goto my_service_mindex_error1;
	if (my_service_buffer_write(fd, "<mindex>\n <offset>") != 0) goto my_service_mindex_error1;
	if (my_service_write_int(fd, offset)                   != 0) goto my_service_mindex_error1;
	if (my_service_buffer_write(fd, "</offset>\n")         != 0) goto my_service_mindex_error1;
	
	if (offset + count != lu_breader_records(h))
	{
		if (my_service_buffer_write(fd, " <next>")            != 0) goto my_service_mindex_error1;
		if (my_service_write_int(fd, offset + LU_PROTO_INDEX) != 0) goto my_service_mindex_error1;
		if (my_service_buffer_write(fd, "</next>\n")          != 0) goto my_service_mindex_error1;
	}
	
	if (offset != 0)
	{
		if (my_service_buffer_write(fd, " <prev>")            != 0) goto my_service_mindex_error1;
		if (my_service_write_int(fd, offset - LU_PROTO_INDEX) != 0) goto my_service_mindex_error1;
		if (my_service_buffer_write(fd, "</prev>\n")          != 0) goto my_service_mindex_error1;
	}
	
	if (my_service_list(fd, l, lu_breader_records(h)) != 0) goto my_service_mindex_error1;
	lu_breader_free(h);
	
	for (i = 0; i < count; i++)
	{
		if (my_service_summary(fd, ids[i]) != 0) goto my_service_mindex_error0;
	}
	
	if (my_service_buffer_write(fd, "</mindex>\n") != 0) goto my_service_mindex_error0;
	
	return 0;
	
my_service_mindex_error1:
	lu_breader_free(h);
	
my_service_mindex_error0:
	return -1;
}

static int my_service_search(st_netfd_t fd, const char* request)
{
	int		i;
	message_id	out;
	message_id	offset;
	const char*	delim;
	const char*	detail;
	
	delim = strchr(request, ' ');
	
	if (delim == 0)
	{
		my_service_error(fd,
			"Invalid request",
			"Lurkerd received a search request with no offset",
			request);
		goto my_service_search_error0;
	}
	
	offset = atol(request);
	if ((offset % LU_PROTO_INDEX) != 0)
	{	/* Must be a multiple of the index */
		my_service_error(fd,
			"Malformed request",
			"Lurkerd received an unaligned search request",
			request);
		
		goto my_service_search_error0;
	}
	
	if (offset > 1000)
	{
		my_service_error(fd,
			"Malformed request",
			"Lurkerd received a search request for too large an offset",
			request);
		goto my_service_search_error0;
	}
	
	if (lu_search_start(delim+1, &detail) != 0)
	{
		my_service_error(fd,
			"Search failed",
			"The lurkerd server rejected this search",
			detail);
		goto my_service_search_error0;
	}
	
	while (offset--)
	{
		if (lu_search_result(&out) != 0)
		{
			my_service_error(fd,
				"Internal error",
				"The lurkerd server failed to search",
				"lu_search_result failure - see logs");
			goto my_service_search_error1;
		}
		
		if (out == lu_common_minvalid)
		{
			my_service_error(fd,
				"Search failed",
				"The lurkerd server rejected this search",
				"There are not that many search results");
			goto my_service_search_error1;
		}
	}
	
	/* Ok! Now, lets start putting out the data */
	if (my_service_xml_head(fd)                            != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "<search>\n <offset>") != 0) goto my_service_search_error1;
	if (my_service_write_int(fd, offset)                   != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "</offset>\n <query>") != 0) goto my_service_search_error1;
	if (my_service_write_str(fd, delim+1)                  != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "</query>\n <hits>")   != 0) goto my_service_search_error1;
	if (my_service_write_int(fd, 0)                        != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "</hits>\n")           != 0) goto my_service_search_error1;
	
	if (offset != 0)
	{
		if (my_service_buffer_write(fd, " <prev>")            != 0) goto my_service_search_error1;
		if (my_service_write_int(fd, offset - LU_PROTO_INDEX) != 0) goto my_service_search_error1;
		if (my_service_buffer_write(fd, "</prev>\n")          != 0) goto my_service_search_error1;
	}
	
	for (i = 0; i < LU_PROTO_INDEX; i++)
	{
		if (lu_search_result(&out) != 0)
		{
			my_service_error(fd,
				"Internal error",
				"The lurkerd server failed to search",
				"lu_search_result failure - see logs");
			goto my_service_search_error1;
		}
		
		if (out == lu_common_minvalid)
		{
			break;
		}
		
		if (my_service_summary(fd, out) != 0) 
		{
			goto my_service_search_error1;
		}
	}
	
	if (out != lu_common_minvalid && lu_search_result(&out) == 0 &&
	    out != lu_common_minvalid)
	{
		if (my_service_buffer_write(fd, " <next>")            != 0) goto my_service_search_error1;
		if (my_service_write_int(fd, offset + LU_PROTO_INDEX) != 0) goto my_service_search_error1;
		if (my_service_buffer_write(fd, "</next>\n")          != 0) goto my_service_search_error1;
	}
	
	lu_search_end();
	if (my_service_buffer_write(fd, "</search>\n") != 0) goto my_service_search_error0;
	
	return 0;

my_service_search_error1:
	lu_search_end();
my_service_search_error0:
	return -1;
}

static int my_service_digest_request(st_netfd_t fd, const char* request)
{
	int out = -1;
	
	my_service_used = 0;

#ifdef DEBUG
	printf("Request: '%s'\n", request);
#endif
	
	/* Determine what to do with the request */
	if (!memcmp(request, LU_PROTO_GETMSG, sizeof(LU_PROTO_GETMSG)-1)) 
		out = my_service_getmsg(fd, request+sizeof(LU_PROTO_GETMSG)-1);
	if (!memcmp(request, LU_PROTO_MINDEX, sizeof(LU_PROTO_MINDEX)-1)) 
		out = my_service_mindex(fd, request+sizeof(LU_PROTO_MINDEX)-1);
	if (!memcmp(request, LU_PROTO_SEARCH, sizeof(LU_PROTO_SEARCH)-1)) 
		out = my_service_search(fd, request+sizeof(LU_PROTO_SEARCH)-1);
	
	/* Get rid of any buffering */
	my_service_buffer_flush(fd);
	
	return out;
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
	
#ifdef DEBUG
	printf("Connect!\n");
#endif
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
