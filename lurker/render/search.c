/*  $Id: search.c,v 1.3 2002-02-11 00:00:59 terpstra Exp $
 *  
 *  search.c - output results from a search/ lookup
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
#include "handler.h"
#include "protocol.h"

int lu_search_handler(char* parameter)
{
	FILE* xml;
	int fragment;
	
	char	buf[4096];
	size_t	got;
	size_t  get;
	
	fprintf(lu_server_link, "%s%s%c", 
		LU_PROTO_SEARCH, parameter, LU_PROTO_ENDREQ);
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
