/*  $Id: service.c,v 1.93 2002-08-30 10:58:42 terpstra Exp $
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
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include "common.h"
#include "io.h"
#include "keyword.h"

#include "decode.h"
#include "config.h"
#include "summary.h"
#include "search.h"
#include "expiry.h"
#include "mbox.h"
#include "service.h"

#include <string.h>
#include <stdlib.h>
#include <iconv.h>

#ifdef DMALLOC
# include <dmalloc.h>
#endif

#define LU_PROTO_INDEX	20

#define PACK_STRATEGY_DEEP_FIRST
/* #define PACK_STRATEGY_SPACE_WASTE */

#define	EMPTY_CELL	"<a/>"
#define BAR_NS		"<b/>"
#define BAR_EW		"<c/>"
#define CORNER_SW	"<d/>"
#define TEE_WSE		"<e/>"

#define MESSAGE_END	'f'
#define MESSAGE_DOWN	'g'
#define MESSAGE_BOTH	'h'
#define TOPMESSAGE_END	'i'
#define TOPMESSAGE_DOWN	'j'
#define TOPMESSAGE_BOTH	'k'

extern void art_scan(const char** s, const char** e);
extern void url_scan(const char** s, const char** e);
extern void mailto_scan(const char** s, const char** e);
extern void quote_scan(const char** s, const char** e);

int lu_service_disable = 0;

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
	lu_decode_header(header, &buf[0], sizeof(buf), coding);
	
	/* Now, write it as an xml escaped string */
	return my_service_write_str(h, &buf[0]);
	
	return 0;
}

static int my_service_write_time(
	My_Service_Handle	h,
	lu_quad			t)
{
	char  buf[80];
	time_t tm = t;
	
	strftime(&buf[0], sizeof(buf), "%H:%M %b %d/%y", localtime(&tm));
	return my_service_write_str(h, &buf[0]);
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
	My_Service_Handle h,
	time_t ttl)
{
	time_t	expire;
	char 	timebuf[100];
	struct tm* tm;
	
	expire = time(0) + ttl;
	tm = gmtime(&expire);
	strftime(&timebuf[0], sizeof(timebuf),
		"%a, %d %b %Y %H:%M:%S GMT", tm);
	
	if (my_service_buffer_write(h, " <server>\n  <hostname>")       != 0) return -1;
	if (my_service_write_str   (h, lu_config_file->list_host)       != 0) return -1;
	if (my_service_buffer_write(h, "</hostname>\n  <email name=\"") != 0) return -1;
	if (my_service_write_str   (h, lu_config_file->admin_name)      != 0) return -1;
	if (my_service_buffer_write(h, "\" address=\"")                 != 0) return -1;
	if (my_service_write_str   (h, lu_config_file->admin_address)   != 0) return -1;
	if (my_service_buffer_write(h, "\"/>\n  <version>")             != 0) return -1;
	if (my_service_write_str   (h, PACKAGE_VERSION)                 != 0) return -1;
	if (my_service_buffer_write(h, "</version>\n  <expires>")       != 0) return -1;
	if (my_service_write_str   (h, &timebuf[0])                     != 0) return -1;
	if (my_service_buffer_write(h, "</expires>\n </server>\n")      != 0) return -1;
	
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
	if (my_service_server(h, 0)                             != 0) return -1;
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
			if (my_service_buffer_write(h, "&#64;"      ) != 0) return -1;
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

static int my_service_mailto(
	My_Service_Handle	h,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, mailto_scan(&sp, &ep), sp)
	{
		if (my_service_write_strl   (h, s, sp-s)     != 0) return -1;
		if (my_service_buffer_write (h, "<mailto>")  != 0) return -1;
		if (my_service_buffer_writel(h, sp, ep-sp)   != 0) return -1;
		if (my_service_buffer_write (h, "</mailto>") != 0) return -1;
		
		s = ep;
	}
	
	if (my_service_write_strl(h, s, e-s) != 0) return -1;
	return 0;
}

static int my_service_url(
	My_Service_Handle	h,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, url_scan(&sp, &ep), sp)
	{
		if (my_service_mailto      (h, s, sp-s)   != 0) return -1;
		if (my_service_buffer_write(h, "<url>")   != 0) return -1;
		if (my_service_write_strl  (h, sp, ep-sp) != 0) return -1;
		if (my_service_buffer_write(h, "</url>")  != 0) return -1;
		
		s = ep;
	}
	
	if (my_service_mailto(h, s, e-s) != 0) return -1;
	return 0;
}

static int my_service_pic(
	My_Service_Handle	h,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, art_scan(&sp, &ep), sp)
	{
		if (my_service_url         (h, s, sp-s)   != 0) return -1;
		if (my_service_buffer_write(h, "<art>")   != 0) return -1;
		if (my_service_url         (h, sp, ep-sp) != 0) return -1;
		if (my_service_buffer_write(h, "</art>")  != 0) return -1;
		
		s = ep;
	}
	
	if (my_service_url(h, s, e-s) != 0) return -1;
	return 0;
}

static int my_service_quote(
	My_Service_Handle	h,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, quote_scan(&sp, &ep), sp)
	{
		if (my_service_pic         (h, s, sp-s)    != 0) return -1;
		if (my_service_buffer_write(h, "<quote>")  != 0) return -1;
		if (my_service_pic         (h, sp, ep-sp)  != 0) return -1;
		if (my_service_buffer_write(h, "</quote>") != 0) return -1;
		
		s = ep;
	}
	
	if (my_service_pic(h, s, e-s) != 0) return -1;
	return 0;
}

