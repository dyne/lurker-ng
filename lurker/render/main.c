/*  $Id: main.c,v 1.22 2002-06-21 00:44:58 terpstra Exp $
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

#include "common.h"
#include "io.h"
#include "error.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/un.h>
#include <sys/socket.h>

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

/* #define DEBUG 1 */

FILE*	lu_server_link;
FILE*	lu_output;
char	cwd[1024];

const char redirect_error[] = 
"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
"<html><head>\r\n"
"<title>301 Moved Permanently</title>\r\n"
"</head><body>\r\n"
"<h1>Moved Permanently</h1>\r\n"
"The document has moved <a href=\"%s\">here</a>.\r\n"
"<p><hr>\r\n"
"</body></html>\r\n";

int lu_forward_data()
{
	char	buf[4096];
	size_t	got;
	size_t  get;
	int	fragment;
	
	fflush(lu_server_link);
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
		fwrite(&buf[0], 1, got, lu_output);
		
		fragment -= got;
	}
	
	return 0;
}

int lu_connect_server()
{
	char*	query;
	
	int			sun_fd;
	struct sockaddr_un	sun_addr;
	int			sun_len;
	
	if ((query = getenv("QUERY_STRING")) == 0)
	{
		printf("Content-Type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Not invoked as a get cgi"), 
			_("Possibly a shell invokation or post"),
			_("The .htaccess file must be modified"));
		return -1;
	}
	
	if ((sun_fd = socket(PF_UNIX, SOCK_STREAM, 0)) == -1)
	{
		printf("Content-Type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Unable to connect to server"), 
			_("opening socket"),
			strerror(errno));
		return -1;
	}
	
	memset(&sun_addr, 0, sizeof(sun_addr));
	sun_addr.sun_family = PF_UNIX;
	strcpy(&sun_addr.sun_path[0], PACKAGE ".sock");
	sun_len = sizeof(sun_addr.sun_family) + strlen(&sun_addr.sun_path[0]) + 1;
	
	if (chdir(*query?query:DBDIR) != 0)
	{
		printf("Content-Type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Unable to connect to server - chdir failed"), 
			*query?query:DBDIR,
			strerror(errno));
		return -1;
	}
	
	if (connect(sun_fd, (struct sockaddr*)&sun_addr, sun_len) != 0)
	{
		printf("Content-Type: text/html\r\n\r\n");
		if (errno == ENOENT || errno == ECONNREFUSED)
		{
			printf(&basic_error[0], 
				_("Unable to connect to server"), 
				"connect",
				_("The daemon (lurkerd) is not running"));
		}
		else
		{
			printf(&basic_error[0], 
				_("Unable to connect to server"), 
				"connect",
				strerror(errno));
		}
		return -1;
	}
	
	if (chdir(&cwd[0]) != 0)
	{
		printf("Content-Type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Restoring location of cgi"), 
			"chdir",
			strerror(errno));
		return -1;
	}
	
	if ((lu_server_link = fdopen(sun_fd, "w+")) == 0)
	{
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Unable to connect to server"), 
			"fdopen",
			strerror(errno));
		return 1;
	}
	
	return 0;
}

