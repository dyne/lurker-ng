/*  $Id: main.c,v 1.3 2002-02-10 07:11:51 terpstra Exp $
 *  
 *  main.c - render missing pages
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
#include "io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#define LIMIT_URI	512

#define LIBXSLT_PUBLIC

#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/HTMLtree.h>
#include <libxml/xmlIO.h>
#include <libxml/DOCBparser.h>
#include <libxml/xinclude.h>
#include <libxml/catalog.h>
#include <libxslt/xslt.h>
#include <libxslt/xsltInternals.h>
#include <libxslt/transform.h>
#include <libxslt/xsltutils.h>

FILE* lu_server_link;

static const char not_found[] =
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html>\r\n"
" <head><title>404 Not Found</title></head>\r\n"
" <body><h1>Not Found</h1>\r\n"
"   The requested URL %s was not found on this server.\r\n"
"   <p><hr>\r\n"
" </body>\r\n"
"</html>\r\n";

const char basic_error[] =
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html>\r\n"
" <head><title>" PACKAGE " - error rendering page</title></head>\r\n"
" <body>\r\n"
"  <h1>" PACKAGE " - error rendering page:</h1>\r\n"
"  <p>%s (%s): %s\r\n"
"  <p><hr>\r\n"
" </body>\r\n"
"</html>\r\n";

const char redirect_error[] = 
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html><head>\r\n"
"<title>301 Moved Permanently</title>\r\n"
"</head><body>\r\n"
"<h1>Moved Permanently</h1>\r\n"
"The document has moved <a href=\"%s\">here</a>.\r\n"
"<p><hr>\r\n"
"</body></html>\r\n";

inline int fromhex(char ch)
{
	if (ch >= 'A' && ch <= 'F') return ch - 'A' + 10;
	if (ch >= 'a' && ch <= 'f') return ch - 'a' + 10;
	if (ch >= '0' && ch <= '9') return ch - '0';
	return 0;
}

FILE* lu_render_open(const char* parameter)
{
	char buf[LIMIT_URI + 10];
	int fd;
	FILE* out;
	
	snprintf(&buf[0], sizeof(buf), "%s.xml", parameter);
	fd = open(&buf[0], 
		O_CREAT | O_RDWR | O_BINARY | O_EXCL,
		LU_S_READ | LU_S_WRITE);
	
	if (fd == -1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not able to create rendered file",
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

int lu_render_close(FILE* f)
{
	int fd = dup(fileno(f));
	if (fd == -1)
	{	/* Fawk, can't dup it, fallback: */
		fflush(f);
		return fileno(f);
	}
	
	fclose(f);
	return fd;
}

static int render_html(const char* parameter)
{
	char	buf[LIMIT_URI + 10];
	int	fd;
	FILE*	out;
	
	xsltStylesheetPtr	cur = 0;
	xmlDocPtr		doc, res;
	const char*		params[1];
	
	snprintf(&buf[0], sizeof(buf), "%s.html", parameter);
	fd = open(&buf[0], 
		O_CREAT | O_RDWR | O_BINARY | O_EXCL,
		LU_S_READ | LU_S_WRITE);
	
	if (fd == -1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not able to create rendered file",
			&buf[0],
			strerror(errno));
		return 0;
	}
	
	if ((out = fdopen(fd, "w")) == 0)
	{
		printf(&basic_error[0], 
			"Not able to fdopen an fd",
			&buf[0],
			strerror(errno));
		return -1;
	}
	
	params[0] = 0;
	
	xmlSubstituteEntitiesDefault(1);
	xmlLoadExtDtdDefaultValue = 1;
	cur = xsltParseStylesheetFile((const xmlChar*)"render.xslt");
	
	snprintf(&buf[0], sizeof(buf), "%s.xml", parameter);
	doc = xmlParseFile(&buf[0]);
	res = xsltApplyStylesheet(cur, doc, params);
	xsltSaveResultToFile(out, res, cur);
	
	xsltFreeStylesheet(cur);
	xmlFreeDoc(res);
	xmlFreeDoc(doc);
	xsltCleanupGlobals();
	xmlCleanupParser();
	
	return lu_render_close(out);
}

