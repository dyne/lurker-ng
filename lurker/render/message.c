/*  $Id: message.c,v 1.8 2002-02-05 01:59:24 terpstra Exp $
 *  
 *  message.c - output results from a message/ lookup
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: GPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *           'Chris L. Bond' <cbond@twistedcircus.org>
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

#include "common.h"
#include "protocol.h"
#include "message.h"
#include "io.h"

#include "handler.h"

#include <string.h>
#include <ctype.h>

static int my_strcasecmp(
	const char* a, 
	const char* b)
{
	while (*a && *b)
	{
		int dif = tolower(*a++) - tolower(*b++);
		if (dif != 0) return dif;
	}
	
	if (*b) return 1;
	if (*a) return -1;
	
	return 0;
}

static void write_xml_escaped(
	const char *buf,
	size_t length,
	FILE *out)
{
	const char *end, *start;

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
			fwrite(start, 1, buf - start, out);
			fputs("&apos;", out);
			start = buf+1;
			break;
		case '<':
			fwrite(start, 1, buf - start, out);
			fputs("&lt;", out);
			start = buf+1;
			break;
		case '>':
			fwrite(start, 1, buf - start, out);
			fputs("&gt;", out);
			start = buf+1;
			break;
		case '"':
			fwrite(start, 1, buf - start, out);
			fputs("&quot;", out);
			start = buf+1;
			break;
		case '&':
			fwrite(start, 1, buf - start, out);
			fputs("&amp;", out);
			start = buf+1;
			break;
		case '\n':
			fwrite(start, 1, buf - start, out);
			fputs("<br/>\n", out);
			start = buf+1;
			break;
		}
	}
	
	fwrite(start, 1, buf - start, out);
}

static void write_xml_escaped_str(
	const char *buf,
	FILE *out)
{
	return write_xml_escaped(buf, strlen(buf), out);
}

static void message_traverse(
	struct msg* in, 
	struct mail_bodystruct* body,
	FILE* out)
{
	struct mail_body_part *p;
	size_t length;
	char *buffer;
	int nfree;
	
	if (!body)
	{	/* You never know... */
		return;
	}

	switch ((int)body->type)
	{
		case TYPEMESSAGE:
			/*
			 * This part contains an encapsulated message.
			 */
			if (my_strcasecmp(body->subtype, "rfc822"))
				break;

			message_traverse(in, body->nested.msg->body, out);
			break;

		case TYPETEXT:
			/*
			 * This is what we want to index -- stop.
			 */
			if (my_strcasecmp(body->subtype, "plain"))
				break;

			buffer = mail_select(in, body, &length, &nfree);
			write_xml_escaped(buffer, length, out);
			if (nfree)
				fs_give((void **)&buffer);

			break;

		case TYPEMULTIPART:
			/*
			 * Multipart message.  Look at the nested parts
			 * and hopefully find some plaintext.
			 */
			for (p = body->nested.part; p != NULL; p = p->next)
				message_traverse(in, &p->body, out);
			break;
		
		default:
			fprintf(out, "<attachment/>");
			break;
	}
}


FILE* lu_message_mbox(const char* parameter)
{
	char buf[240];
	int fd;
	FILE* out;
	
	snprintf(&buf[0], sizeof(buf), "%s.mbox", parameter);
	fd = open(&buf[0], 
		O_CREAT | O_RDWR | O_BINARY | O_EXCL,
		LU_S_READ | LU_S_WRITE);
	
	if (fd == -1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not able to create mbox file",
			&buf[0],
			strerror(errno));
		return 0;
	}
	
	if ((out = fdopen(fd, "w+")) == 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not able to fdopen an fd",
			fd,
			strerror(errno));
		
		close(fd);
	}
	
	return out;
}

static void write_addresses(ADDRESS* addr, const char* name, FILE* out)
{
	if (!addr) return;
	
	fprintf(out, " <%s>", name);
	for (; addr; addr = addr->next)
	{
		if (!addr->personal && (!addr->mailbox || !addr->host))
		{
			continue;
		}
		
		fprintf(out, "<email");
		
		if (addr->personal)
		{
			fputs(" name=\"", out);
			write_xml_escaped_str(addr->personal, out);
			fputs("\"", out);
		}
		
		if (addr->mailbox && addr->host)
		{
			fputs(" address=\"", out);
			write_xml_escaped_str(addr->mailbox, out);
			fputs("@", out);
			write_xml_escaped_str(addr->host,    out);
			fputs("\"", out);
		}
		
		fputs("/>", out);
	}
	
	fprintf(out, "</%s>\n", name);
}