static void my_service_kill_html(
	char*	buf)
{
	char* w = buf;
	
	while (*buf) switch (*buf)
	{
	case '&':
		if      (!strncmp(buf+1, "lt;", 3)) { *w++ = '<'; buf += 4; }
		else if (!strncmp(buf+1, "gt;", 3)) { *w++ = '>'; buf += 4; }
		else if (!strncmp(buf+1, "apos;",5)){ *w++ = '\'';buf += 6; }
		else if (!strncmp(buf+1, "nbsp;",5)){ *w++ = ' '; buf += 6; }
		else if (!strncmp(buf+1, "amp;",4)) { *w++ = '&'; buf += 5; }
		else if (!strncmp(buf+1, "quot;",5)){ *w++ = '"'; buf += 6; }
		else { *w++ = *buf++; }
		break;
	case '<':
		if ((buf[1] == 'b' || buf[1] == 'B') &&
		    (buf[2] == 'r' || buf[2] == 'R'))
		{
			*w++ = '\n';
		}
		
		/* Scan to end */
		while (*buf && *buf != '>') switch (*buf)
		{
		case '"':
			buf++;
			while (*buf && *buf != '"')
				if (*buf++ == '\\' && *buf) buf++;
			if (*buf) buf++;
			break;
		default:
			buf++;
		}
		if (*buf) buf++;
		
		break;
	case '\n':
		/* Ignore whitespace */
		buf++;
		break;
	default:
		*w++ = *buf++;
	}
	
	*w = 0;
}

static int my_service_dump(
	My_Service_Handle	h,
	const char*		dat,
	size_t			len,
	const char*		coding,
	int			html)
{
	char*	b;
	char	buf[10240];
	size_t	fill, tmp, got;
	iconv_t	ic;
	
	/* Make a guess at what the coding is supposed to be. */
	if (!coding) coding = "iso-8859-1";
	
	ic = iconv_open("utf-8", coding);
	if (ic == (iconv_t)-1)
	{
		/* Don't know this encoding - just dump the data anyways */
		syslog(LOG_WARNING, _("Unknown coding: %s\n"), coding);
		my_service_write_str(h, _("\n*** WARNING: UNKNOWN CHARSET '"));
		my_service_write_str(h, coding);
		my_service_write_str(h, _("' FALLING BACK TO ISO-8859-1 ***\n"));
		
		coding = "iso-8859-1";
		ic = iconv_open("utf-8", coding);
	}
	
	while (len)
	{
		fill = sizeof(buf)-1;
		b = &buf[0];
		tmp = iconv(ic, (ICONV_CONST char**)&dat, &len, &b, &fill);
		
		got = sizeof(buf)-1 - fill;
		if (html)
		{
			buf[got] = 0;
			my_service_kill_html(&buf[0]);
			got = strlen(&buf[0]);
		}
		
		/*!!! can miss stuff on 10k boundaries. oh well. */
		if (my_service_quote(h, &buf[0], got) != 0)
			break;
		
		if (tmp == (size_t)-1 && errno != E2BIG && errno != EAGAIN)
			break;
	}
	
	iconv_close(ic);
	
	if (len)
	{
		my_service_write_str(h, _("\n*** ERROR: BROKEN CHARSET '"));
		my_service_write_str(h, coding);
		my_service_write_str(h, _("' DURING DECODE ***\n"));
	}
	
	return 0;
}