int main(int argc, char* argv[])
{
	int	html;
	char*	origuri;
	char*	uri;
	char*	suffix;
	char*	parameter;
	char*	module;
	char*   r;
	char*   w;
	int   (*handler)(char* parameter);
	int	fd;
	char	buf[4096];
	int	got;
	
	int			sun_fd;
	struct sockaddr_un	sun_addr;
	int			sun_len;
	
	/* What URL are we rendering? */
	if ((origuri = getenv("REQUEST_URI")) == 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not invoked as an ErrorDocument", 
			"either shell or normal cgi invokation",
			"The .htaccess file must be modified");
		return 1;
	}
	uri = strdup(origuri);
	
	/* Try to connect to the server */
	if ((sun_fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Unable to connect to server", 
			"opening socket",
			strerror(errno));
		return 1;
	}
	
	memset(&sun_addr, 0, sizeof(sun_addr));
	sun_addr.sun_family = PF_UNIX;
	strcpy(&sun_addr.sun_path[0], PACKAGE ".sock");
	sun_len = sizeof(sun_addr.sun_family) + strlen(&sun_addr.sun_path[0]) + 1;
	
	if (getcwd(&buf[0], sizeof(buf)) == 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Saving location of cgi", 
			"getcwd",
			strerror(errno));
		return 1;
	}
	
	/*!!! use some parameter to the cgi instead */\
	if (chdir(DBDIR) != 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Unable to connect to server", 
			"chdir",
			strerror(errno));
		return 1;
	}
	
	if (connect(sun_fd, (struct sockaddr*)&sun_addr, sun_len) != 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Unable to connect to server", 
			"connect",
			strerror(errno));
		return 1;
	}
	
	if (chdir(&buf[0]) != 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Restoring location of cgi", 
			"chdir",
			strerror(errno));
		return 1;
	}
	
	if ((lu_server_link = fdopen(sun_fd, "w+")) == 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Unable to connect to server", 
			"fdopen",
			strerror(errno));
		return 1;
	}
	
	/* First, decode the URI - kill %XX values */
	w = r = uri;
	while (*r) switch (*r)
	{
	 case '%':
	 	if (*(r+1) && *(r+2))
	 	{
	 		*w++ = fromhex(*++r) << 4 | fromhex(*++r);
		 	r++;
		 	break;
	 	}
	 default:
	 	*w++ = *r++;
	}
	*w = 0;
	
	/* Now, scan it and break it into pieces */
	
	for (suffix = uri + strlen(uri); suffix != uri;	suffix--)
	{
		if (*suffix == '.')
		{
			*suffix++ = 0;
			break;
		}
	}
	
	if (suffix == uri)
	{
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], origuri);
		return 1;
	}
	
	if      (!strcmp(suffix, "html")) html = 1;
	else if (!strcmp(suffix, "xml"))  html = 0;
	else
	{
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], origuri);
		return 1;
	}
	
	for (parameter = suffix - 1; parameter != uri; parameter--)
	{
		if (*parameter == '/')
		{
			*parameter++ = 0;
			break;
		}
	}
	
	if (parameter == uri)
	{
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], origuri);
		return 1;
	}
	
	for (module = parameter - 1; module != uri; module--)
	{
		if (*module == '/')
		{
			module++;
			break;
		}
	}
	
	if (module == uri)
	{
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], origuri);
		return 1;
	}
	
	if      (!strcmp(module, "message")) handler = &lu_message_handler;
	else if (!strcmp(module, "thread" )) handler = &lu_thread_handler;
	else if (!strcmp(module, "mindex" )) handler = &lu_mindex_handler;
	else if (!strcmp(module, "tindex" )) handler = &lu_tindex_handler;
	else if (!strcmp(module, "search" )) handler = &lu_search_handler;
	else
	{
		printf("Status: 404 Not Found\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&not_found[0], origuri);
		return 1;
	}
	
	/* Get into the correct directory */
	
	if (chdir(module) == -1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Not able to enter module directory",
			module,
			strerror(errno));
		return 1;
	}
	
	if (strlen(parameter) >= LIMIT_URI)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Requested URI is invalid",
			parameter,
			"too long");
		return 1;
	}
	
	*(suffix+0) = 'x';
	*(suffix+1) = 'm';
	*(suffix+2) = 'l';
	*(suffix+3) = 0;
	*(suffix-1) = '.';
	
	if (html && access("render.xslt", R_OK) == -1)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			"Unable to access stylesheet",
			"render.xslt",
			strerror(errno));
		return 1;
	}
	
	if (access(parameter, R_OK) == -1)
	{
		*(suffix-1) = 0;
		
		/* Invoke the correct handler */
		if ((fd = handler(parameter)) == -1)
		{
			return 1;
		}
		
		/* Reseek to the start */
		if (lseek(fd, 0, SEEK_SET) == -1)
		{
			printf("Status: 404 Not Found\r\n");
			printf("Content-type: text/html\r\n\r\n");
			printf(&not_found[0], origuri);
			return 1;
		}
	}
	else
	{
		fd = -1;
		*(suffix-1) = 0;
		
		/* the file exists already - wtf - we're broken */
		if (!html)
		{
			printf("Status: 200 OK\r\n");
			printf("Content-type: text/html\r\n\r\n");
			printf(&basic_error[0], 
				"File already exists",
				parameter,
				strerror(errno));
			return 1;
		}
	}
	
	printf("Status: 200 OK\r\n");
	
	if (html)
	{
		printf("Content-type: text/html\r\n\r\n");
		fflush(stdout);
		
		if (fd != -1) close(fd);
		
		/* Make stderr to stdout */
		close(2);
		dup(1);
		
		fd = render_html(parameter);
		
		/* Reseek to the start */
		if (lseek(fd, 0, SEEK_SET) == -1)
		{
			printf("Status: 404 Not Found\r\n");
			printf("Content-type: text/html\r\n\r\n");
			printf(&not_found[0], origuri);
			return 1;
		}
	}
	else
	{
		printf("Content-type: text/xml\r\n\r\n");
		fflush(stdout);
	}
	
	/* Pipe it through */
	while ((got = read(fd, &buf[0], sizeof(buf))) > 0)
		write(1, &buf[0], got);
	
	return 0;
}
