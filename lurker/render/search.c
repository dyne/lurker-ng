/*  $Id: search.c,v 1.9 2002-03-04 04:03:55 terpstra Exp $
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
#include "prefix.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>

static void extract_keyword(
	const char* parameter, 
	char** w,
	char* e,
	const char* prefix,
	const char* field)
{
	const char* t;
	char* s;
	
	if ((s = strstr(parameter, field)) == 0)
		return;
	
	s += strlen(field); /* skip the field */
	if (!*s++) return;	/* skip the = */
	
	while (*w != e && *s && *s != '&')
	{
		while (*s)
		{
			if (*s == '+') { s++; continue; }
			if (*s == ' ') { s++; continue; }
			if (*s == '%' && *(s+1) == '2' && *(s+2) == '0')
			{
				s += 3;
				continue;
			}
			break;
		}
		
		for (t = "%20"; *w != e && *t; t++)
			*(*w)++ = *t;
		
		for (t = prefix; *w != e && *t; t++)
			*(*w)++ = *t;
		
		for (; *w != e && *s; s++)
		{
			if (*s == '+' || *s == ' ' || *s == '&' || 
			    (*s == '%' && *(s+1) == '2' && *(s+2) == '0'))
			{
				break;
			}
			
			*(*w)++ = *s;
		}
	}
}

int lu_search_handler(
	char* parameter, 
	const char* uri, 
	lu_doctype t)
{
	char		buf[4096];
	char*		w;
	char*		e;
	const char*	s;
	
	if (!memcmp(parameter, "bounce", 6))
	{	/* We were called like a real CGI - we need to redirect the
		 * user to a document that we can create
		 */
		 
		for (s = uri + strlen(uri); s != uri; s--)
		{
			if (*s == '/')
				break;
		}
		
		memcpy(&buf[0], uri, s - uri);
		buf[s - uri] = '/';
		buf[s - uri+1] = '0';
		buf[s - uri+2] = 0;
		
		s++;
		w = &buf[strlen(buf)];
		e = &buf[sizeof(buf)-1];
		
		extract_keyword(s, &w, e, LU_KEYWORD_SUBJECT, "subject");
		extract_keyword(s, &w, e, LU_KEYWORD_AUTHOR,  "author");
		extract_keyword(s, &w, e, LU_KEYWORD_WORD,    "query");
		
		extract_keyword(s, &w, e, LU_KEYWORD_WEEKDAY,      "weekday");
		extract_keyword(s, &w, e, LU_KEYWORD_DAY_OF_MONTH, "dom");
		extract_keyword(s, &w, e, LU_KEYWORD_MONTH,        "month");
		extract_keyword(s, &w, e, LU_KEYWORD_YEAR,         "year");
		extract_keyword(s, &w, e, LU_KEYWORD_LIST,         "list");
		
		/*!!! No idea how xml searching will work */
		for (s = ".html"; w != e && *s; s++)
			*w++ = *s;
		
		printf("Status: 303 Moved Permanently\r\n");
		printf("Location: %s\r\n", &buf[0]);
		printf("Content-type: text/html\r\n\r\n");
		printf(&redirect_error[0], &buf[0]);
		return -1;
	}
	
	if (t != LU_XML && t != LU_HTML)
	{	/* Can only handle wierd stuff in case above */
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], uri);
		return -1;
	}
	
	fprintf(lu_server_link, "%s%s%c", 
		LU_PROTO_SEARCH, parameter, LU_PROTO_ENDREQ);
		
	return lu_forward_xml(parameter);
}