static int my_service_traverse(
	My_Service_Handle	h,
	struct Lu_Mbox_Message* in, 
	struct mail_bodystruct* body,
	int			count,
	int			draw)
{
	struct mail_body_part*	p;
	size_t			length;
	char*			buffer;
	int			nfree;
	char*			name;
	const char*		charset;
	char*			lower;
	
	if (!body)
	{	/* You never know... */
		return count;
	}
	
	if (body->type > 8) body->type = 8;
	
	name    = my_service_find_name(body);
	charset = lu_mbox_find_charset(body);
	
	for (lower = body->subtype; *lower; lower++)
		if (*lower >= 'A' && *lower <= 'Z')
			*lower += 'a' - 'A';
	
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
	case TYPEMULTIPART:
		/*
		 * Multipart message.  Look at the nested parts
		 * and hopefully find some plaintext.
		 */
		
		p = body->nested.part;
		if (p)
		{
			count = my_service_traverse(h, in, 
				&p->body, count, 
				draw);
			p = p->next;
		}
		
		if (count == -1) break;
		
		for (; p != NULL; p = p->next)
		{
			count = my_service_traverse(h, in,
				&p->body, count, 
				draw && strcmp(body->subtype, "alternative"));
			if (count == -1) break;
		}
		
		break;
		
	case TYPEMESSAGE:
		/* This part contains an encapsulated message.
		 */
		count = my_service_traverse(h, in, 
				body->nested.msg->body, count, 
				draw);
		break;
	
	case TYPETEXT:
		/*
		 * This is what we want to display -- stop.
		 */
		
		if (draw)
		{
			buffer = lu_mbox_select_body(in, body, &length, &nfree);
			if (my_service_dump(h, buffer, length, charset,
				!strcmp(body->subtype, "html") ||
				!strcmp(body->subtype, "HTML")) != 0)
				count = -1;
			if (nfree) fs_give((void **)&buffer);
		}
		break;
		
	default:
		break;
	}
	
	if (count == -1) return -1;
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
	
	if (msg->thread == id)
	{
		if (my_service_buffer_write(h, "   <thread-head>yes</thread-head>\n") != 0) return -1;
	}
	
	if (lu_summary_write_variable(
		(int(*)(void*, const char*))        &my_service_buffer_write,
		(int(*)(void*, const char*, size_t))&my_service_write_strl,
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
	
	if (lu_summary_read_msummary(id, &msg) != 0)
		return -1;
	
	if (my_service_buffer_write(h, "  <summary>\n")  != 0) return -1;
	if (my_service_summary_body(h, id, &msg)         != 0) return -1;
	if (my_service_buffer_write(h, "  </summary>\n") != 0) return -1;
	
	return 0;
}

static int my_service_list(
	My_Service_Handle h,
	Lu_Config_List* l,
	message_id msgs,
	message_id offset,
	int next_prev)
{
	message_id	go, get;
	message_id	where[3];
	char		keyword[80];
	int		out;
	KRecord		kr;
	
	if (my_service_buffer_write(h, " <list>\n  <id>")       != 0) return -1;
	if (my_service_write_str(h, l->name)                    != 0) return -1;
	if (my_service_buffer_write(h, "</id>\n")               != 0) return -1;
	
	if (next_prev)
	{
		snprintf(&keyword[0], sizeof(keyword), "%s%s", LU_KEYWORD_LIST, l->name);
		out = kap_kopen(lu_config_keyword, &kr, &keyword[0]);
		if (out != 0)
		{
			return -1;
		}
		
		if (offset != 0)	go = offset - 1;
		else			go = offset;
		
		if (go + 3 > kr.records)get = kr.records - go;
		else			get = 3;
		
		out = kap_kread(lu_config_keyword, &kr, &keyword[0],
			go, &where[0], get);
		kap_kclose(lu_config_keyword, &kr, &keyword[0]);
		if (out != 0)
		{
			return -1;
		}
		
		if (offset != 0)
		{
			if (my_service_buffer_write(h, "  <prev>\n")  != 0) return -1;
			if (my_service_summary(h, where[0])           != 0) return -1;
			if (my_service_buffer_write(h, "  </prev>\n") != 0) return -1;
		}
		
		if (offset + 1 < kr.records)
		{
			if (my_service_buffer_write(h, "  <next>\n")  != 0) return -1;
			if (my_service_summary(h, where[offset?2:1])  != 0) return -1;
			if (my_service_buffer_write(h, "  </next>\n") != 0) return -1;
		}
		else
		{
			/*!!! depend on import to thread */
		}
	}
	
	if (msgs != lu_common_minvalid)
	{
		if (my_service_buffer_write(h, "  <messages>")  != 0) return -1;
		if (my_service_write_int(h, msgs)               != 0) return -1;
		if (my_service_buffer_write(h, "</messages>\n") != 0) return -1;
	}
	
	if (offset != lu_common_minvalid)
	{
		if (my_service_buffer_write(h, "  <offset>")    != 0) return -1;
		if (my_service_write_int(h, offset)             != 0) return -1;
		if (my_service_buffer_write(h, "</offset>\n")   != 0) return -1;
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

static int my_service_list_cb(
	void*		arg,
	lu_word		list,
	message_id	offset)
{
	Lu_Config_List* l = lu_config_find_listi(lu_config_file, list);
	
	if (l)
	{
		return my_service_list((My_Service_Handle)arg, l,
		                lu_common_minvalid, offset, 1);
	}
	
	return 0;
}

static int my_service_thread_load(
	My_Service_Handle	h,
	message_id		id,
	My_Service_Reply_Tree**	out_tree,
	int*			out_tree_size)
{
	char			key[20];
	My_Service_Reply_Tree*	tree;
	int			tree_size;
	int			i, out;
	KRecord			kr;
	
	snprintf(&key[0], sizeof(key), "%s%d", LU_KEYWORD_THREAD, id);
	out = kap_kopen(lu_config_keyword, &kr, &key[0]);
	if (out != 0)
	{
		my_service_error(h,
			_("Internal Server Error"),
			_("Lurkerd failed to open a KRecord for thread"),
			kap_strerror(out));
		
		goto my_service_thread_load_error0;
	}
	
	if (kr.records == 0)
	{
		my_service_error(h,
			_("Thread Not Found"),
			_("Lurkerd received a request for a non-existant thread"),
			&key[0]);
		
		goto my_service_thread_load_error1;
	}
	
	*out_tree_size = tree_size = kr.records;
	*out_tree = tree = malloc(tree_size * sizeof(My_Service_Reply_Tree));
	if (!tree)
	{
		my_service_error(h,
			_("Out of Memory"),
			_("Lurkerd ran out of memory processing this thread"),
			&key[0]);
		
		goto my_service_thread_load_error1;
	}
	
	for (i = 0; i < tree_size; i++)
	{
		if ((out = kap_kread(lu_config_keyword, &kr, &key[0],
			i, &tree[i].id, 1)) != 0)
		{
			my_service_error(h,
				_("Internal Server Error"),
				_("Lurkerd was unable to read from a krecord"),
				kap_strerror(out));
		
			goto my_service_thread_load_error2;
		}
		
		if (lu_summary_read_msummary(tree[i].id, &tree[i].summary) != 0)
		{
			my_service_error(h,
				_("Internal Server Error"),
				_("Lurkerd was unable to read a summary record"),
				&key[0]);
		
			goto my_service_thread_load_error2;
		}
	}
	
	kap_kclose(lu_config_keyword, &kr, &key[0]);
	return 0;

my_service_thread_load_error2:
	free(tree);
my_service_thread_load_error1:
	kap_kclose(lu_config_keyword, &kr, &key[0]);
my_service_thread_load_error0:
	*out_tree = 0;
	return -1;
}

static int my_service_tree_compare(const void* a, const void* b)
{
	message_id key = *(const message_id*)a;
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

static int my_service_tree_message_link(
	My_Service_Handle	h,
	My_Service_Reply_Tree*	tree,
	int 			i,
	int			hl)
{
	int  selected	= (hl == i);
	int  drift	= (tree[i].summary.replies != tree[i].replies);
	char buf[60];
	char x;
	
	if (tree[i].replyee == -1)
	{
		switch (tree[i].replies)
		{
		case 0:  x = TOPMESSAGE_END;  break;
		case 1:  x = TOPMESSAGE_DOWN; break;
		default: x = TOPMESSAGE_BOTH; break;
		}
	}
	else
	{
		switch (tree[i].replies)
		{
		case 0:  x = MESSAGE_END;  break;
		case 1:  x = MESSAGE_DOWN; break;
		default: x = MESSAGE_BOTH; break;
		}
	}
	
	if (hl == -1)
	{
		sprintf(&buf[0], "<%c%s/>", 
			x,
			drift?" drift=\"yes\"":"");
		return my_service_buffer_write(h, &buf[0]);
	}
	
	
	sprintf(&buf[0], "<%c%s%s><summary>\n", 
		x, 
		selected?" selected=\"yes\"":"",
		drift   ?" drift=\"yes\"":"");
		
	if (my_service_buffer_write(h, &buf[0]) != 0) return -1;
	
	if (my_service_summary_body(h, tree[i].id, &tree[i].summary) != 0)
		return -1;
	
	sprintf(&buf[0], "</summary></%c>", x);
	if (my_service_buffer_write(h, &buf[0]) != 0) return -1;
	
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
			_("Malformed request"),
			_("Lurkerd received a request for a non-textual mbox"),
			request);
		goto my_service_mbox_error0;
	}
	
	id = lu_summary_lookup_mid(request);
	if (id == lu_common_minvalid)
	{
		my_service_error(h,
			_("Message does not exist"),
			_("Lurkerd received a request for a non-existant message"),
			request);
		goto my_service_mbox_error0;
	}
	
	if (lu_summary_read_msummary(id, &msg) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd failed to retrieve the summary for the named message"),
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
	
	list = lu_config_find_listi(lu_config_file, list_id);
	if (!list)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd has a message which refers to a missing mailing list"),
			request);
		goto my_service_mbox_error0;
	}
	
	mbox = lu_config_find_mboxi(list, mbox_id);
	if (!mbox)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd has a message which refers to a missing mailbox"),
			request);
		goto my_service_mbox_error0;
	}
	
	if (lu_mbox_map_message(mbox, &cmsg, offset) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd was unable to mmap the message into memory"),
			request);
		goto my_service_mbox_error0;
	}
	
	base = cmsg.map.base;
	base += cmsg.headers - cmsg.map.off;
	len = cmsg.end - cmsg.headers;
	
	if (my_service_buffer_init(h, "text/plain\n", 1, 
			lu_config_file->cache_message_ttl, LU_EXPIRY_NO_LIST) != 0)
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
	
	mid = strchr(request, '@');
	if (!mid || sscanf(request, "%d@", &aid) != 1)
	{	/* There was nothing valid, or it did not end in whitespace
		 * or a null.
		 */
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received a request for a non-numeric attachment"),
			request);
		goto my_service_attach_error0;
	}
	mid++;
	
	id = lu_summary_lookup_mid(mid);
	if (id == lu_common_minvalid)
	{
		my_service_error(h,
			_("Message does not exist"),
			_("Lurkerd received a request for a non-existant message"),
			mid);
		goto my_service_attach_error0;
	}
	
	if (lu_summary_read_msummary(id, &msg) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd failed to retrieve the summary for the named message"),
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
	
	list = lu_config_find_listi(lu_config_file, list_id);
	if (!list)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd has a message which refers to a missing mailing list"),
			request);
		goto my_service_attach_error0;
	}
	
	mbox = lu_config_find_mboxi(list, mbox_id);
	if (!mbox)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd has a message which refers to a missing mailbox"),
			request);
		goto my_service_attach_error0;
	}
	
	if (lu_mbox_map_message(mbox, &cmsg, offset) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd was unable to mmap the message into memory"),
			request);
		goto my_service_attach_error0;
	}
	
	if (lu_mbox_parse_message(&cmsg, &mmsg) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd was unable to parse the message"),
			request);
		goto my_service_attach_error1;
	}
	
	attach = my_service_attach_id(&mmsg, mmsg.body, &aid);
	
	if (!attach)
	{
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received a request for a non-existant attachment"),
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

/* return what we use up to */
static int my_service_draw_snippet(
	My_Service_Reply_Tree* tree, 
	int p, 
	int row)
{
	int col;
	int c;
	
	col = tree[p].column = tree[p].consumed;
	
	if (row < 3)
	{
		for (c = tree[p].replyor_first; c != -1; c = tree[c].replyor_next)
		{
			tree[c].consumed = col;
			col = my_service_draw_snippet(tree, c, row+1);
		}
	}
	
	if (tree[p].replyor_first == -1 || row >= 3) col++;
	
	return col;
}
static void my_service_draw_snippet_row(
	My_Service_Handle	h,
	My_Service_Reply_Tree*	tree, 
	int* draw_head, 
	int row,
	int hl)
{
	int	p;
	int	c;
	int	col = 0;

	/* First, draw the current row */
	for (p = *draw_head; p != -1; p = tree[p].draw_next)
	{
		for (; col < tree[p].column; col++)
			my_service_buffer_write(h, EMPTY_CELL);
		
		
		my_service_tree_message_link(h, tree, p, hl);
		col++;
		
		/* Now, inject our children in place.
		 * Also, draw the stuff down to them.
		 */
		for (c = tree[p].replyor_first; c != -1; c = tree[c].replyor_next)
		{
			*draw_head = c;
			draw_head = &tree[c].draw_next;
			
			if (c == tree[p].replyor_first)
				continue;
			
			for (; col < tree[c].column; col++)
				my_service_buffer_write(h, BAR_EW);
			
			if (tree[c].replyor_next == -1)
				my_service_buffer_write(h, CORNER_SW);
			else
				my_service_buffer_write(h, TEE_WSE);
			col++;
		}
	}
	
	/* Terminate the list */
	*draw_head = -1;
}

static int my_service_message(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	time_t		ttl = lu_config_file->cache_message_ttl;
	
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
	
	KRecord			kr;
	char			keyword[LU_KEYWORD_LEN+1];
	const char*		coding;
	
	message_id		count, ind, get, i, buf[20];
	
	My_Service_Reply_Tree*	tree = 0;
	int			tree_size, p, n, j, head, cols, out;
	
	if (strcmp(ext, "xml") && strcmp(ext, "html"))
	{
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received a message request for non html/xml"),
			ext);
		
		goto my_service_message_error0;
	}
	
	id = lu_summary_lookup_mid(request);
	if (id == lu_common_minvalid)
	{
		my_service_error(h,
			_("Message does not exist"),
			_("Lurkerd received a request for a non-existant message"),
			request);
		goto my_service_message_error0;
	}
	
	if (lu_summary_read_msummary(id, &msg) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd failed to retrieve the summary for the named message"),
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
	
	list = lu_config_find_listi(lu_config_file, list_id);
	if (!list)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd has a message which refers to a missing mailing list"),
			request);
		goto my_service_message_error0;
	}
	
	mbox = lu_config_find_mboxi(list, mbox_id);
	if (!mbox)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd has a message which refers to a missing mailbox"),
			request);
		goto my_service_message_error0;
	}
	
	if (lu_mbox_map_message(mbox, &cmsg, offset) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd was unable to mmap the message into memory"),
			request);
		goto my_service_message_error0;
	}
	
	if (lu_mbox_parse_message(&cmsg, &mmsg) != 0)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd was unable to parse the map'd message"),
			request);
		goto my_service_message_error1;
	}
	
	if (mmsg.env->message_id)
	{
		snprintf(&keyword[0], sizeof(keyword), "%s%s",
			LU_KEYWORD_REPLY_TO,
			lu_decode_id(mmsg.env->message_id));
		
		out = kap_kopen(lu_config_keyword, &kr, &keyword[0]);
		if (out != 0)
		{
			my_service_error(h,
				_("Internal Error"),
				_("Lurkerd was unable to access the reply-to keyword"),
				kap_strerror(out));
			goto my_service_message_error2;
		}
	}
	else
	{
		out = -1;
	}
	
	coding = lu_mbox_find_charset(mmsg.body);
	
	if (my_service_thread_load(h, msg.thread, &tree, &tree_size) != 0)
	{
		goto my_service_message_error3;
	}
	
	my_service_init_tree(tree, tree_size);
	for (n = 0; n < tree_size; n++) if (tree[n].id == id) break;
	
	if (n == tree_size)
	{
		my_service_error(h,
			_("Internal Error"),
			_("Lurkerd cannot find a message in its own thread"),
			request);
		goto my_service_message_error3;
	}
	
	if (tree[n].replyor_first == -1 && tree[n].replyee != -1)
		p = tree[n].replyee; 
	else	p = n;
	
	if (tree[p].replyee != -1) p = tree[p].replyee;
	
	cols = my_service_draw_snippet(tree, p, 0);
	
	if (my_service_buffer_init(h, "text/xml\n", 1, ttl, LU_EXPIRY_NO_LIST) != 0)
		goto my_service_message_error3;
	
	if (my_service_xml_head(h)                       != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, "<message>\n")    != 0) goto my_service_message_error3;
	if (my_service_server(h, ttl)                    != 0) goto my_service_message_error3;
	
	if (my_service_summary_body(h, id, &msg)               != 0) goto my_service_message_error3;
	if (lu_summary_write_lists(&my_service_list_cb, h, id) != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, " <threading>\n")       != 0) goto my_service_message_error3;
	
	if (n != 0)
	{
		if (my_service_buffer_write(h, "  <prev><summary>\n")   != 0) goto my_service_message_error3;
		if (my_service_summary_body(h,
		                      tree[n-1].id, &tree[n-1].summary) != 0) goto my_service_message_error3;
		if (my_service_buffer_write(h, "  </summary></prev>\n") != 0) goto my_service_message_error3;
	}
	if (n+1 < tree_size)
	{
		if (my_service_buffer_write(h, "  <next><summary>\n")   != 0) goto my_service_message_error3;
		if (my_service_summary_body(h,
		                      tree[n+1].id, &tree[n+1].summary) != 0) goto my_service_message_error3;
		if (my_service_buffer_write(h, "  </summary></next>\n") != 0) goto my_service_message_error3;
	}
	
	if (msg.in_reply_to != lu_common_minvalid)
	{
		if (my_service_buffer_write(h, "  <inreplyto>\n")  != 0) goto my_service_message_error3;
		if (my_service_summary(h, msg.in_reply_to)         != 0) goto my_service_message_error3;
		if (my_service_buffer_write(h, "  </inreplyto>\n") != 0) goto my_service_message_error3;
	}
	
	if (out == 0 && (count = kr.records) != 0)
	{
 		ind = 0;
 		
		if (my_service_buffer_write(h, "  <replies>\n") != 0) goto my_service_message_error3;
		
 		while (count)
 		{
 			get = count;
 			if (get > sizeof(buf)/sizeof(message_id))
 				get = sizeof(buf)/sizeof(message_id);
 			
 			if (kap_kread(lu_config_keyword, &kr, &keyword[0],
 				ind, &buf[0], get) != 0)
 				goto my_service_message_error3;
 			
 			for (i = 0; i < get; i++)
 			{
 				if (my_service_summary(h, buf[i]) != 0)
 					goto my_service_message_error3;
 			}
 			
 			ind   += get;
 			count -= get;
 		}
 		
		if (my_service_buffer_write(h, "  </replies>\n") != 0) goto my_service_message_error3;
 	}
	
	if (my_service_buffer_write(h, "  <snippet cols=\"") != 0) goto my_service_message_error3;
	if (my_service_write_int   (h, cols)                 != 0) goto my_service_message_error3;
	if (my_service_buffer_write(h, " \">\n")             != 0) goto my_service_message_error3;
	
	head = p;
	for (j = 0; j < 3; j++)
	{
		my_service_buffer_write(h, "   ");
		my_service_draw_snippet_row(h, tree, &head, j, n);
		my_service_buffer_write(h, "<br/>\n");
	}
	if (my_service_buffer_write(h, "  </snippet>\n")       != 0) goto my_service_message_error3;
	
	if (my_service_buffer_write(h, " </threading>\n")      != 0) goto my_service_message_error3;
	
	if (my_service_addresses(h, mmsg.env->from,     "from",     coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->sender,   "sender",   coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->reply_to, "reply-to", coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->to,       "to",       coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->cc,       "cc",       coding) != 0) goto my_service_message_error3;
	if (my_service_addresses(h, mmsg.env->bcc,      "bcc",      coding) != 0) goto my_service_message_error3;
	
	if (my_service_traverse(h, &mmsg, mmsg.body, 0, 1) == -1) goto my_service_message_error3;
	if (my_service_buffer_write(h, "</message>\n")  != 0) goto my_service_message_error3;
	
	if (out == 0) kap_kclose(lu_config_keyword, &kr, &keyword[0]);
	free(tree);
	lu_mbox_destroy_message(&mmsg);
	lu_mbox_destroy_map(&cmsg);
	return 0;
	
