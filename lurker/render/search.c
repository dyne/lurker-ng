#include "prefix.h"
#include "common.h"
#include "error.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char is_div[256];

const char redirect_error[] = 
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html><head>\r\n"
"<title>301 Moved Permanently</title>\r\n"
"</head><body>\r\n"
"<h1>Moved Permanently</h1>\r\n"
"The document has moved <a href=\"%s/search/0%%20%s.%s\">here</a>.\r\n"
"<p><hr>\r\n"
"</body></html>\r\n";

static void extract_keyword(
	const char* parameter, 
	char** w,
	char* e,
	char* scratch,
	const char* prefix,
	const char* field)
{
	const char* t;
	char* s;
	char* o;
	int i;
	char buf[6];
	
	if ((s = strstr(parameter, field)) == 0)
		return;
	
	/* make sure it is a parameter */
	if (s != parameter && *(s-1) != '&')
		return;
	
	/* skip the field */
	s += strlen(field); 
	
	/* pass the = */
	if (*s != '=') return;
	if (!*s++) return;
	
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
	
	/* Begin formatting it into url goop */
	for (s = scratch; *w != e && *s && *s != '&';)
	{
		/* Skip all the dividers */
		while (*s && *s != '&' && is_div[(int)*s]) s++;
		if (!*s || *s == '&') break;
		
		/* Seperator */
		for (t = "%20"; *w != e && *t; t++)
			*(*w)++ = *t;

		/* Output the prefix */
		for (t = prefix; *w != e && *t; t++)
			*(*w)++ = *t;
		
		/* Output up to next divider */
		while (*w != e && !is_div[(int)*s])
		{
			/* Don't do high-bit stuff */
			if (*s >= 127 || *s < 0) { s++; continue; }
			
			if ((*s >= 48 && *s <= 57) ||
			    (*s >= 64 && *s <= 90) ||
			    (*s >= 97 && *s <=122))
			{
				*(*w)++ = *s++;
			}
			else if (*s == '/')
			{	/* special case: '/' can't live on FS */
				for (t = "%01"; *w != e && *t; t++)
					*(*w)++ = *t;
				/* lurker knows 01 = / */
				s++;
			}
			else
			{
				sprintf(&buf[0], "%%%2X", *s++);
				for (t = &buf[0]; *w != e && *t; t++)
					*(*w)++ = *t;
			}
		}
	}
}

int main(int argc, char** argv)
{
	char* uri = getenv("REQUEST_URI");
	char* qs  = getenv("QUERY_STRING");
	char  buf[4096];
	char* w;
	char* e;
	char* s;
	char* scratch;
	int   i;
	
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
	
	/* Bootstrap the conversion table */
	memset(&is_div[0], 0, sizeof(is_div));
	for (i = 0; i < 040; i++)	is_div[i] = 1;
	for (s = WORD_BREAKS; *s; s++)	is_div[(int)*s] = 1;
	
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
	
	/* Find the format */
	s = qs;
	do s = strstr(s, "format=");
	while (s && s != qs && *(s-1) != '&');
	if (s)
	{
		s += strlen("format=");
		for (w = s; *w && *w != '&'; w++);
		*w = 0;
	}
	else	s = "xml";
	
	printf("Status: 303 Moved Permanently\r\n");
	printf("Location: %s/search/0%%20%s.%s\r\n", uri, &buf[3], s);
	printf("Content-type: text/%s\r\n\r\n", s);
	printf(&redirect_error[0], uri, &buf[3], s);
	
	return 0;
}
