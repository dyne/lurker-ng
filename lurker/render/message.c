/*  $Id: message.c,v 1.14 2002-02-22 01:55:58 terpstra Exp $
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
#include "handler.h"

#include "io.h"

#include <string.h>

int lu_message_handler(
	char* parameter, 
	const char* uri, 
	lu_doctype t)
{
	int i;
	
	if (t != LU_XML && t != LU_HTML)
	{	/* Don't know how to deal with other types */
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], uri);
		return -1;
	}
	
	if (sscanf(parameter, "%d", &i) != 1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Reading parameters",
			parameter,
			"Not formatted as &lt;message-id&gt;");
		return -1;
	}
	
	fprintf(lu_server_link, "%s %i%c", 
		LU_PROTO_GETMSG, i, LU_PROTO_ENDREQ);
	
	return lu_forward_xml(parameter);
}

int lu_mbox_handler(
	char* parameter, 
	const char* uri, 
	lu_doctype t)
{
	FILE* out;
	int   fd;
	int   i;
	
	if (t != LU_OTHER)
	{	/* Only know how to handle .txt */
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], uri);
		return -1;
	}
	
	if (sscanf(parameter, "%d.txt", &i) != 1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Reading parameters",
			parameter,
			"Not formatted as &lt;message-id&gt.txt;");
		return -1;
	}
	
	fd = open(parameter, O_CREAT | O_RDWR | O_BINARY | O_EXCL,
		LU_S_READ | LU_S_WRITE);
	if (fd == -1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not able to create mbox file",
			parameter,
			strerror(errno));
		return -1;
	}
	
	if ((out = fdopen(fd, "w")) == 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not able to fdopen an fd",
			parameter,
			strerror(errno));
		return -1;
	}
	
	fprintf(lu_server_link, "%s %i%c", 
		LU_PROTO_GETMBOX, i, LU_PROTO_ENDREQ);
	
	printf("Status: 200 OK\r\n");
	printf("Content-type: text/plain\r\n\r\n");
	
	lu_forward_data(out);
	
	/* We handled the request internally */
	return -1;
}