my_service_message_error3:
	if (out == 0) kap_kclose(lu_config_keyword, &kr, &keyword[0]);
	if (tree != 0) free(tree);
my_service_message_error2:
	lu_mbox_destroy_message(&mmsg);
my_service_message_error1:
	lu_mbox_destroy_map(&cmsg);
my_service_message_error0:
	return -1;
}

static int my_service_jump(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	time_t			ttl = lu_config_file->cache_index_ttl;
	
	lu_quad			tm;
	int			ln;
	Lu_Config_List*		list;
	
	char			keyword[40];
	KRecord			kr;
	int			out;
	
	message_id		jump;
	ssize_t			off;
	message_id		ljump;
	
	if (sscanf(request, "%d@%n", &tm, &ln) < 1)
	{	/* They did something funny. */
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received an improperly formatted jump request"),
			request);
		
		goto my_service_jump_error0;
	}
	
	if (strcmp(ext, "xml") && strcmp(ext, "html"))
	{
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received a jump request for non html/xml"),
			ext);
		
		goto my_service_jump_error0;
	}
	
	list = lu_config_find_listn(lu_config_file, request+ln);
	if (!list)
	{
		my_service_error(h,
			_("Internal error"),
			_("The specified mailing list does not exist"),
			request+ln);
		
		goto my_service_jump_error0;
	}
	
	/* Ok, every prior to here is prior to tm. */
	jump = lu_summary_find_timestamp(tm);
	
	/* Open a krecord */
	sprintf(&keyword[0], "%s%s", LU_KEYWORD_LIST, list->name);
	out = kap_kopen(lu_config_keyword, &kr, &keyword[0]);
	if (out != 0)
	{
		my_service_error(h,
			_("Internal error"),
			_("Lurkerd failed to access the keyword database"),
			kap_strerror(out));
		
		goto my_service_jump_error0;
	}
	
	out = kap_find(
		lu_config_keyword,
		&kr,
		&lu_search_find_le,
		&jump,
		&off,
		&ljump);
	if (out == KAP_NOT_FOUND)
	{
		off = 0;
	}
	else if (out != 0)
	{
		my_service_error(h,
			_("Internal error"),
			_("Lurkerd failed to jump to a record"),
			kap_strerror(out));
		
		goto my_service_jump_error1;
	}
	
	/* Normalize the offset */
	off -= (off % LU_PROTO_INDEX);
	
	if (my_service_buffer_init(h, "text/xml\n", 1, ttl,
		(off+LU_PROTO_INDEX >= kr.records)?list->key:LU_EXPIRY_NO_LIST) != 0)
	{
		goto my_service_jump_error1;
	}
	
	if (my_service_xml_head    (h)                 != 0) goto my_service_jump_error1;
	if (my_service_buffer_write(h, "<redirect>\n") != 0) goto my_service_jump_error1;
	if (my_service_server      (h, ttl)            != 0) goto my_service_jump_error1;
	if (my_service_buffer_write(h, " <url>mindex/")!= 0) goto my_service_jump_error1;
	if (my_service_write_int   (h, off)            != 0) goto my_service_jump_error1;
	if (my_service_buffer_write(h, "@")            != 0) goto my_service_jump_error1;
	if (my_service_write_str   (h, list->name)     != 0) goto my_service_jump_error1;
	if (my_service_buffer_write(h, "</url>\n")     != 0) goto my_service_jump_error1;
	if (my_service_buffer_write(h, "</redirect>\n")!= 0) goto my_service_jump_error1;
	
	kap_kclose(lu_config_keyword, &kr, &keyword[0]);
	return 0;
	
