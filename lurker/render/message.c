/*  $Id: message.c,v 1.2 2002-02-04 01:37:21 terpstra Exp $
 *  
 *  message.c - output results from a message/ lookup
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

#include "common.h"
#include "protocol.h"

#include "handler.h"

#include <string.h>

int lu_message_handler(char* parameter)
{
	FILE*			f;
	Lu_Proto_Message	msg;
	time_t			tm;
	message_id		id;
	
	if ((f = lu_render_open(parameter)) == 0)
		return -1;
	
	fprintf(f, "<message>\n");
	
	fprintf(lu_server_link, "getmsg %s%c", parameter, LU_PROTO_ENDREQ);
	if (fread(&msg, sizeof(Lu_Proto_Message), 1, lu_server_link) != 1)
	{
		fprintf(f, " <error>%s</error>\n", strerror(errno));
		fprintf(f, "</message>\n");
		return lu_render_close(f);
	}
	
	fprintf(f, " <list>\n");
	fprintf(f, "  <name>%s</name>\n", &msg.list_name[0]);
	fprintf(f, "  <address>%s</address>\n", &msg.list_address[0]);
	fprintf(f, "  <desc>%s</desc>\n", &msg.list_desc[0]);
	fprintf(f, " </list>\n");
	
	fprintf(f, " <mbox>%s</mbox>\n", &msg.mbox_name[0]);
	
	fprintf(f, " <timestamp>%d</timestamp>\n", msg.timestamp);
	tm = msg.timestamp;
	fprintf(f, " <time>%s</time>\n", ctime(&tm));
	fprintf(f, " <thread>%d</thread>\n", msg.thread);
	
	if (msg.in_reply_to != lu_common_minvalid)
	{
		fprintf(f, " <inreplyto>%d</inreplyto>\n", msg.in_reply_to);
	}
	
	if (msg.replys)
	{
		fprintf(f, " <replys>\n");
		while (msg.replys)
		{
			if (fread(&id, sizeof(message_id), 1, lu_server_link) 
				!= 1)
			{
				break;
			}
			
			fprintf(f, "  <id>%d</id>\n", id);
			msg.replys--;
		}
		fprintf(f, " </replys>\n");
	}
	fprintf(f, " <body>");
	fprintf(f, "</body>\n");
	
	fprintf(f, "</message>\n");
	return lu_render_close(f);
}
