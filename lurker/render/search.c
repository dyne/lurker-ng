/*  $Id: search.c,v 1.19 2002-07-12 21:28:33 terpstra Exp $
 *  
 *  search.c - redirect search postings
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

#include "keyword.h"
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
"The document has moved <a href=\"%s/search/0%%20%ld%%20%ld%s.%s\">here</a>.\r\n"
"<p><hr>\r\n"
"</body></html>\r\n";

char* eos;
static int pump_keyword(
	const char*	keyword,
	void*		arg)
{
	char**       w = arg;
	const char*  s;
	const char*  t;
	char         buf[6];
	
	/* Seperator */
	for (t = "%20"; *w != eos && *t; t++)
		*(*w)++ = *t;
	
	/* Begin formatting it into url goop */
	for (s = keyword; *w != eos && *s;)
	{
		if ((*s >= 48 && *s <= 57) ||
		    (*s >= 97 && *s <=122) ||
		    (*s >= 65 && *s <= 90))
		{
			*(*w)++ = *s++;
		}
		else if (*s == '/')
		{	/* special case: '/' can't live on FS */
			for (t = "%01"; *w != eos && *t; t++)
				*(*w)++ = *t;
			/* lurkerd knows 01 = / */
			s++;
		}
		else
		{
			sprintf(&buf[0], "%%%02X", *s++);
			for (t = &buf[0]; *w != eos && *t; t++)
				*(*w)++ = *t;
		}
	}
	
	return 0;
}

static void extract_keyword(
	const char* parameter, 
	char** w,
	char* e,
	char* scratch,
	const char* prefix,
	const char* field)
{
	const char* s;
	char*       o;
	int         i;
	
	s = parameter-1;
	do s = strstr(s+1, field);
	while (s && s != parameter && *(s-1) != '&');
	
	if (s == 0)
		return;
	
	/* skip the field */
	s += strlen(field); 
	
	/* pass the = */
	if (*s != '=') return;
	s++;
	
	/* De-urlify the data */
	for (o = scratch; *s && *s != '&'; o++)
	{
		if (*s == '+') { *o = ' '; s++; }
		else if (*s == '%')
		{
			s++;
			if (*s && *(s+1))
			{
				sscanf(s, "%2X", &i);
				*o = i;
				s += 2;
			}
			else
			{
				*o = '?';
			}
		}
		else
		{
			*o = *s++;
		}
	}
	*o = 0;
	
	eos = e;
	my_keyword_digest_string(scratch, o - scratch, prefix,
		&pump_keyword, w, 0);
}

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
	char  buf[4096];
	char* w;
	char* e;
	const char* s;
	char* scratch;
	int   tmp;
	
	time_t		start, end;
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
			"Not invoked as a GET cgi",
			"REQUEST_URI invalid",
			"environment variable missing");
		return 0;
	}
	
	if (!qs)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0],
			"Not invoked as a GET cgi",
			"QUERY_STRING invalid",
			"environment variable missing");
		return 0;
	}
	
	/* Prune off the first ? and then the last / -> get path */
	w = strchr(uri, '?');
	if (w) *w = 0;
	w = strrchr(uri, '/');
	if (w) *w = 0;
	
	/* Ok, uri is now the path relative to which the request goes */
	
	/* Start slapping on parameters */
	e = &buf[sizeof(buf)];
	w = &buf[0];
	scratch = strdup(qs);
	
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_SUBJECT, "subject");
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_AUTHOR,  "author");
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_WORD,    "query");
		
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_WEEKDAY,      "weekday");
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_DAY_OF_MONTH, "dom");
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_MONTH,        "month");
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_YEAR,         "year");
	extract_keyword(qs, &w, e, scratch, LU_KEYWORD_LIST,         "list");
	
	start = time(0);
	memcpy(&tms, localtime(&start), sizeof(struct tm));
	tmp = extract_int(qs, "start-sec");  tms.tm_sec  = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "start-min");  tms.tm_min  = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "start-hour"); tms.tm_hour = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "start-mday"); tms.tm_mday = (tmp==-1)?1:tmp;
	tmp = extract_int(qs, "start-mon");  tms.tm_mon  = (tmp==-1)?0:tmp-1;
	tmp = extract_int(qs, "start-year"); if (tmp!=-1) tms.tm_year = tmp-1900;
	start = mktime(&tms);
	
	end = time(0);
	memcpy(&tms, localtime(&end), sizeof(struct tm));
	tmp = extract_int(qs, "end-sec");  tms.tm_sec  = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "end-min");  tms.tm_min  = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "end-hour"); tms.tm_hour = (tmp==-1)?0:tmp;
	tmp = extract_int(qs, "end-mday"); tms.tm_mday = (tmp==-1)?1:tmp;
	tmp = extract_int(qs, "end-mon");  tms.tm_mon  = (tmp==-1)?0:tmp-1;
	tmp = extract_int(qs, "end-year"); if (tmp!=-1) tms.tm_year = tmp-1900;
	end = mktime(&tms);
	
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
	printf("Location: %s/search/0%%20%ld%%20%ld%s.%s\r\n", 
		uri, (long)start, (long)end, &buf[0], s);
	printf("Content-type: text/%s\r\n\r\n", s);
	printf(&redirect_error[0], 
		uri, (long)start, (long)end, &buf[0], s);
	
	return 0;
}