my_service_jump_error1:
	kap_kclose(lu_config_keyword, &kr, &keyword[0]);
	
my_service_jump_error0:
	return -1;
}

static int my_service_mindex(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	time_t			ttl;
	lu_word			ttl_list;
	
	KRecord			kr;
	int			list;
	Lu_Config_List*		l;
	message_id		offset;
	char			keyword[80];
	message_id		ids[LU_PROTO_INDEX];
	int			i, out;
	message_id		count;
	Lu_Summary_Message	msg;
	time_t			tt;
	struct tm*		tm;
	
	if (sscanf(request, "%d@%n", &offset, &list) < 1)
	{	/* They did something funny. */
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received an improperly formatted mindex request"),
			request);
		
		goto my_service_mindex_error0;
	}
	
	if (strcmp(ext, "xml") && strcmp(ext, "html"))
	{
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received an index request for non html/xml"),
			ext);
		
		goto my_service_mindex_error0;
	}
	
	if ((offset % LU_PROTO_INDEX) != 0)
	{	/* Must be a multiple of the index */
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received an unaligned mindex request"),
			request);
		
		goto my_service_mindex_error0;
	}
	
	l = lu_config_find_listn(lu_config_file, request+list);
	if (l == 0)
	{
		my_service_error(h,
			_("Invalid request"),
			_("Lurkerd received a request for a non-existant list"),
			request);
		
		goto my_service_mindex_error0;
	}
	
	sprintf(&keyword[0], "%s%s", LU_KEYWORD_LIST, l->name);
	out = kap_kopen(lu_config_keyword, &kr, &keyword[0]);
	if (out != 0)
	{
		my_service_error(h,
			_("Internal error"),
			_("Lurkerd failed to access the keyword database"),
			kap_strerror(out));
		
		goto my_service_mindex_error0;
	}
	
	if (offset >= kr.records)
	{
		my_service_error(h,
			_("Invalid request"),
			_("Lurkerd received a mindex request past the end of the list"),
			request);
		
		goto my_service_mindex_error1;
	}
	
	count = kr.records - offset;
	if (count > LU_PROTO_INDEX)
		count = LU_PROTO_INDEX;
	
	if ((out = kap_kread(lu_config_keyword, &kr, &keyword[0],
		offset, &ids[0], count)) != 0)
	{
		my_service_error(h,
			_("Internal error"),
			_("Lurkerd failed to retrieve the records from the keyword file"),
			kap_strerror(out));
		
		goto my_service_mindex_error1;
	}
	
	if (lu_summary_read_msummary(ids[0], &msg) != 0)
	{
		my_service_error(h,
			_("Internal error"),
			_("Lurkerd failed to retrieve the first summary record for the file"),
			kap_strerror(out));
		
		goto my_service_mindex_error1;
	}
	
	if (offset + count < kr.records)
	{
		ttl_list = LU_EXPIRY_NO_LIST;
		ttl = lu_config_file->cache_index_ttl;
	}
	else
	{
		ttl_list = l->key; /* Expire the static html on import */
		ttl = 300; /* Only have the client browser cache for 5 min */
	}
		
	/* We shouldn't change if we already have next link */
	if (my_service_buffer_init(h, "text/xml\n", 1, ttl, ttl_list) != 0)
	{
		goto my_service_mindex_error1;
	}
	
	if (my_service_xml_head(h)                            != 0) goto my_service_mindex_error1;
	if (my_service_buffer_write(h, "<mindex>\n <offset>") != 0) goto my_service_mindex_error1;
	if (my_service_write_int(h, offset)                   != 0) goto my_service_mindex_error1;
	if (my_service_buffer_write(h, "</offset>\n")         != 0) goto my_service_mindex_error1;
	
	if (offset + count != kr.records)
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
	
	if (my_service_server(h, ttl) != 0) goto my_service_mindex_error1;
	
	if (my_service_list(h, l, kr.records, offset, 0) != 0) goto my_service_mindex_error1;
	kap_kclose(lu_config_keyword, &kr, &keyword[0]);
	
	tt = msg.timestamp;
	tm = localtime(&tt);
	
	if (my_service_buffer_write(h, " <time>\n")      != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "  <year>")       != 0) goto my_service_mindex_error0;
	if (my_service_write_int   (h, tm->tm_year+1900) != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "</year>\n")      != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "  <mon>")        != 0) goto my_service_mindex_error0;
	if (my_service_write_int   (h, tm->tm_mon+1)     != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "</mon>\n")       != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "  <mday>")       != 0) goto my_service_mindex_error0;
	if (my_service_write_int   (h, tm->tm_mday)      != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "</mday>\n")      != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "  <hour>")       != 0) goto my_service_mindex_error0;
	if (my_service_write_int   (h, tm->tm_hour)      != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "</hour>\n")      != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "  <min>")        != 0) goto my_service_mindex_error0;
	if (my_service_write_int   (h, tm->tm_min)       != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, "</min>\n")       != 0) goto my_service_mindex_error0;
	if (my_service_buffer_write(h, " </time>\n")     != 0) goto my_service_mindex_error0;
	
	for (i = 0; i < count; i++)
	{
		if (my_service_summary(h, ids[i]) != 0) goto my_service_mindex_error0;
	}
	
	if (my_service_buffer_write(h, "</mindex>\n") != 0) goto my_service_mindex_error0;
	
	return 0;
	