int main(int argc, char* argv[])
{
	char	cache[10];
	char	type[100];
	char	buf[1024];
	char*	uri;
	char*	qs;
	char*	mod;
	char*	ext;
	char*	w;
	char*   r;
	int	i;
	int	stdoutfd = 1;
	int	fd;
	int	got;
	
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	bind_textdomain_codeset(PACKAGE, "utf-8");
	
	/* What URL are we rendering? */
	if ((uri = getenv("REQUEST_URI")) == 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Not invoked as an ErrorDocument"), 
			_("either shell or normal cgi invokation"),
			_("The .htaccess file must be modified"));
		return 1;
	}
	
	uri = strdup(uri);
	ext = strrchr(uri, '.');
	qs = strrchr(uri, '/');
	
	if (!qs || ext <= qs)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0],
			_("Improper request"),
			_("Malformed REQUSET_URI"),
			_("Missing an extension"));
		return 1;
	}
	
	*ext++ = 0;
	*qs++  = 0;
	mod = strrchr(uri, '/');
	
	if (!mod)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0],
			_("Improper request"),
			_("Malformed REQUSET_URI"),
			_("Missing the request directory"));
		return 1;
	}
	*mod++ = 0;
	
	/* Decode the query string */
	for (w = r = qs; *r; )
	{
		switch (*r)
		{
		case '+':
			*w++ = ' ';
			r++; 
			break;
		case '%':
			r++;
			if (*r && *(r+1))
			{
				sscanf(r, "%2X", &i);
				r += 2;
				*w++ = i;
			}
			else	*w++ = '?';
			break;
		default:
			*w++ = *r++;
		}
	}
	*w++ = 0;
	
	if (getcwd(&cwd[0], sizeof(cwd)) == 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-Type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Saving location of cgi"), 
			"getcwd",
			strerror(errno));
		return 1;
	}
	
	if (chdir(mod) != 0)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0], 
			_("Invalid request"), 
			"chdir&access",
			strerror(errno));
		return 1;
	}
	
	if (lu_connect_server() != 0)
		return 1;
	
	/* Get back into the directory; connect leaves it. */
	chdir(mod);
	
	/* Create the file immediately. The server might delete it due to
	 * import WHILE we are generating it. It must be around to get 
	 * unlinked already.
	 */
	snprintf(&buf[0], sizeof(buf), "%s.%s", qs, ext);
	fd = open(&buf[0], O_RDWR | O_BINARY | O_CREAT | O_EXCL,
		LU_S_READ | LU_S_WRITE);
	
	if (fd == -1 && errno != EEXIST)
	{
		printf("Status: 200 OK\r\n");
		printf("Content-type: text/html\r\n\r\n");
		printf(&basic_error[0],
			_("Unable to create cache file"),
			&buf[0],
			strerror(errno));
		return 1;
	}
	
	if (fd == -1) /* errno = EEXIST */
	{
		printf("Status: 303 Moved Permanently\r\n");
		printf("Location: %s\r\n", getenv("REQUEST_URI"));
		printf("Content-type: text/html\r\n\r\n");
		printf(&redirect_error[0], getenv("REQUEST_URI"));
		return 0;
	}
	
	printf("Status: 200 OK\r\n");
	
	/* Send the server the request */
	fprintf(lu_server_link, "%s\n%s\n%s\n%s\n", &cwd[0], mod, qs, ext);
	fflush(lu_server_link);
	
	/* Read back server processing directions */
	fgets(&type [0], sizeof(type),  lu_server_link);
	fgets(&cache[0], sizeof(cache), lu_server_link);
#ifdef DEBUG
	printf("%s - %s\n", type, cache);
#endif
	
	if (strlen(type) > 1) type[strlen(type)-1] = 0;
	
	/* Print out the correct content-type */
	if (!strcmp(type, "text/xml"))
	{
		if (strcmp(ext, "xml"))
		{
			printf("Content-type: text/html\r\n\r\n");
		}
		else
		{
			printf("Content-type: text/xml\r\n\r\n");
		}
	}
	else
	{
		printf("Content-type: %s\r\n\r\n", type);
	}
	fflush(stdout);
	
	/* Redirect stdout to cache file */
	stdoutfd = dup(1);
	close(1);
	dup(fd);
	
	/* If we are not supposed to cache, kill the file now. We will
	 * still have a handle to it. 
	 */
	if (cache[0] == '0')
	{
		unlink(&buf[0]);
	}
	
	/* If we need xslt conversion, prep it */
	if (!strcmp(type, "text/xml") && strcmp(ext, "xml"))
		lu_output = popen("../" PACKAGE ".xslt ../fmt/render-html.xsl", "w");
	else
		lu_output = stdout;
	
	if (lu_forward_data() != 0)
		return 1;
	
	if (!strcmp(type, "text/xml") && strcmp(ext, "xml"))
		pclose(lu_output);
	
	/* Reclaim our stdout */
	fflush(stdout);
	close(1);
	dup(stdoutfd);
	close(stdoutfd);
	
	/* Reseek to the start */
	if (lseek(fd, 0, SEEK_SET) == -1)
	{
		printf(&basic_error[0], 
			_("Unable to rewind to start of output"),
			_("reseek cache file"),
			strerror(errno));
		return 1;
	}
	
	/* Pipe it through */
	while ((got = read(fd, &buf[0], sizeof(buf))) > 0)
		write(1, &buf[0], got);
	
	close(fd);
	return 0;
}
