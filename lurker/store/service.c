/*  $Id: service.c,v 1.49 2002-05-21 18:17:25 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "prefix.h"

#include "config.h"
#include "summary.h"
#include "search.h"
#include "breader.h"
#include "expiry.h"
#include "mbox.h"
#include "service.h"

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <iconv.h>

#define LU_PROTO_INDEX	20

#define	EMPTY_CELL	"<a/>"
#define BAR_NS		"<b/>"
#define BAR_EW		"<c/>"
#define CORNER_SW	"<d/>"
#define TEE_WSE		"<e/>"
#define MESSAGE_END	"<f/>"
#define MESSAGE_DOWN	"<g/>"
#define MESSAGE_BOTH	"<h/>"
#define TOPMESSAGE_END	"<i/>"
#define TOPMESSAGE_DOWN	"<j/>"
#define TOPMESSAGE_BOTH	"<k/>"

/*------------------------------------------------- Private types */

/* We need one of these for each connection - a global will cause much
 * pain and suffering.
 */
typedef struct My_Service_Handle_T
{
	int		used;
	int		sent;
	
	st_netfd_t	fd;
	
	int		cache;
	time_t		ttl;
	lu_word		list;
	
	int		first_flush;
	int		init;
	
	int		bufsize;
	char* 		buffer;
}* My_Service_Handle;

typedef struct My_Service_Reply_Tree_T
{
	message_id		id;
	Lu_Summary_Message	summary;
	
	int			replies;
	int			replyee;
	int			replyor_first;
	int			replyor_next;
	
	int			depth;
	int			consumed;
	int			column;
	int			draw_next;
} My_Service_Reply_Tree;

/*------------------------------------------------- Private globals */

static const char* my_service_mime[] = 
{	"text", "multipart", "message", "application", "audio", 
	"image", "video", "model", "x-unknown"
};
	    
/*------------------------------------------------- Private helper methods */

static int my_service_buffer_init(
	My_Service_Handle h,
	const char*	type,
	int		cache,
	time_t		ttl,
	lu_word		list)
{
	assert(!h->init);
	
	if (st_write(h->fd, type, strlen(type), 5000000) != strlen(type))
	{
		return -1;
	}
	
	h->sent  = 0;
	h->used  = 0;
	
	h->init  = 1;
	
	h->cache = cache;
	h->ttl   = ttl;
	h->list  = list;
	
	if (cache)
	{
		h->first_flush = 1;
	}
	else
	{
		if (st_write(h->fd, "0\n", 2, 5000000) != 2)
			return -1;
		h->first_flush = 0;
	}
	
	return 0;
}

static int my_service_buffer_writel(
	My_Service_Handle h,
	const char* str,
	size_t len)
{
	char buf[20];
	
	assert(h->init);
	
	while (len)
	{
		size_t amt = h->bufsize - h->used;
		if (amt > len)
			amt = len;
		
		memcpy(&h->buffer[h->used], str, amt);
		
		str += amt;
		len -= amt;
		h->used += amt;
		
		if (h->used == h->bufsize)
		{
#ifdef DEBUG
			write(1, &h->buffer[0], h->bufsize);
#endif
			if (h->first_flush)
			{
				if (st_write(h->fd, "0\n", 2, 5000000) != 2)
				{
					return -1;
				}
				h->first_flush = 0;
			}
			
			/* Tell the caller how big this chunk is */
			sprintf(&buf[0], "%d\n", h->bufsize);
			if (st_write(h->fd, &buf[0], 
				strlen(&buf[0]), 5000000) !=
				strlen(&buf[0]))
			{
				return -1;
			}
			
			if (st_write(h->fd, &h->buffer[0],
				h->bufsize, 5000000) !=
				h->bufsize)
			{
				return -1;
			}
			
			h->sent += h->bufsize;
			h->used = 0;
		}
	}
	
	return 0;
}

static int my_service_buffer_write(
	My_Service_Handle	h,
	const char*		str)
{
	return my_service_buffer_writel(h, str, strlen(str));
}

static int my_service_buffer_flush(
	My_Service_Handle h)
{
	char buf[20];
	
#ifdef DEBUG
	write(1, &h->buffer[0], h->used);
#endif
	
	assert(h->init);
	
	if (h->first_flush)
	{
		if (st_write(h->fd, "1\n", 2, 5000000) != 2)
		{
			return -1;
		}
		h->first_flush = 0;
	}

	/* Tell the caller how big this chunk is */
	sprintf(&buf[0], "%d\n", h->used);
	if (st_write(h->fd, &buf[0],
		strlen(&buf[0]), 5000000) !=
		strlen(&buf[0]))
	{
		return -1;
	}
	
	if (st_write(h->fd, &h->buffer[0], 
		h->used, 5000000) !=
		h->used)
	{
		return -1;
	}
	
	h->sent += h->used;
	
	if (h->used != 0)
	{	/* Signal end of reply if we haven't already */
		sprintf(&buf[0], "%d\n", 0);
		if (st_write(h->fd, &buf[0], 
			strlen(&buf[0]), 5000000) !=
			strlen(&buf[0]))
		{
			return -1;
		}
		
		h->used = 0;
	}
	
	return 0;
}