my_service_mindex_error1:
	kap_kclose(lu_config_keyword, &kr, &keyword[0]);
	
my_service_mindex_error0:
	return -1;
}

#ifdef PACK_STRATEGY_DEEP_FIRST
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
#endif

#ifdef PACK_STRATEGY_SPACE_WASTE
/* return space wasted */
static int my_service_draw_tree(
	My_Service_Reply_Tree* tree, 
	int n, int u)
{
}
#endif


static void my_service_draw_tree_row(
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
	
	for (; col < tree[i].column; col++)
		my_service_buffer_write(h, EMPTY_CELL);
	
	my_service_tree_message_link(h, tree, i, -1);
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
	time_t			ttl = lu_config_file->cache_message_ttl;
	
	My_Service_Reply_Tree*	tree;
	int			tree_size;
	int			draw_head;
	int			i;
	message_id		id;
	
	if (strcmp(ext, "xml") && strcmp(ext, "html"))
	{
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received a thread request for non html/xml"),
			ext);
		
		goto my_service_thread_error0;
	}
	
	if (sscanf(request, "%d", &id) != 1)
	{	/* They did something funny. */
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received an improperly formatted thread request"),
			request);
		
		goto my_service_thread_error0;
	}
	
	if (my_service_thread_load(h, id, &tree, &tree_size) != 0)
	{
		goto my_service_thread_error0;
	}
	
	my_service_init_tree(tree, tree_size);
	for (i = 0; i < tree_size; i++)
	{
		if (tree[i].replyee == -1)
			my_service_draw_tree(tree, i, i);
	}
	
	if (my_service_buffer_init(h, "text/xml\n", 1, ttl, LU_EXPIRY_NO_LIST) != 0)
		goto my_service_thread_error1;
	
	if (my_service_xml_head(h)                        != 0) goto my_service_thread_error1;
	if (my_service_buffer_write(h, "<thread>\n <id>") != 0) goto my_service_thread_error1;
	if (my_service_write_int(h, id)                   != 0) goto my_service_thread_error1;
	if (my_service_buffer_write(h, "</id>\n")         != 0) goto my_service_thread_error1;
	if (my_service_server(h, ttl)                     != 0) goto my_service_thread_error1;
	
	draw_head = -1;
	
	for (i = 0; i < tree_size; i++)
	{
		my_service_buffer_write(h, " <summary>\n") ;
		my_service_summary_body(h, tree[i].id, &tree[i].summary);
		my_service_buffer_write(h, "   <draw>") ;
		my_service_draw_tree_row(h, tree, &draw_head, i);
		my_service_buffer_write(h, "</draw>\n </summary>\n") ;
	}
	
	if (my_service_buffer_write(h, "</thread>\n") != 0) goto my_service_thread_error1;
	
	free(tree);
	return 0;
	
