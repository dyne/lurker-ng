/*  $Id: splash.c,v 1.2 2002-02-12 07:07:05 terpstra Exp $
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

int lu_splash_handler(
	char* parameter, 
	const char* uri, 
	lu_doctype t)
{
	if (t != LU_XML && t != LU_HTML)
	{	/* Don't know how to deal with other types */
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], uri);
		return -1;
	}
	
	if (strcmp(parameter, "index"))
	{
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], uri);
		return -1;
	}
	
	fprintf(lu_server_link, "%s%c", 
		LU_PROTO_LISTS, LU_PROTO_ENDREQ);
	
	return lu_forward_xml(parameter);
}