static int my_service_write_strl(
	My_Service_Handle	h,
	const char*		buf,
	size_t			length)
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
			if (my_service_buffer_writel(h, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(h, "&apos;") != 0)
				return -1;
			
			start = buf+1;
			break;
			
		case '<':
			if (my_service_buffer_writel(h, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(h, "&lt;") != 0)
				return -1;
			
			start = buf+1;
			break;
			
		case '>':
			if (my_service_buffer_writel(h, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(h, "&gt;") != 0)
				return -1;
			
			start = buf+1;
			break;
			
		case '"':
			if (my_service_buffer_writel(h, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(h, "&quot;") != 0)
				return -1;
			
			start = buf+1;
			break;
		
		case '&':
			if (my_service_buffer_writel(h, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(h, "&amp;") != 0)
				return -1;
				
			start = buf+1;
			break;
			
		case '\n':
			if (my_service_buffer_writel(h, start, buf - start) != 0)
				return -1;
			if (my_service_buffer_write(h, "<br/>\n") != 0)
				return -1;
			
			start = buf+1;
			break;
		
		default:
			/* Simply drop control characters: we are outputing
			 * utf-8; they shouldn't be there.
			 */
			if (*buf < 32 && *buf >= 0)
			{
				if (my_service_buffer_writel(h, start, buf - start) != 0)
					return -1;
				start = buf+1;
			}
		}
	}
	
	if (my_service_buffer_writel(h, start, buf - start) != 0)
		return -1;
	
	return 0;
}

static int my_service_write_str(
	My_Service_Handle	h,
	const char*		buf)
{
	return my_service_write_strl(h, buf, strlen(buf));
}

static int my_service_write_int(
	My_Service_Handle	h, 
	message_id		id)
{
	char buf[20];
	sprintf(&buf[0], "%d", id);
	return my_service_buffer_write(h, &buf[0]);
}

static int my_service_write_ehead(
	My_Service_Handle	h,
	const char*		header,
	const char*		coding)
{
	char buf[1024];
	
	/* This will transform the string into raw binary encoded utf-8 */
	lu_common_decode_header(header, &buf[0], sizeof(buf), coding);
	
	/* Now, write it as an xml escaped string */
	return my_service_write_str(h, &buf[0]);
	
	return 0;
}

static int my_service_write_time(
	My_Service_Handle	h,
	lu_quad			t)
{
	char  buf[80];
	char* e;
	time_t tm = t;
	
	strcpy(&buf[0], ctime(&tm));
	for (e = &buf[0]; *e; e++)
		if (*e == '\n')
			break;
	
	*e = 0;
	
	return my_service_write_str(h, &buf[0]);
}

static int my_service_write_url(
	My_Service_Handle	h,
	const char*		buf)
{
	char hex[8];
	const char* start;

	for (start = buf; *buf; buf++)
	{
		if (!isalnum(*buf))
		{
			snprintf(&hex[0], sizeof(hex), "%%%2X", *buf);
			
			if (my_service_buffer_writel(h, start, buf - start) != 0)
				return -1;
			if (my_service_write_str(h, &hex[0]) != 0)
				return -1;
			
			start = buf+1;
		}
	}
	
	if (my_service_buffer_writel(h, start, buf - start) != 0)
		return -1;
	
	return 0;
	
}

static int my_service_xml_head(
	My_Service_Handle h)
{
	return my_service_buffer_write(h, 
		"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		"<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/render.xsl\"?>\n"
		);
}

static int my_service_server(
	My_Service_Handle h)
{
	if (my_service_buffer_write(h, " <server>\n  <hostname>")       != 0) return -1;
	if (my_service_write_str(h, lu_config_list_host)                != 0) return -1;
	if (my_service_buffer_write(h, "</hostname>\n  <email name=\"") != 0) return -1;
	if (my_service_write_str(h, lu_config_admin_name)               != 0) return -1;
	if (my_service_buffer_write(h, "\" address=\"")                 != 0) return -1;
	if (my_service_write_str(h, lu_config_admin_address)            != 0) return -1;
	if (my_service_buffer_write(h, "\"/>\n </server>\n")            != 0) return -1;
	
	return 0;
}

static int my_service_error(
	My_Service_Handle h,
	const char* title,
	const char* error,
	const char* detail)
{
	if (my_service_buffer_init(h, "text/xml\n", 0, 0, 0)    != 0) return -1;
	if (my_service_xml_head(h)                              != 0) return -1;
	if (my_service_buffer_write(h, "<error>\n <title>")     != 0) return -1;
	if (my_service_write_str(h, title)                      != 0) return -1;
	if (my_service_buffer_write(h, "</title>\n <message>")  != 0) return -1;
	if (my_service_write_str(h, error)                      != 0) return -1;
	if (my_service_buffer_write(h, "</message>\n <detail>") != 0) return -1;
	if (my_service_write_str(h, detail)                     != 0) return -1;
	if (my_service_buffer_write(h, "</detail>\n")           != 0) return -1;
	if (my_service_server(h)                                != 0) return -1;
	if (my_service_buffer_write(h, "</error>\n")            != 0) return -1;
	
	return 0;
}

static int my_service_addresses(
	My_Service_Handle h,
	ADDRESS* addr, 
	const char* name,
	const char* coding)
{
	if (!addr) return 0;
	
	if (my_service_buffer_write(h, " <") != 0) return -1;
	if (my_service_write_str   (h, name) != 0) return -1;
	if (my_service_buffer_write(h, ">" ) != 0) return -1;
	
	for (; addr; addr = addr->next)
	{
		if (!addr->personal && (!addr->mailbox || !addr->host))
		{
			continue;
		}
		
		if (my_service_buffer_write(h, "<email") != 0) return -1;
		
		if (addr->personal)
		{
			if (my_service_buffer_write(h, " name=\""            ) != 0) return -1;
			if (my_service_write_ehead (h, addr->personal, coding) != 0) return -1;
			if (my_service_buffer_write(h, "\""                  ) != 0) return -1;
		}
		
		if (addr->mailbox && addr->host)
		{
			if (my_service_buffer_write(h, " address=\"") != 0) return -1;
			if (my_service_write_str   (h, addr->mailbox) != 0) return -1;
			if (my_service_buffer_write(h, "@"          ) != 0) return -1;
			if (my_service_write_str   (h, addr->host   ) != 0) return -1;
			if (my_service_buffer_write(h, "\""         ) != 0) return -1;
		}
		
		if (my_service_buffer_write(h, "/>") != 0) return -1;
	}
	
	if (my_service_buffer_write(h, "</")  != 0) return -1;
	if (my_service_write_str   (h, name)  != 0) return -1;
	if (my_service_buffer_write(h, ">\n") != 0) return -1;
	
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
	My_Service_Handle	h,
	const char*		dat,
	size_t			len,
	const char*		coding)
{
	char*	b;
	char	buf[1024];
	size_t	fill, tmp;
	iconv_t	ic;
	
	/* Make a guess at what the coding is supposed to be. */
	if (!coding) coding = "iso-8859-1";
	
	ic = iconv_open("utf-8", coding);
	if (ic == (iconv_t)-1)
	{
		/* Don't know this encoding - just dump the data anyways */
		syslog(LOG_WARNING, "Unknown coding: %s\n", coding);
		my_service_write_str(h, "\n*** WARNING: UNKNOWN CHARSET '");
		my_service_write_str(h, coding);
		my_service_write_str(h, "' FALLING BACK TO ISO-8869-1 ***\n");
		
		coding = "iso-8869-1";
		ic = iconv_open("utf-8", coding);
	}
	
	while (len)
	{
		fill = sizeof(buf);
		b = &buf[0];
		tmp = iconv(ic, (ICONV_CAST)&dat, &len, &b, &fill);
		
		/*!!! Search the message for URLs, email address, quotations, etc */
		if (my_service_write_strl(h, &buf[0], sizeof(buf) - fill) != 0)
			break;
		
		if (tmp == (size_t)-1 && errno != E2BIG)
			break;
	}
	
	iconv_close(ic);
	
	if (len)
	{
		my_service_write_str(h, "\n*** ERROR: BROKEN CHARSET '");
		my_service_write_str(h, coding);
		my_service_write_str(h, "' DURING DECODE ***\n");
	}
	
	return 0;
}

static int my_service_traverse(
	My_Service_Handle	h,
	struct Lu_Mbox_Message* in, 
	struct mail_bodystruct* body,
	int			count)
{
	struct mail_body_part*	p;
	size_t			length;
	char*			buffer;
	int			nfree;
	char*			name;
	const char*		charset;
	
	if (!body)
	{	/* You never know... */
		return count;
	}
	
	if (body->type > 8) body->type = 8;
	
	name    = my_service_find_name(body);
	charset = lu_mbox_find_charset(body);
	
	if (my_service_buffer_write(h, "<mime id=\"") != 0) return -1;
	if (my_service_write_int   (h, count        ) != 0) return -1;
	if (my_service_buffer_write(h, "\" type=\"" ) != 0) return -1;
	if (my_service_buffer_write(h, 
	                 my_service_mime[body->type]) != 0) return -1;
	if (my_service_buffer_write(h, "/"          ) != 0) return -1;
	if (my_service_buffer_write(h, body->subtype) != 0) return -1;
	if (my_service_buffer_write(h, "\""         ) != 0) return -1;
	
	if (name)
	{
		if (my_service_buffer_write(h, " name=\"") != 0) return -1;
		if (my_service_write_str   (h, name      ) != 0) return -1;
		if (my_service_buffer_write(h, "\""      ) != 0) return -1;
	}
	
	if (my_service_buffer_write(h, ">") != 0) return -1;

	count++;
	
	switch ((int)body->type)
	{
		case TYPEMESSAGE:
			/* This part contains an encapsulated message.
			 */
			count = my_service_traverse(h, in, 
					body->nested.msg->body, count);
				
			if (count == -1) return -1;
			break;

		case TYPETEXT:
			/*
			 * This is what we want to display -- stop.
			 */
			buffer = lu_mbox_select_body(in, body, &length, &nfree);
			
			if (my_service_dump(h, buffer, length, charset) != 0)
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
				count = my_service_traverse(h, in,
						&p->body, count);
				if (count == -1) break;
			}
			
			break;
			
		default:
			break;
	}
	
	if (my_service_buffer_write(h, "</mime>\n") != 0) return -1;
	
	return count;
}

static struct mail_bodystruct* my_service_attach_id(
	struct Lu_Mbox_Message* in, 
	struct mail_bodystruct* body,
	int*			count)
{
	struct mail_bodystruct* s;
	struct mail_body_part*	p;
	
	if (*count == 0)
	{	/* Base case */
		return body;
	}
	
	if (!body)
	{	/* You never know... */
		return 0;
	}
	
	(*count)--;
	
	switch ((int)body->type)
	{
		case TYPEMESSAGE:
			return my_service_attach_id(in, 
				body->nested.msg->body, count);
			break;

		case TYPEMULTIPART:
			/*
			 * Multipart message.  Look at the nested parts
			 * and hopefully find some plaintext.
			 */
			for (p = body->nested.part; p != NULL; p = p->next)
			{
				s = my_service_attach_id(in,
					&p->body, count);
				if (s) return s;
			}
			break;
	}
	
	return 0;
}

static int my_service_list(
	My_Service_Handle h,
	Lu_Config_List* l,
	message_id msgs)
{
	if (my_service_buffer_write(h, " <list>\n  <id>")       != 0) return -1;
	if (my_service_write_int(h, l->id)                      != 0) return -1;
	if (my_service_buffer_write(h, "</id>\n")               != 0) return -1;
	
	if (msgs != lu_common_minvalid)
	{
		if (my_service_buffer_write(h, "  <messages>")  != 0) return -1;
		if (my_service_write_int(h, msgs)               != 0) return -1;
		if (my_service_buffer_write(h, "</messages>\n") != 0) return -1;
	}
	
	if (my_service_buffer_write(h, "  <email") != 0) return -1;
	
	if (l->name)
	{
		if (my_service_buffer_write(h, " name=\"")  != 0) return -1;
		if (my_service_write_str(h, l->name)        != 0) return -1;
		if (my_service_buffer_write(h, "\"")        != 0) return -1;
	}
	
	if (l->address)
	{
		if (my_service_buffer_write(h, " address=\"") != 0) return -1;
		if (my_service_write_str(h, l->address)       != 0) return -1;
		if (my_service_buffer_write(h, "\"")          != 0) return -1;
	}
	
	if (my_service_buffer_write(h, "/>\n") != 0) return -1;
	
	if (l->description)
	{
		if (my_service_buffer_write(h, "  <description>")  != 0) return -1;
		if (my_service_write_str(h, l->description)        != 0) return -1;
		if (my_service_buffer_write(h, "</description>\n") != 0) return -1;
	}
	
	if (my_service_buffer_write(h, " </list>\n") != 0) return -1;
	
	return 0;
}

static int my_service_summary_body(
	My_Service_Handle	h,
	message_id		id,
	Lu_Summary_Message*	msg)
{
	if (my_service_buffer_write(h, "   <id>")                 != 0) return -1;
	if (my_service_write_int(h, id)                           != 0) return -1;
	if (my_service_buffer_write(h, "</id>\n   <timestamp>")   != 0) return -1;
	if (my_service_write_int(h, msg->timestamp)               != 0) return -1;
	if (my_service_buffer_write(h, "</timestamp>\n   <time>") != 0) return -1;
	if (my_service_write_time(h, msg->timestamp)              != 0) return -1;
	if (my_service_buffer_write(h, "</time>\n   <thread>")    != 0) return -1;
	if (my_service_write_int(h, msg->thread)                  != 0) return -1;
	if (my_service_buffer_write(h, "</thread>\n")             != 0) return -1;
	
	if (lu_summary_write_variable(
		(int(*)(void*, const char*))        &my_service_buffer_write,
		(int(*)(void*, const char*, size_t))&my_service_write_strl,
		h,
		msg->flat_offset) != 0) return -1;
	
	return 0;
}

static int my_service_summary(
	My_Service_Handle	h,
	message_id		id)
{
	Lu_Summary_Message	msg;
	
	msg = lu_summary_read_msummary(id);
	
	if (my_service_buffer_write(h, "  <summary>\n")  != 0) return -1;
	if (my_service_summary_body(h, id, &msg)         != 0) return -1;
	if (my_service_buffer_write(h, "  </summary>\n") != 0) return -1;
	
	return 0;
}

static int my_service_mbox(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	message_id	id;
	lu_addr		bits;
	
	lu_word		list_id;
	lu_word		mbox_id;
	off_t		offset;
	
	Lu_Summary_Message		msg;
	struct Lu_Config_Message	cmsg;
	
	Lu_Config_List*		list;
	Lu_Config_Mbox*		mbox;
	
	char*		base;
	size_t		len;
	
	if (strcmp(ext, "txt"))
	{
		my_service_error(h,
			"Malformed request",
			"Lurkerd received a request for a non-textual mbox",
			request);
		goto my_service_mbox_error0;
	}
	
	id = lu_summary_lookup_mid(request);
	if (id == lu_common_minvalid)
	{
		my_service_error(h,
			"Message does not exist",
			"Lurkerd received a request for a non-existant message",
			request);
		goto my_service_mbox_error0;
	}
	
	msg = lu_summary_read_msummary(id);
	if (msg.timestamp == 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd failed to retrieve the summary for the named message",
			request);
		goto my_service_mbox_error0;
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
		my_service_error(h,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailing list",
			request);
		goto my_service_mbox_error0;
	}
	
	mbox = lu_config_find_mbox(list, mbox_id);
	if (!mbox)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailbox",
			request);
		goto my_service_mbox_error0;
	}
	
	if (lu_mbox_map_message(mbox, &cmsg, offset) != 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd was unable to mmap the message into memory",
			request);
		goto my_service_mbox_error0;
	}
	
	base = cmsg.map.base;
	base += cmsg.headers - cmsg.map.off;
	len = cmsg.end - cmsg.headers;
	
	if (my_service_buffer_init(h, "text/plain\n", 1, 
			lu_config_cache_message_ttl, LU_EXPIRY_NO_LIST) != 0)
		goto my_service_mbox_error1;
	if (my_service_buffer_writel(h, base, len) != 0)
		goto my_service_mbox_error1;
	
	lu_mbox_destroy_map(&cmsg);
	return 0;