my_service_thread_error1:
	free(tree);
my_service_thread_error0:
	return -1;
}

static int my_service_search(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	time_t		ttl = lu_config_file->cache_search_ttl;
	
	int		i;
	message_id	out;
	message_id	offset;
	message_id	predict;
	const char*	delim;
	const char*	detail;
	char*		demux;
	long		off, start, end;
	int		pull;
	
	start = end = -1;
	pull = 0;
	sscanf(request, "%ld %ld %ld %n", &off, &start, &end, &pull);
	delim = request+pull;
	
	if (end == -1 || pull == 0)
	{
		my_service_error(h,
			_("Invalid request"),
			_("Lurkerd received a search request with wrong args"),
			request);
		goto my_service_search_error0;
	}
	
	offset = off;
	if ((offset % LU_PROTO_INDEX) != 0)
	{	/* Must be a multiple of the index */
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received an unaligned search request"),
			request);
		
		goto my_service_search_error0;
	}
	
	if (offset > 1000)
	{
		my_service_error(h,
			_("Malformed request"),
			_("Lurkerd received a search request for too large an offset"),
			request);
		goto my_service_search_error0;
	}
	
	for (demux = (char*)delim; *demux; demux++)
		if (*demux == 1) *demux = '/';
	
	if (lu_search_start(delim, &detail, start, end) != 0)
	{
		my_service_error(h,
			_("Search failed"),
			_("The lurkerd server rejected this search"),
			detail);
		goto my_service_search_error0;
	}
	
	for (i = 0; i < offset; i++)
	{
		if (lu_search_result(&out) != 0)
		{
			my_service_error(h,
				_("Internal error"),
				_("The lurkerd server failed to search"),
				_("lu_search_result failure - see logs"));
			goto my_service_search_error1;
		}
		
		if (out == lu_common_minvalid)
		{
			my_service_error(h,
				_("Search failed"),
				_("The lurkerd server rejected this search"),
				_("There are not that many search results"));
			goto my_service_search_error1;
		}
	}
	
	if (my_service_buffer_init(h, "text/xml\n", 1, ttl, LU_EXPIRY_NO_LIST) != 0)
		goto my_service_search_error1;
	
	/* Ok! Now, lets start putting out the data */
	if (my_service_xml_head(h)                             != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "<search>\n <offset>")  != 0) goto my_service_search_error1;
	if (my_service_write_int(h, offset)                    != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "</offset>\n <query>")  != 0) goto my_service_search_error1;
	if (my_service_write_str(h, delim)                     != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "</query>\n <queryurl>")!= 0) goto my_service_search_error1;
	if (my_service_write_int(h, start)                     != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "%20")                  != 0) goto my_service_search_error1;
	if (my_service_write_int(h, end)                       != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "%20")                  != 0) goto my_service_search_error1;
	
 	for (demux = (char*)delim; *demux; demux++)
		if (*demux == '/') *demux = 1;
		
	if (my_service_write_str(h, delim)              != 0) goto my_service_search_error1;
	if (my_service_buffer_write(h, "</queryurl>\n") != 0) goto my_service_search_error1;
	if (my_service_server(h, ttl)                   != 0) goto my_service_search_error1;
	
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
	
	lu_search_end(&predict);
	
	if (my_service_buffer_write(h, " <hits>")     != 0) goto my_service_search_error0;
	if (my_service_write_int   (h, predict)       != 0) goto my_service_search_error0;
	if (my_service_buffer_write(h, "</hits>\n")   != 0) goto my_service_search_error0;
	if (my_service_buffer_write(h, "</search>\n") != 0) goto my_service_search_error0;
	
	return 0;

