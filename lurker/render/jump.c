/*  $Id: jump.c,v 1.5 2002-07-11 20:28:30 terpstra Exp $
 *  
 *  jump.c - redirect mindex jumps
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

#include "common.h"
#include "error.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

const char redirect_error[] = 
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html><head>\r\n"
"<title>301 Moved Permanently</title>\r\n"
"</head><body>\r\n"
"<h1>Moved Permanently</h1>\r\n"
"The document has moved <a href=\"%s/jump/%d@%ld.%s\">here</a>.\r\n"
"<p><hr>\r\n"
"</body></html>\r\n";

static int extract_int(
	const char* parameter, 
	const char* field)
{
	const char* s;
	
	s = parameter-1;
	do s = strstr(s+1, field);
	while (s && s != parameter && *(s-1) != '&');
	
	if (s == 0)
		return -1;
	
	/* skip the field */
	s += strlen(field); 
	
	/* pass the = */
	if (*s != '=') return -1;
	s++;
	
	return atol(s);
}

int main(int argc, char** argv)
{
	char* uri = getenv("REQUEST_URI");
	char* qs  = getenv("QUERY_STRING");
	char* w;
	const char* s;
	int   list;
	int   tmp;
	
	time_t		tmt;
	struct tm	tms;
	
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	bind_textdomain_codeset(PACKAGE, "utf-8");
	
	if (!uri)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0],
			_("Not invoked as a GET cgi"),
			_("REQUEST_URI invalid"),
			_("environment variable missing"));
		return 0;
	}
	
	if (!qs)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0],
			_("Not invoked as a GET cgi"),
			_("QUERY_STRING invalid"),
			_("environment variable missing"));
		return 0;
	}
	
	/* Prune off the first ? and then the last / -> get path */
	w = strchr(uri, '?');
	if (w) *w = 0;
	w = strrchr(uri, '/');
	if (w) *w = 0;
	
	/* Ok, uri is now the path relative to which the request goes */
	
	list = extract_int(qs, "list");
	
	tmt = time(0);
	memcpy(&tms, localtime(&tmt), sizeof(struct tm));
	
	tmp = extract_int(qs, "sec");  tms.tm_sec  = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "min");  tms.tm_min  = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "hour"); tms.tm_hour = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "mday"); tms.tm_mday = (tmp==-1)?1:tmp;
	tmp = extract_int(qs, "mon");  tms.tm_mon  = (tmp==-1)?0:tmp-1;
	tmp = extract_int(qs, "year"); if (tmp!=-1) tms.tm_year = tmp-1900;
	
	tmt = mktime(&tms);
	
	/* Find the format */
	w = qs-1;
	do w = strstr(w+1, "format=");
	while (w && w != qs && *(w-1) != '&');
	if (w)
	{
		w += strlen("format=");
		for (s = w; *w && *w != '&'; w++);
		*w = 0;
	}
	else	s = "xml";
	
	printf("Status: 303 Moved Permanently\r\n");
	printf("Location: %s/jump/%d@%ld.%s\r\n", uri, list, (long)tmt, s);
	printf("Content-type: text/%s\r\n\r\n", s);
	printf(&redirect_error[0], uri, list, (long)tmt, s);
	
	return 0;
}