my_service_mbox_error1:
	lu_mbox_destroy_map(&cmsg);
	
my_service_mbox_error0:
	return -1;
}

static int my_service_attach(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	message_id	id;
	int		aid;
	lu_addr		bits;
	char*		mid;
	
	lu_word		list_id;
	lu_word		mbox_id;
	off_t		offset;
	
	Lu_Summary_Message		msg;
	struct Lu_Config_Message	cmsg;
	struct Lu_Mbox_Message		mmsg;
	
	Lu_Config_List*		list;
	Lu_Config_Mbox*		mbox;
	
	struct mail_bodystruct*	attach;
	char*			base;
	size_t			len;
	int			nfree;
	char			buf[100];
	
	if (strcmp(ext, "attach"))
	{
		my_service_error(h,
			"Malformed request",
			"Lurkerd received an attachment request for non .attach",
			ext);
		
		goto my_service_attach_error0;
	}
	
	mid = strchr(request, '@');
	if (!mid || sscanf(request, "%d@", &aid) != 1)
	{	/* There was nothing valid, or it did not end in whitespace
		 * or a null.
		 */
		my_service_error(h,
			"Malformed request",
			"Lurkerd received a request for a non-numeric attachment",
			request);
		goto my_service_attach_error0;
	}
	mid++;
	
	id = lu_summary_lookup_mid(mid);
	if (id == lu_common_minvalid)
	{
		my_service_error(h,
			"Message does not exist",
			"Lurkerd received a request for a non-existant message",
			mid);
		goto my_service_attach_error0;
	}
	
	msg = lu_summary_read_msummary(id);
	if (msg.timestamp == 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd failed to retrieve the summary for the named message",
			request);
		goto my_service_attach_error0;
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
		my_service_error(h,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailing list",
			request);
		goto my_service_attach_error0;
	}
	
	mbox = lu_config_find_mbox(list, mbox_id);
	if (!mbox)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailbox",
			request);
		goto my_service_attach_error0;
	}
	
	if (lu_mbox_map_message(mbox, &cmsg, offset) != 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd was unable to mmap the message into memory",
			request);
		goto my_service_attach_error0;
	}
	
	if (lu_mbox_parse_message(&cmsg, &mmsg) != 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd was unable to parse the message",
			request);
		goto my_service_attach_error1;
	}
	
	attach = my_service_attach_id(&mmsg, mmsg.body, &aid);
	
	if (!attach)
	{
		my_service_error(h,
			"Malformed request",
			"Lurkerd received a request for a non-existant attachment",
			request);
		goto my_service_attach_error2;
	}
	
	if (attach->type > 8) attach->type = 8;
	
	snprintf(&buf[0], sizeof(buf), "%s/%s\n",
		my_service_mime[attach->type], attach->subtype);
	
	if (my_service_buffer_init(h, buf, 0, 0, 0) != 0)
		goto my_service_attach_error2;
	
	base = lu_mbox_select_body(&mmsg, attach, &len, &nfree);
	
	if (my_service_buffer_writel(h, base, len) != 0)
		goto my_service_attach_error3;
	
	if (nfree) fs_give((void**)&base);
	lu_mbox_destroy_message(&mmsg);
	lu_mbox_destroy_map(&cmsg);
	return 0;