my_service_search_error1:
	lu_search_end(&predict);
my_service_search_error0:
	return -1;
}

static int my_service_splash(
	My_Service_Handle h, 
	const char* request,
	const char* ext)
{
	time_t			ttl = 2592000; /* why hard-coded 30 days? */
	time_t			ttlc = 300; /* why hard-coded 5 minutes? */
	
	Lu_Config_List*		scan;
	char			key[40];
	size_t			records;
	int			out;
	
	if (strcmp(request, "index"))
	{
		my_service_error(h,
			_("Request error"),
			_("You requested an invalid splash - must be index"),
			request);
		return -1;
	}
	
	if (strcmp(ext, "html") && strcmp(ext, "xml"))
	{
		my_service_error(h,
			_("Request error"),
			_("You requested an invalid splash extension - must be xml or html"),
			ext);
		return -1;
	}
	
	if (my_service_buffer_init(h, "text/xml\n", 1, ttl, LU_EXPIRY_ANY_LIST) != 0) return -1;
	if (my_service_xml_head(h)                   != 0) return -1;
	if (my_service_buffer_write(h, "<splash>\n") != 0) return -1;
	if (my_service_server(h, ttlc)               != 0) return -1;

	for (	scan = lu_config_file->list; 
		scan != lu_config_file->list + lu_config_file->lists; 
		scan++)
	{
		snprintf(&key[0], sizeof(key), "%s%s", LU_KEYWORD_LIST, scan->name);
		
		out = kap_krecords(lu_config_keyword, &records, &key[0]);
		if (out != 0) return -1;
		
		if (my_service_list(
			h, 
			scan, 
			records, 
			records
			?((records - 1) / LU_PROTO_INDEX * LU_PROTO_INDEX)
			:0,
			0) != 0)
		{
			return -1;
		}
	}
	
	if (my_service_buffer_write(h, "</splash>\n") != 0) return -1;
	
	return 0;
}

/*------------------------------------------------- Public component methods */

int lu_service_init(void)
{
	return 0;
}

int lu_service_open(void)
{
	return 0;
}

int lu_service_sync(void)
{
	return 0;
}

int lu_service_close(void)
{
	return 0;
}

int lu_service_quit(void)
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
	
	h.buffer  = malloc(lu_config_file->cache_cutoff);
	h.bufsize = lu_config_file->cache_cutoff;
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
	if (lu_service_disable)
	{
		my_service_error(&h,
			_("Service disabled"),
			_("The lurker server has disabled access at this time"),
			qs);
	}
	else if (!strcmp(mod, "message")) my_service_message(&h, qs, ext);
	else if (!strcmp(mod, "mbox"))    my_service_mbox   (&h, qs, ext);
	else if (!strcmp(mod, "attach"))  my_service_attach (&h, qs, ext);
	else if (!strcmp(mod, "mindex"))  my_service_mindex (&h, qs, ext);
	else if (!strcmp(mod, "search"))  my_service_search (&h, qs, ext);
	else if (!strcmp(mod, "splash"))  my_service_splash (&h, qs, ext);
	else if (!strcmp(mod, "thread"))  my_service_thread (&h, qs, ext);
	else if (!strcmp(mod, "jump"))    my_service_jump   (&h, qs, ext);
	else
	{
		my_service_error(&h,
			_("Request error"),
			_("You requested an invalid module"),
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
