/*  $Id: mindex.c,v 1.6 2002-02-11 03:45:51 terpstra Exp $
 *  
 *  mindex.c - output results from a mindex/ lookup
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

#include "handler.h"
#include "common.h"
#include "protocol.h"

int lu_mindex_handler(
	char* parameter, 
	const char* uri, 
	lu_doctype t)
{
	FILE* xml;
	int i, j, fragment;
	
	char	buf[4096];
	size_t	got;
	size_t  get;
	
	if (t != LU_XML && t != LU_HTML)
	{	/* Don't know how to deal with other types */
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], uri);
		return -1;
	}
	
	if (sscanf(parameter, "%d@%d", &i, &j) != 2)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Reading parameters",
			parameter,
			"Not formatted as &lt;list&gt;@&lt;ffset&gt;");
		return -1;
	}
	
	fprintf(lu_server_link, "%s%d %d%c", 
		LU_PROTO_MINDEX, i, j, LU_PROTO_ENDREQ);
	fflush(lu_server_link);
	
	if ((xml = lu_render_open(parameter)) == 0)
		return -1;
	
	fragment = 0;
	while (1)
	{
		if (fragment == 0)
		{
			if (fscanf(lu_server_link, "%d", &fragment) != 1)
				break;
			if (fgetc(lu_server_link) != '\n')
				break;
			if (fragment == 0)
				break;
		}
		
		get = sizeof(buf);
		if (get > fragment) get = fragment;
		
		got = fread(&buf[0], 1, get, lu_server_link);
		fwrite(&buf[0], 1, got, xml);
		
		fragment -= got;
	}
	
	return lu_render_close(xml);
}