my_service_attach_error3:
	if (nfree) fs_give((void**)&base);
	
my_service_attach_error2:
	lu_mbox_destroy_message(&mmsg);
	
my_service_attach_error1:
	lu_mbox_destroy_map(&cmsg);
	
my_service_attach_error0:
	return -1;
}

static int my_service_message(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	message_id	id;
	lu_addr		bits;
	
	lu_word		list_id;
	lu_word		mbox_id;
	off_t		offset;
	
	Lu_Summary_Message		msg;
	struct Lu_Config_Message	cmsg;
	struct Lu_Mbox_Message		mmsg;
	
	Lu_Config_List*		list;
	Lu_Config_Mbox*		mbox;
	
	Lu_Breader_Handle	b;
	char			keyword[LU_KEYWORD_LEN+1];
	const char*		coding;
	
	message_id		count, ind, get, i, buf[1024];
	
	if (strcmp(ext, "xml") && strcmp(ext, "html"))
	{
		my_service_error(h,
			"Malformed request",
			"Lurkerd received a message request for non html/xml",
			ext);
		
		goto my_service_message_error0;
	}
	
	id = lu_summary_lookup_mid(request);
	if (id == lu_common_minvalid)
	{
		my_service_error(h,
			"Message does not exist",
			"Lurkerd received a request for a non-existant message",
			request);
		goto my_service_message_error0;
	}
	
	msg = lu_summary_read_msummary(id);
	if (msg.timestamp == 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd failed to retrieve the summary for the named message",
			request);
		goto my_service_message_error0;
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
		my_service_error(h,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailing list",
			request);
		goto my_service_message_error0;
	}
	
	mbox = lu_config_find_mbox(list, mbox_id);
	if (!mbox)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd has a message which refers to a missing mailbox",
			request);
		goto my_service_message_error0;
	}
	
	if (lu_mbox_map_message(mbox, &cmsg, offset) != 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd was unable to mmap the message into memory",
			request);
		goto my_service_message_error0;
	}
	
	if (lu_mbox_parse_message(&cmsg, &mmsg) != 0)
	{
		my_service_error(h,
			"Internal Error",
			"Lurkerd was unable to parse the map'd message",
			request);
		goto my_service_message_error1;
	}
	
	if (mmsg.env->message_id)
	{
		snprintf(&keyword[0], sizeof(keyword), "%s%s",
			LU_KEYWORD_REPLY_TO,
			lu_common_cleanup_id(mmsg.env->message_id));
		
		b = lu_breader_new(&keyword[0]);
		if (b == 0)
		{
			my_service_error(h,
				"Internal Error",
				"Lurkerd was unable to access the reply-to keyword",
				request);
			goto my_service_message_error2;
		}
	}
	else
	{
		b = 0;
	}
	
	coding = lu_mbox_find_charset(mmsg.body);
	
	if (my_service_buffer_init(h, "text/xml\n", 1, 
			lu_config_cache_message_ttl, LU_EXPIRY_NO_LIST) != 0)
		goto my_service_message_error3;
	
	if (my_service_xml_head(h)                       != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, "<message>\n")    != 0) goto my_service_message_error3;
	if (my_service_server(h)                         != 0) goto my_service_message_error3;
	if (my_service_list(h, list, lu_common_minvalid) != 0) goto my_service_message_error3;
	
	if (my_service_buffer_write(h, " <id>")                 != 0) goto my_service_message_error3;
	if (my_service_write_int   (h, id)                      != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, "</id>\n <timestamp>")   != 0) goto my_service_message_error3;
	if (my_service_write_int   (h, msg.timestamp)           != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, "</timestamp>\n <time>") != 0) goto my_service_message_error3;
	if (my_service_write_time  (h, msg.timestamp)           != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, "</time>\n <thread>")    != 0) goto my_service_message_error3;
	if (my_service_write_int   (h, msg.thread)              != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, "</thread>\n")           != 0) goto my_service_message_error3;
	
	if (msg.in_reply_to != lu_common_minvalid)
	{
		if (my_service_buffer_write(h, " <inreplyto>\n")  != 0) goto my_service_message_error3;
		if (my_service_summary(h, msg.in_reply_to)        != 0) goto my_service_message_error3;
		if (my_service_buffer_write(h, " </inreplyto>\n") != 0) goto my_service_message_error3;
	}
	
	if (b != 0 && (count = lu_breader_records(b)) != 0)
	{
 		ind = 0;
 		
		if (my_service_buffer_write(h, " <replies>\n") != 0) goto my_service_message_error3;
		
 		while (count)
 		{
 			get = count;
 			if (get > sizeof(buf)/sizeof(message_id))
 				get = sizeof(buf)/sizeof(message_id);
 			
 			if (lu_breader_read(b, ind, get, &buf[0]) != 0)
 				goto my_service_message_error3;
 			
 			for (i = 0; i < get; i++)
 			{
 				if (my_service_summary(h, buf[i]) != 0)
 					goto my_service_message_error3;
 			}
 			
 			ind   += get;
 			count -= get;
 		}
 		
		if (my_service_buffer_write(h, " </replies>\n") != 0) goto my_service_message_error3;
 	}
	
	if (my_service_addresses(h, mmsg.env->from,     "from",     coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->sender,   "sender",   coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->reply_to, "reply-to", coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->to,       "to",       coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->cc,       "cc",       coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->bcc,      "bcc",      coding) != 0) goto my_service_message_error3;
	
	if (mmsg.env->message_id)
	{
		if (my_service_buffer_write(h, " <mid>"  ) != 0) goto my_service_message_error3;
		if (my_service_write_str   (h, request   ) != 0) goto my_service_message_error3;
		if (my_service_buffer_write(h, "</mid>\n") != 0) goto my_service_message_error3;
	}
	
	if (my_service_buffer_write(h, " <subject>") != 0) goto my_service_message_error3;
	if (mmsg.env->subject && mmsg.env->subject[0])
	{
		if (my_service_write_ehead (h, mmsg.env->subject, coding) != 0) goto my_service_message_error3;
	}
	else
	{
		if (my_service_buffer_write(h, "No subject") != 0) goto my_service_message_error3;
	}
	if (my_service_buffer_write(h, "</subject>\n") != 0) goto my_service_message_error3;
	
	if (my_service_traverse(h, &mmsg, mmsg.body, 0) == -1) goto my_service_message_error3;
	if (my_service_buffer_write(h, "</message>\n")  != 0) goto my_service_message_error3;
	
	if (b != 0) lu_breader_free(b);
	lu_mbox_destroy_message(&mmsg);
	lu_mbox_destroy_map(&cmsg);
	return 0;
	
