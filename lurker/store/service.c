/*  $Id: service.c,v 1.21 2002-02-22 00:51:43 terpstra Exp $
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
#include "mbox.h"
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
switch (*buf) 
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
			if (my_service_buffer_write(fd, "<br/>\n") != 0)
				return -1;
			
			start = buf+1;
			break;
			
		default:
			if (iscntrl(*buf) || (*buf & 0x80) != 0)
			{
				if (my_service_buffer_writel(fd, start, buf - start) != 0)
					return -1;
				
				/*!!! Do some sort of unicode craziness */
				start = buf+1;
			}
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

static int my_service_write_url(
	st_netfd_t fd,
	const char* buf)
{
	char hex[8];
	const char* start;

	for (start = buf; *buf; buf++)
	{
		if (!isalnum(*buf))
		{
			snprintf(&hex[0], sizeof(hex), "%%%2X", *buf);
			
			if (my_service_buffer_writel(fd, start, buf - start) != 0)
				return -1;
			if (my_service_write_str(fd, &hex[0]) != 0)
				return -1;
			
			start = buf+1;
		}
	}
	
	if (my_service_buffer_writel(fd, start, buf - start) != 0)
		return -1;
	
	return 0;
	
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

static int my_service_server(
	st_netfd_t fd)
{
	if (my_service_buffer_write(fd, " <server>\n  <hostname>")       != 0) return -1;
	if (my_service_write_str(fd, lu_config_list_host)                != 0) return -1;
	if (my_service_buffer_write(fd, "</hostname>\n  <email name=\"") != 0) return -1;
	if (my_service_write_str(fd, lu_config_admin_name)               != 0) return -1;
	if (my_service_buffer_write(fd, "\" address=\"")                 != 0) return -1;
	if (my_service_write_str(fd, lu_config_admin_address)            != 0) return -1;
	if (my_service_buffer_write(fd, "\"/>\n </server>\n")            != 0) return -1;
	
	return 0;
}

static int my_service_addresses(
	st_netfd_t fd,
	ADDRESS* addr, 
	const char* name)
{
	if (!addr) return 0;
	
	if (my_service_buffer_write(fd, " <") != 0) return -1;
	if (my_service_write_str   (fd, name) != 0) return -1;
	if (my_service_buffer_write(fd, ">" ) != 0) return -1;
	
	for (; addr; addr = addr->next)
	{
		if (!addr->personal && (!addr->mailbox || !addr->host))
		{
			continue;
		}
		
		if (my_service_buffer_write(fd, "<email") != 0) return -1;
		
		if (addr->personal)
		{
			if (my_service_buffer_write(fd, " name=\""    ) != 0) return -1;
			if (my_service_write_str   (fd, addr->personal) != 0) return -1;
			if (my_service_buffer_write(fd, "\""          ) != 0) return -1;
		}
		
		if (addr->mailbox && addr->host)
		{
			if (my_service_buffer_write(fd, " address=\"") != 0) return -1;
			if (my_service_write_str   (fd, addr->mailbox) != 0) return -1;
			if (my_service_buffer_write(fd, "@"          ) != 0) return -1;
			if (my_service_write_str   (fd, addr->host   ) != 0) return -1;
			if (my_service_buffer_write(fd, "\""         ) != 0) return -1;
		}
		
		if (my_service_buffer_write(fd, "/>") != 0) return -1;
	}
	
	if (my_service_buffer_write(fd, "</")  != 0) return -1;
	if (my_service_write_str   (fd, name)  != 0) return -1;
	if (my_service_buffer_write(fd, ">\n") != 0) return -1;
	
	return 0;
}

static char* my_service_find_name(
	struct mail_bodystruct* body)
{
	PARAMETER* scan;
	
	for (scan = body->disposition.parameter; scan; scan = scan->next)
	{
		if (!strcmp(scan->attribute, "filename"))
			return scan->value;
	}
	
	for (scan = body->parameter; scan; scan = scan->next)
	{
		if (!strcmp(scan->attribute, "name"))
			return scan->value;
	}
	
	return 0;
}

static int my_service_dump(
	st_netfd_t	fd,
	const char*	buf,
	size_t		len)
{
	/*!!! Search the message for URLs, email address, quotations, etc */
	return my_service_write_strl(fd, buf, len);
}

static int my_service_traverse(
	st_netfd_t		fd,
	struct Lu_Mbox_Message* in, 
	struct mail_bodystruct* body,
	int			count)
{
	static const char* names[] = 
	{	"text", "multipart", "message", "application", "audio", 
		"image", "video", "model", "x-unknown"
	};
	    
	struct mail_body_part*	p;
	size_t			length;
	char*			buffer;
	int			nfree;
	char*			name;
	
	if (!body)
	{	/* You never know... */
		return count;
	}
	
	if (body->type > 8) body->type = 8;
	
	name = my_service_find_name(body);
	
	if (my_service_buffer_write(fd, "<mime id=\""    ) != 0) return -1;
	if (my_service_write_int   (fd, count            ) != 0) return -1;
	if (my_service_buffer_write(fd, "\" type=\""     ) != 0) return -1;
	if (my_service_buffer_write(fd, names[body->type]) != 0) return -1;
	if (my_service_buffer_write(fd, "/"              ) != 0) return -1;
	if (my_service_buffer_write(fd, body->subtype    ) != 0) return -1;
	if (my_service_buffer_write(fd, "\""             ) != 0) return -1;
	
	if (name)
	{
		if (my_service_buffer_write(fd, " name=\"") != 0) return -1;
		if (my_service_write_str   (fd, name      ) != 0) return -1;
		if (my_service_buffer_write(fd, "\""      ) != 0) return -1;
	}
	
	if (my_service_buffer_write(fd, ">") != 0) return -1;

	count++;
	
	switch ((int)body->type)
	{
		case TYPEMESSAGE:
			/* This part contains an encapsulated message.
			 */
			count = my_service_traverse(fd, in, 
				body->nested.msg->body, count);
				
			if (count == -1) return -1;
			break;

		case TYPETEXT:
			/*
			 * This is what we want to display -- stop.
			 */
			buffer = lu_mbox_select_body(in, body, &length, &nfree);
			
			if (my_service_dump(fd, buffer, length) != 0)
				return -1;
			
			if (nfree)
				fs_give((void **)&buffer);

			break;

		case TYPEMULTIPART:
			/*
			 * Multipart message.  Look at the nested parts
			 * and hopefully find some plaintext.
			 */
			for (p = body->nested.part; p != NULL; p = p->next)
			{
				count = my_service_traverse(fd, in,
					&p->body, count);
				if (count == -1) break;
			}
			
			break;
			
		default:
			break;
	}
	
	if (my_service_buffer_write(fd, "</mime>\n") != 0) return -1;
	
	return count;
}

static int my_service_list(
	st_netfd_t fd,
	Lu_Config_List* l,
	message_id msgs)
{
	if (my_service_buffer_write(fd, " <list>\n  <id>")       != 0) return -1;
	if (my_service_write_int(fd, l->id)                      != 0) return -1;
	if (my_service_buffer_write(fd, "</id>\n")               != 0) return -1;
	
	if (msgs != lu_common_minvalid)
	{
		if (my_service_buffer_write(fd, "  <messages>")  != 0) return -1;
		if (my_service_write_int(fd, msgs)               != 0) return -1;
		if (my_service_buffer_write(fd, "</messages>\n") != 0) return -1;
	}
	
	if (my_service_buffer_write(fd, "  <email") != 0) return -1;
	
	if (l->name)
	{
		if (my_service_buffer_write(fd, " name=\"")  != 0) return -1;
		if (my_service_write_str(fd, l->name)        != 0) return -1;
		if (my_service_buffer_write(fd, "\"")        != 0) return -1;
	}
	
	if (l->address)
	{
		if (my_service_buffer_write(fd, " address=\"") != 0) return -1;
		if (my_service_write_str(fd, l->address)       != 0) return -1;
		if (my_service_buffer_write(fd, "\"")          != 0) return -1;
	}
	
	if (my_service_buffer_write(fd, "/>\n") != 0) return -1;
	
	if (l->description)
	{
		if (my_service_buffer_write(fd, "  <description>")  != 0) return -1;
		if (my_service_write_str(fd, l->description)        != 0) return -1;
		if (my_service_buffer_write(fd, "</description>\n") != 0) return -1;
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

static int my_service_getmsg(
	st_netfd_t fd, 
	const char* request)
{
	const char*	scan;
	const char*	last;
	char*		eptr;
	message_id	id;
	lu_addr		bits;
	time_t		tm;
	
	lu_word		list_id;
	lu_word		mbox_id;
	off_t		offset;
	
	Lu_Summary_Message		msg;
	struct Lu_Config_Message	cmsg;
	struct Lu_Mbox_Message		mmsg;
	
	Lu_Config_List*		list;
	Lu_Config_Mbox*		mbox;
	
	id = strtoul(request, &eptr, 0);
	if (eptr == request || (*eptr && !isspace(*eptr)))
	{	/* There was nothing valid, or it did not end in whitespace
		 * or a null.
		 */
		my_service_error(fd,
			"Malformed request",
			"Lurkerd received a request for a non-numeric message",
			request);
		goto my_service_getmsg_error0;
	}
	
	msg = lu_summary_read_msummary(id);
	if (msg.timestamp == 0)
	{
		my_service_error(fd,
			"Internal Error",
			"Lurkerd failed to retrieve the summary for the named message",
			request);
		goto my_service_getmsg_error0;
	}
	
	bits = msg.flat_offset;
	bits >>= (sizeof(lu_addr)*8) - 16;
	list_id = bits;
	
	bits = msg.mbox_offset;
	bits >>= (sizeof(lu_addr)*8) - 16;
	mbox_id = bits;
	
	bits = 0xFFFFUL;
	bits <<= (sizeof(lu_addr)*8) - 16;
	offset = msg.mbox_offset & ~bits;
	
	list = lu_config_find_list(list_id);
	if (!list)
	{
		my_service_error(fd,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailing list",
			request);
		goto my_service_getmsg_error0;
	}
	
	mbox = lu_config_find_mbox(list, mbox_id);
	if (!mbox)
	{
		my_service_error(fd,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailbox",
			request);
		goto my_service_getmsg_error0;
	}
	
	if (lu_mbox_map_message(mbox, &cmsg, offset) != 0)
	{
		my_service_error(fd,
			"Internal Error",
			"Lurkerd was unable to mmap the message into memory",
			request);
		goto my_service_getmsg_error0;
	}
	
	if (lu_mbox_parse_message(&cmsg, &mmsg) != 0)
	{
		my_service_error(fd,
			"Internal Error",
			"Lurkerd was unable to parse the map'd message",
			request);
		goto my_service_getmsg_error1;
	}
	
	if (my_service_xml_head(fd)                       != 0) goto my_service_getmsg_error2;
	if (my_service_buffer_write(fd, "<message>\n")    != 0) goto my_service_getmsg_error2;
	if (my_service_server(fd)                         != 0) goto my_service_getmsg_error2;
	if (my_service_list(fd, list, lu_common_minvalid) != 0) goto my_service_getmsg_error2;
	
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
		
		if (my_service_buffer_write(fd, " <mbox>")   != 0) goto my_service_getmsg_error2;
		if (my_service_write_str(fd, last)           != 0) goto my_service_getmsg_error2;
		if (my_service_buffer_write(fd, "</mbox>\n") != 0) goto my_service_getmsg_error2;
	}
	
	tm = msg.timestamp;
	
	if (my_service_buffer_write(fd, " <id>")                 != 0) goto my_service_getmsg_error2;
	if (my_service_write_int   (fd, id)                      != 0) goto my_service_getmsg_error2;
	if (my_service_buffer_write(fd, "</id>\n <timestamp>")   != 0) goto my_service_getmsg_error2;
	if (my_service_write_int   (fd, msg.timestamp)           != 0) goto my_service_getmsg_error2;
	if (my_service_buffer_write(fd, "</timestamp>\n <time>") != 0) goto my_service_getmsg_error2;
	if (my_service_write_str   (fd, ctime(&tm))              != 0) goto my_service_getmsg_error2;
	if (my_service_buffer_write(fd, "</time>\n <thread>")    != 0) goto my_service_getmsg_error2;
	if (my_service_write_int   (fd, msg.thread_parent)       != 0) goto my_service_getmsg_error2;
	if (my_service_buffer_write(fd, "</thread>")             != 0) goto my_service_getmsg_error2;
	
	if (msg.in_reply_to != lu_common_minvalid)
	{
		if (my_service_buffer_write(fd, "<inreplyto>")     != 0) goto my_service_getmsg_error2;
		if (my_service_write_int   (fd, msg.in_reply_to)   != 0) goto my_service_getmsg_error2;
		if (my_service_buffer_write(fd, "</inreplyto>\n")  != 0) goto my_service_getmsg_error2;
	}
	
	/*!!! list those message that reply to this one */
	
	if (my_service_addresses(fd, mmsg.env->from,     "from"    ) != 0) goto my_service_getmsg_error2;
	if (my_service_addresses(fd, mmsg.env->sender,   "sender"  ) != 0) goto my_service_getmsg_error2;
	if (my_service_addresses(fd, mmsg.env->reply_to, "reply-to") != 0) goto my_service_getmsg_error2;
	if (my_service_addresses(fd, mmsg.env->to,       "to"      ) != 0) goto my_service_getmsg_error2;
	if (my_service_addresses(fd, mmsg.env->cc,       "cc"      ) != 0) goto my_service_getmsg_error2;
	if (my_service_addresses(fd, mmsg.env->bcc,      "bcc"     ) != 0) goto my_service_getmsg_error2;
	
	if (mmsg.env->message_id)
	{
		if (my_service_buffer_write(fd, " <message-id>"     ) != 0) goto my_service_getmsg_error2;
		if (my_service_write_str   (fd, mmsg.env->message_id) != 0) goto my_service_getmsg_error2;
		if (my_service_buffer_write(fd, "</message-id>\n"   ) != 0) goto my_service_getmsg_error2;
	}
	
	if (mmsg.env->subject)
	{
		if (my_service_buffer_write(fd, " <subject>"     ) != 0) goto my_service_getmsg_error2;
		if (my_service_write_str   (fd, mmsg.env->subject) != 0) goto my_service_getmsg_error2;
		if (my_service_buffer_write(fd, "</subject>\n"   ) != 0) goto my_service_getmsg_error2;
	}
	
	if (my_service_traverse(fd, &mmsg, mmsg.body, 0) == -1) goto my_service_getmsg_error2;
	if (my_service_buffer_write(fd, "</message>\n")  != 0) goto my_service_getmsg_error2;
	
	lu_mbox_destroy_message(&mmsg);
	lu_mbox_destroy_map(&cmsg);
	return 0;
	
my_service_getmsg_error2:
	lu_mbox_destroy_message(&mmsg);
my_service_getmsg_error1:
	lu_mbox_destroy_map(&cmsg);
my_service_getmsg_error0:
	return -1;
}

static int my_service_mindex(
	st_netfd_t fd, 
	const char* request)
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
	
	if (my_service_server(fd) != 0) goto my_service_mindex_error1;
	
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

static int my_service_search(
	st_netfd_t fd, 
	const char* request)
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
	
	for (i = 0; i < offset; i++)
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
	if (my_service_xml_head(fd)                             != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "<search>\n <offset>")  != 0) goto my_service_search_error1;
	if (my_service_write_int(fd, offset)                    != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "</offset>\n <query>")  != 0) goto my_service_search_error1;
	if (my_service_write_str(fd, delim+1)                   != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "</query>\n <queryurl>")!= 0) goto my_service_search_error1;
	if (my_service_write_url(fd, delim+1)                   != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "</queryurl>\n <hits>") != 0) goto my_service_search_error1;
	if (my_service_write_int(fd, 0)                         != 0) goto my_service_search_error1;
	if (my_service_buffer_write(fd, "</hits>\n")            != 0) goto my_service_search_error1;
	if (my_service_server(fd)                               != 0) goto my_service_search_error1;
	
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

static int my_service_lists(
	st_netfd_t fd, 
	const char* request)
{
	Lu_Config_List*		scan;
	Lu_Breader_Handle	h;
	char			key[40];
	
	if (my_service_xml_head(fd)                  != 0) return -1;
	if (my_service_buffer_write(fd, "<lists>\n") != 0) return -1;
	if (my_service_server(fd)                    != 0) return -1;

	for (	scan = lu_config_list; 
		scan != lu_config_list + lu_config_lists; 
		scan++)
	{
		snprintf(&key[0], sizeof(key), "%s%d", LU_KEYWORD_LIST, scan->id);
		h = lu_breader_new(&key[0]);
		if (h == 0) continue;
		if (my_service_list(fd, scan, lu_breader_records(h)) != 0)
		{
			lu_breader_free(h);
			return -1;
		}
		lu_breader_free(h);
		
	}
	if (my_service_buffer_write(fd, "</lists>\n") != 0) return -1;
	
	return 0;
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
	if (!memcmp(request, LU_PROTO_LISTS, sizeof(LU_PROTO_LISTS)-1)) 
		out = my_service_lists(fd, request+sizeof(LU_PROTO_LISTS)-1);
	
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