int lu_message_handler(char* parameter)
{
	FILE*			mbox;
	FILE*			xml;
	Lu_Proto_Message	msg;
	time_t			tm;
	message_id		id;
	char			buf[4096];
	size_t			got;
	int			mbox_fd;
	struct msg*		email;
	
	fprintf(lu_server_link, "getmsg %s%c", parameter, LU_PROTO_ENDREQ);
	if (fread(&msg, sizeof(Lu_Proto_Message), 1, lu_server_link) != 1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Talking to server",
			"fread digest",
			strerror(errno));
		return -1;
	}
	
	if ((mbox = lu_message_mbox(parameter)) == 0)
		return -1;
	
	while ((got = fread(&buf[0], 1, sizeof(buf), lu_server_link)) > 0)
	{
		fwrite(&buf[0], 1, got, mbox);
	}
	
	mbox_fd = dup(fileno(mbox));
	fclose(mbox);
	
	if ((xml = lu_render_open(parameter)) == 0)
	{
		close(mbox_fd);
		return -1;
	}
	
	email = mail_parse(mbox_fd, 0);
	if (!email)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Parsing email",
			"mail_parse",
			strerror(errno));
		fclose(xml);
		close(mbox_fd);
		return -1;
	}
	
	fputs("<?xml-stylesheet type=\"text/xml\" href=\"render.xslt\"?>\n", xml);
	fputs("<message>\n", xml);
	
	fputs(" <id>", xml);
	fputs(parameter, xml);
	fputs("</id>\n", xml);
	
	fputs(" <list>\n  <name>", xml);
	write_xml_escaped_str(&msg.list_name[0], xml);
	fputs("</name>\n  <address>", xml);
	write_xml_escaped_str(&msg.list_address[0], xml);
	fputs("</address>\n  <desc>", xml);
	write_xml_escaped_str(&msg.list_desc[0], xml);
	fputs("</desc>\n </list>\n <mbox>", xml);
	write_xml_escaped_str(&msg.mbox_name[0], xml);
	fputs("</mbox>\n", xml);
	
	fprintf(xml, " <timestamp>%d</timestamp>\n", msg.timestamp);
	tm = msg.timestamp;
	fprintf(xml, " <time>%s</time>\n", ctime(&tm));
	fprintf(xml, " <thread>%d</thread>\n", msg.thread);
	
	if (msg.in_reply_to != lu_common_minvalid)
	{
		fprintf(xml, " <inreplyto>%d</inreplyto>\n", msg.in_reply_to);
	}
	
	if (msg.replys)
	{
		fprintf(xml, " <replys>\n");
		while (msg.replys)
		{
			if (fread(&id, sizeof(message_id), 1, lu_server_link) 
				!= 1)
			{
				break;
			}
			
			fprintf(xml, "  <id>%d</id>\n", id);
			msg.replys--;
		}
		fprintf(xml, " </replys>\n");
	}
	
	if (email->env->subject)
	{
		fputs(" <subject>", xml);
		write_xml_escaped(email->env->subject,
			strlen(email->env->subject), xml);
		fputs("</subject>\n", xml);
	}
	
	write_addresses(email->env->from,     "from",     xml);
	write_addresses(email->env->sender,   "sender",   xml);
	write_addresses(email->env->reply_to, "reply-to", xml);
	write_addresses(email->env->to,       "to",       xml);
	write_addresses(email->env->cc,       "cc",       xml);
	write_addresses(email->env->bcc,      "bcc",      xml);
	
	if (email->env->message_id)
	{
		fputs(" <message-id>", xml);
		write_xml_escaped_str(email->env->message_id, xml);
		fputs("</message-id>\n", xml);
	}
	
	fprintf(xml, " <body>");
	message_traverse(email, email->body, xml);
	
	fprintf(xml, "</body>\n");
	fprintf(xml, "</message>\n");
	
	close(mbox_fd);
	return lu_render_close(xml);
}