my_service_message_error3:
	if (b != 0) lu_breader_free(b);
my_service_message_error2:
	lu_mbox_destroy_message(&mmsg);
my_service_message_error1:
	lu_mbox_destroy_map(&cmsg);
my_service_message_error0:
	return -1;
}

static int my_service_mindex(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	Lu_Breader_Handle	b;
	Lu_Config_List*		l;
	message_id		offset;
	int			list;
	char			keyword[40];
	message_id		ids[LU_PROTO_INDEX];
	int			i;
	message_id		count;
	
	if (sscanf(request, "%d@%d", &list, &offset) != 2)
	{	/* They did something funny. */
		my_service_error(h,
			"Malformed request",
			"Lurkerd received an improperly formatted mindex request",
			request);
		
		goto my_service_mindex_error0;
	}
	
	if (strcmp(ext, "xml") && strcmp(ext, "html"))
	{
		my_service_error(h,
			"Malformed request",
			"Lurkerd received an index request for non html/xml",
			ext);
		
		goto my_service_mindex_error0;
	}
	
	if ((offset % LU_PROTO_INDEX) != 0)
	{	/* Must be a multiple of the index */
		my_service_error(h,
			"Malformed request",
			"Lurkerd received an unaligned mindex request",
			request);
		
		goto my_service_mindex_error0;
	}
	
	l = lu_config_find_list(list);
	if (l == 0)
	{
		my_service_error(h,
			"Invalid request",
			"Lurkerd received a request for a non-existant list",
			request);
		
		goto my_service_mindex_error0;
	}
	
	sprintf(&keyword[0], "%s%d", LU_KEYWORD_LIST, list);
	b = lu_breader_new(&keyword[0]);
	if (b == 0)
	{
		my_service_error(h,
			"Internal error",
			"Lurkerd failed to access the keyword database",
			"server failure - see log files");
		
		goto my_service_mindex_error0;
	}
	
	if (offset >= lu_breader_records(b))
	{
		my_service_error(h,
			"Invalid request",
			"Lurkerd received a mindex request past the end of the list",
			request);
		
		goto my_service_mindex_error1;
	}
	
	count = lu_breader_records(b) - offset;
	if (count > LU_PROTO_INDEX)
		count = LU_PROTO_INDEX;
	
	if (lu_breader_read(b, offset, count, &ids[0]) != 0)
	{
		my_service_error(h,
			"Internal error",
			"Lurkerd failed to retrieve the records from the keyword file",
			"server failure - see log files");
		
		goto my_service_mindex_error1;
	}
	
	if (my_service_buffer_init(h, "text/xml\n", 1, 
			lu_config_cache_index_ttl, list) != 0)
		goto my_service_mindex_error1;
	
	if (my_service_xml_head(h)                            != 0) goto my_service_mindex_error1;
	if (my_service_buffer_write(h, "<mindex>\n <offset>") != 0) goto my_service_mindex_error1;
	if (my_service_write_int(h, offset)                   != 0) goto my_service_mindex_error1;
	if (my_service_buffer_write(h, "</offset>\n")         != 0) goto my_service_mindex_error1;
	
	if (offset + count != lu_breader_records(b))
	{
		if (my_service_buffer_write(h, " <next>")            != 0) goto my_service_mindex_error1;
		if (my_service_write_int(h, offset + LU_PROTO_INDEX) != 0) goto my_service_mindex_error1;
		if (my_service_buffer_write(h, "</next>\n")          != 0) goto my_service_mindex_error1;
	}
	
	if (offset != 0)
	{
		if (my_service_buffer_write(h, " <prev>")            != 0) goto my_service_mindex_error1;
		if (my_service_write_int(h, offset - LU_PROTO_INDEX) != 0) goto my_service_mindex_error1;
		if (my_service_buffer_write(h, "</prev>\n")          != 0) goto my_service_mindex_error1;
	}
	
	if (my_service_server(h) != 0) goto my_service_mindex_error1;
	
	if (my_service_list(h, l, lu_breader_records(b)) != 0) goto my_service_mindex_error1;
	lu_breader_free(b);
	
	for (i = 0; i < count; i++)
	{
		if (my_service_summary(h, ids[i]) != 0) goto my_service_mindex_error0;
	}
	
	if (my_service_buffer_write(h, "</mindex>\n") != 0) goto my_service_mindex_error0;
	
	return 0;
	
my_service_mindex_error1:
	lu_breader_free(b);
	
my_service_mindex_error0:
	return -1;
}

static int my_service_tree_compare(const void* a, const void* b)
{
	message_id key = *(message_id*)a;
	message_id val = ((const My_Service_Reply_Tree*)b)->id;
	
	if (key < val)	return -1;
	if (key > val)	return 1;
	return 0;
}

static void my_service_init_tree(
	My_Service_Reply_Tree* tree, 
	int tree_size)
{
	My_Service_Reply_Tree*	bs;
	int	i, p;
	int*	w;
	
	for (i = 0; i < tree_size; i++)
	{
		tree[i].replyee = -1;
		if (tree[i].summary.in_reply_to != lu_common_minvalid)
		{
			bs = bsearch(
				&tree[i].summary.in_reply_to,
				tree,
				tree_size,
				sizeof(My_Service_Reply_Tree),
				&my_service_tree_compare);
			
			if (bs)
				tree[i].replyee = (bs - tree);
			
			if (tree[i].replyee >= i)
				tree[i].replyee = -1;
		}
		
		if (tree[i].replyee >= 0) tree[tree[i].replyee].replies++;
		
		tree[i].replies       = 0;
		tree[i].replyor_first = -1;
		tree[i].replyor_next  = -1;
		tree[i].draw_next     = -1;
		tree[i].depth         = i;
		tree[i].consumed      = 0;
	}
	
	for (i = tree_size-1; i > 0; i--)
	{
		p = tree[i].replyee;
		if (p == -1) continue;
				
		if (tree[p].depth < tree[i].depth)
			tree[p].depth = tree[i].depth;
		
		/* Insertion sort is not that fast...
		 * But compared to the drawing algo, this n^2 is negligable.
		 */
		w = &tree[p].replyor_first;
		while (*w != -1 && tree[*w].depth > tree[i].depth)
			w = &tree[*w].replyor_next;
		
		tree[i].replyor_next = *w;
		*w                   = i;
	}
}

/* return what we use up to */
static int my_service_draw_tree(
	My_Service_Reply_Tree* tree, 
	int n, int u)
{
	int i;
	int c;
	int col = 0;
	
	/* Find the column we can use. */
	for (i = u; i <= tree[n].depth; i++)
		if (col < tree[i].consumed)
			col = tree[i].consumed;
	
	tree[n].consumed = col;
	
	tree[n].column = col;
	for (c = tree[n].replyor_first; c != -1; c = tree[c].replyor_next)
	{
		col = my_service_draw_tree(tree, c, n); 
		for (i = n; i < c; i++)
			tree[i].consumed = col+1;
	}
	
	return tree[n].column;
}

static void my_service_draw_row(
	My_Service_Handle	h,
	My_Service_Reply_Tree*	tree, 
	int* head, 
	int i)
{
	int	col, p;
	int*	w;

#ifdef DEBUG
	printf("\nOffset: %d", tree[i].column);
#endif
	
	col = 0;
	w = head;
	while (*w != -1)
	{
		for (; col < tree[*w].column; col++)
			my_service_buffer_write(h, EMPTY_CELL);
		
		if (*w == i) break;
		
		my_service_buffer_write(h, BAR_NS);
		col++;
		w = &tree[*w].draw_next;
	}
	
	if (tree[i].replyee == -1)
	{
		switch (tree[i].replies)
		{
		case 0:  my_service_buffer_write(h, TOPMESSAGE_END);  break;
		case 1:  my_service_buffer_write(h, TOPMESSAGE_DOWN); break;
		default: my_service_buffer_write(h, TOPMESSAGE_BOTH); break;
		}
	}
	else
	{
		switch (tree[i].replies)
		{
		case 0:  my_service_buffer_write(h, MESSAGE_END);  break;
		case 1:  my_service_buffer_write(h, MESSAGE_DOWN); break;
		default: my_service_buffer_write(h, MESSAGE_BOTH); break;
		}
	}
	col++;
	
	/* Cut ourselves out of the list and graft on our
	 * children. While we're at it, draw the links.
	 */
	for (p = tree[i].replyor_first; p != -1; p = tree[p].replyor_next)
	{
		*w = p;
		w = &tree[p].draw_next;
		
		if (col > tree[p].column) continue;
		
		for (; col < tree[p].column; col++)
			my_service_buffer_write(h, BAR_EW);
		col++;
		if (tree[p].replyor_next == -1)
			my_service_buffer_write(h, CORNER_SW);
		else
			my_service_buffer_write(h, TEE_WSE);
	}
	*w = tree[i].draw_next;
	
	/* Continue drawing the other children */
	for (p = *w; p != -1; p = tree[p].draw_next)
	{
		for (; col < tree[p].column; col++)
			my_service_buffer_write(h, EMPTY_CELL);
		col++;
		my_service_buffer_write(h, BAR_NS);
	}
}

static int my_service_thread(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	My_Service_Reply_Tree*	tree;
	int			tree_size;
	int			draw_head;
	int			i;
	message_id		id;
	char			key[20];
	Lu_Breader_Handle	b;
	
	if (strcmp(ext, "xml") && strcmp(ext, "html"))
	{
		my_service_error(h,
			"Malformed request",
			"Lurkerd received a thread request for non html/xml",
			ext);
		
		goto my_service_thread_error0;
	}
	
	if (sscanf(request, "%d", &id) != 1)
	{	/* They did something funny. */
		my_service_error(h,
			"Malformed request",
			"Lurkerd received an improperly formatted thread request",
			request);
		
		goto my_service_thread_error0;
	}
	
	snprintf(&key[0], sizeof(key), "%s%d", LU_KEYWORD_THREAD, id);
	b = lu_breader_new(&key[0]);
	if (b == 0)
	{
		my_service_error(h,
			"Internal Server Error",
			"Lurkerd failed to open a breader for thread",
			request);
		
		goto my_service_thread_error0;
	}
	
	if (lu_breader_records(b) == 0)
	{
		my_service_error(h,
			"Thread Not Found",
			"Lurkerd received a request for a non-existant thread",
			request);
		
		goto my_service_thread_error1;
	}
	
	tree_size = lu_breader_records(b);
	tree = malloc(tree_size * sizeof(My_Service_Reply_Tree));
	if (!tree)
	{
		my_service_error(h,
			"Out of Memory",
			"Lurkerd ran out of memory processing this thread",
			request);
		
		goto my_service_thread_error1;
	}
	
	for (i = 0; i < tree_size; i++)
	{
		if (lu_breader_read(b, i, 1, &tree[i].id) != 0)
		{
			my_service_error(h,
				"Internal Server Error",
				"Lurkerd was unable to read a breader",
				request);
		
			goto my_service_thread_error2;
		}
		
		tree[i].summary = lu_summary_read_msummary(tree[i].id);
		if (tree[i].summary.timestamp == 0)
		{
			my_service_error(h,
				"Internal Server Error",
				"Lurkerd was unable to read a summary record",
				request);
		
			goto my_service_thread_error2;
		}
	}
	
	my_service_init_tree(tree, tree_size);
	for (i = 0; i < tree_size; i++)
	{
		if (tree[i].replyee == -1)
			my_service_draw_tree(tree, i, i);
	}
	
	if (my_service_buffer_init(h, "text/xml\n", 1, 
			lu_config_cache_message_ttl, LU_EXPIRY_NO_LIST) != 0)
		goto my_service_thread_error2;
	
	if (my_service_xml_head(h)                        != 0) goto my_service_thread_error2;
	if (my_service_buffer_write(h, "<thread>\n <id>") != 0) goto my_service_thread_error2;
	if (my_service_write_int(h, id)                   != 0) goto my_service_thread_error2;
	if (my_service_buffer_write(h, "</id>\n")         != 0) goto my_service_thread_error2;
	if (my_service_server(h)                          != 0) goto my_service_thread_error2;
	
	draw_head = -1;
	
	for (i = 0; i < tree_size; i++)
	{
		my_service_buffer_write(h, " <summary>\n") ;
		my_service_summary_body(h, tree[i].id, &tree[i].summary);
		my_service_buffer_write(h, "   <draw>") ;
		my_service_draw_row(h, tree, &draw_head, i);
		my_service_buffer_write(h, "</draw>\n </summary>\n") ;
	}
	
	if (my_service_buffer_write(h, "</thread>\n") != 0) goto my_service_thread_error2;
	
	free(tree);
	lu_breader_free(b);
	return 0;
	
my_service_thread_error2:
	free(tree);
my_service_thread_error1:
	lu_breader_free(b);
my_service_thread_error0:
	return -1;
}

static int my_service_search(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	int		i;
	message_id	out;
	message_id	offset;
	const char*	delim;
	const char*	detail;
	
	delim = strchr(request, ' ');
	
	if (delim == 0)
	{
		my_service_error(h,
			"Invalid request",
			"Lurkerd received a search request with no offset",
			request);
		goto my_service_search_error0;
	}
	
	offset = atol(request);
	if ((offset % LU_PROTO_INDEX) != 0)
	{	/* Must be a multiple of the index */
		my_service_error(h,
			"Malformed request",
			"Lurkerd received an unaligned search request",
			request);
		
		goto my_service_search_error0;
	}
	
	if (offset > 1000)
	{
		my_service_error(h,
			"Malformed request",
			"Lurkerd received a search request for too large an offset",
			request);
		goto my_service_search_error0;
	}
	
	if (lu_search_start(delim+1, &detail) != 0)
	{
		my_service_error(h,
			"Search failed",
			"The lurkerd server rejected this search",
			detail);
		goto my_service_search_error0;
	}
	
	for (i = 0; i < offset; i++)
	{
		if (lu_search_result(&out) != 0)
		{
			my_service_error(h,
				"Internal error",
				"The lurkerd server failed to search",
				"lu_search_result failure - see logs");
			goto my_service_search_error1;
		}
		
		if (out == lu_common_minvalid)
		{
			my_service_error(h,
				"Search failed",
				"The lurkerd server rejected this search",
				"There are not that many search results");
			goto my_service_search_error1;
		}
	}
	
	if (my_service_buffer_init(h, "text/xml\n", 1, 
			lu_config_cache_search_ttl, LU_EXPIRY_NO_LIST) != 0)
		goto my_service_search_error1;
	
	/* Ok! Now, lets start putting out the data */
	if (my_service_xml_head(h)                             != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "<search>\n <offset>")  != 0) goto my_service_search_error1;
	if (my_service_write_int(h, offset)                    != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "</offset>\n <query>")  != 0) goto my_service_search_error1;
	if (my_service_write_str(h, delim+1)                   != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "</query>\n <queryurl>")!= 0) goto my_service_search_error1;
	if (my_service_write_url(h, delim+1)                   != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "</queryurl>\n <hits>") != 0) goto my_service_search_error1;
	if (my_service_write_int(h, 0)                         != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "</hits>\n")            != 0) goto my_service_search_error1;
	if (my_service_server(h)                               != 0) goto my_service_search_error1;
	
	if (offset != 0)
	{
		if (my_service_buffer_write(h, " <prev>")            != 0) goto my_service_search_error1;
		if (my_service_write_int(h, offset - LU_PROTO_INDEX) != 0) goto my_service_search_error1;
		if (my_service_buffer_write(h, "</prev>\n")          != 0) goto my_service_search_error1;
	}
	
	for (i = 0; i < LU_PROTO_INDEX; i++)
	{
		if (lu_search_result(&out) != 0)
		{
			my_service_error(h,
				"Internal error",
				"The lurkerd server failed to search",
				"lu_search_result failure - see logs");
			goto my_service_search_error1;
		}
		
		if (out == lu_common_minvalid)
		{
			break;
		}
		
		if (my_service_summary(h, out) != 0) 
		{
			goto my_service_search_error1;
		}
	}
	
	if (out != lu_common_minvalid && lu_search_result(&out) == 0 &&
	    out != lu_common_minvalid)
	{
		if (my_service_buffer_write(h, " <next>")            != 0) goto my_service_search_error1;
		if (my_service_write_int(h, offset + LU_PROTO_INDEX) != 0) goto my_service_search_error1;
		if (my_service_buffer_write(h, "</next>\n")          != 0) goto my_service_search_error1;
	}
	
	lu_search_end();
	if (my_service_buffer_write(h, "</search>\n") != 0) goto my_service_search_error0;
	
	return 0;

my_service_search_error1:
	lu_search_end();
my_service_search_error0:
	return -1;
}

static int my_service_splash(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	Lu_Config_List*		scan;
	char			key[40];
	
	if (strcmp(request, "index"))
	{
		my_service_error(h,
			"Request error",
			"You requested an invalid splash - must be index",
			request);
		return -1;
	}
	
	if (strcmp(ext, "html") && strcmp(ext, "xml"))
	{
		my_service_error(h,
			"Request error",
			"You requested an invalid splash extension - must be xml or html",
			ext);
		return -1;
	}
	
	if (my_service_buffer_init(h, "text/xml\n", 1, 
			2592000, LU_EXPIRY_ANY_LIST) != 0) return -1;
	if (my_service_xml_head(h)                   != 0) return -1;
	if (my_service_buffer_write(h, "<splash>\n") != 0) return -1;
	if (my_service_server(h)                     != 0) return -1;

	for (	scan = lu_config_list; 
		scan != lu_config_list + lu_config_lists; 
		scan++)
	{
		snprintf(&key[0], sizeof(key), "%s%d", LU_KEYWORD_LIST, scan->id);
		if (my_service_list(h, scan, 
			lu_breader_quick_records(&key[0])) != 0)
		{
			return -1;
		}
	}
	
	if (my_service_buffer_write(h, "</splash>\n") != 0) return -1;
	
	return 0;
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
	char*	s;
	int	off;
	ssize_t	got;
	int	lfs;
	char*	cwd = 0;
	char*	mod = 0;
	char*	qs  = 0;
	char*	ext = 0;
	char	buf[1024];
	
	struct My_Service_Handle_T h;
	h.fd = fd;
	
	h.buffer  = malloc(lu_config_cache_cutoff);
	h.bufsize = lu_config_cache_cutoff;
	h.init = 0;
	
	if (!h.buffer)
		return -1;
	
#ifdef DEBUG
	printf("Connect!\n");
#endif
	
	off = 0;
	lfs = 0;
	
	cwd = &buf[0];
	while (lfs < 4)
	{
		got = st_read(fd, 
			&buf[off], sizeof(buf) - off, 
			5000000); /* They have 5 seconds to talk */
		if (got <= 0)
		{	/* We're done - they closed link, timed out, or else
			 * something else went wrong.
			 */
			free(h.buffer);
			return -1;
		}
		
		for (s = &buf[off]; s != &buf[off+got]; s++)
		{
			if (*s == '\n') switch (lfs)
			{
			case 0:	*s = 0; lfs++; mod = s+1; break;
			case 1:	*s = 0; lfs++; qs  = s+1; break;
			case 2: *s = 0; lfs++; ext = s+1; break;
			default: *s = 0; lfs++; break;
			}
		}
		
		off += got;
	}

#ifdef DEBUG
	printf("Request: %s: %s . %s\n", mod, qs, ext);
#endif
	
	if      (!strcmp(mod, "message")) my_service_message(&h, qs, ext);
	else if (!strcmp(mod, "mbox"))    my_service_mbox   (&h, qs, ext);
	else if (!strcmp(mod, "attach"))  my_service_attach (&h, qs, ext);
	else if (!strcmp(mod, "mindex"))  my_service_mindex (&h, qs, ext);
	else if (!strcmp(mod, "search"))  my_service_search (&h, qs, ext);
	else if (!strcmp(mod, "splash"))  my_service_splash (&h, qs, ext);
	else if (!strcmp(mod, "thread"))  my_service_thread (&h, qs, ext);
	else
	{
		my_service_error(&h,
			"Request error",
			"You requested an invalid module",
			mod);
	}
	
	/* Make cwd into path */
	*(mod-1) = '/';
	*(qs-1)  = '/';
	*(ext-1) = '.';
	
	my_service_buffer_flush(&h);
	
	if (h.cache)
	{
#if DEBUG
		printf("Cache file: %s\n", cwd);
#endif
		
		if (lu_expiry_record_file(cwd, h.sent, h.ttl, h.list))
			unlink(cwd);
	}
	
	free(h.buffer);
	return 0;
}
