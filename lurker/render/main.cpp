/*  $Id: main.cpp,v 1.4 2003-04-23 23:22:09 terpstra Exp $
 *  
 *  main.cpp - Transform a database snapshot to useful output
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

#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "commands.h"

#include <unistd.h>	// chdir

/* #define DEBUG 1 */

using namespace std;

string redirect(const string& url)
{
	return	"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html><head>\r\n"
		"<title>301 Moved Permanently</title>\r\n"
		"</head><body>\r\n"
		"<h1>Moved Permanently</h1>\r\n"
		"The document has moved <a href=\""
		+ url + 
		"s\">here</a>.\r\n"
		"<p><hr>\r\n"
		"</body></html>\r\n";
}

string error(
	const string& main, 
	const string& sub, 
	const string& suggest)
{
	return	"<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
		"<html>\r\n"
		" <head><title>Lurker - "
		+ main + 
		"</title></head>\r\n"
		" <body>\r\n"
		"  <h1>Lurker - failed to render page:</h1>\r\n"
		"  <h2>"
		+ main + " (" + sub + "):</h2><p>\r\n"
		+ suggest + "\r\n"
		"  <p><hr>\r\n"
		" </body>\r\n"
		"</html>\r\n";
}

void tokenize(
	const string& str,
	vector<string>& tokens,
	const string& delimiters)
{
	string::size_type lastPos = str.find_first_not_of(delimiters, 0);
	string::size_type pos     = str.find_first_of(delimiters, lastPos);
	
	while (string::npos != pos || string::npos != lastPos)
	{
		tokens.push_back(str.substr(lastPos, pos - lastPos));
		lastPos = str.find_first_not_of(delimiters, pos);
		pos = str.find_first_of(delimiters, lastPos);
	}
}

void help(const string& about)
{
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << error(_("Not invoked correctly"), about,
		_("The lurker.cgi must have two parameters: the config file "
		  "and the request to render. The value of QUERY_STRING is "
		  "taken to be the config file, and REQUEST_URI is taken "
		  "to be the requested page to render. Setting a 404 error "
		  "handler to lurker.cgi?config.file will usually set these "
		  "environment variables. Additionally, lurker may be invoked "
		  "from the command-line. Here, the first parameter is the "
		  "config file and the second is the requested uri."));
}

int main(int argc, char** argv)
{
	string config, cdpath, request, host, port, cgipath;
	const char* tmp;
	
#if 0	
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	bind_textdomain_codeset(PACKAGE, "utf-8");
#endif
	
	if ((tmp = getenv("QUERY_STRING")) != 0) config  = tmp;
	if ((tmp = getenv("REQUEST_URI" )) != 0) request = tmp;
	if ((tmp = getenv("SERVER_NAME" )) != 0) host    = tmp;
	if ((tmp = getenv("SERVER_PORT" )) != 0) port    = tmp;
	if ((tmp = getenv("SCRIPT_NAME" )) != 0) cgipath = tmp;
	
	if (argc > 1) config  = argv[1];
	if (argc > 2) request = argv[2];
	
	string::size_type csplit;
	if ((csplit = config.find('?')) != string::npos)
	{
		cdpath = config.substr(csplit+1, string::npos);
		
		
		if (csplit != 0 && config[csplit-1] == '\\')
			config.resize(csplit-1);
		else	config.resize(csplit);
	}
	
	if (config == "")
	{
		help(_("no config file set"));
		return 1;
	}
	if (request == "")
	{
		help(_("no request set"));
		return 1;
	}
	
	if (cdpath != "" && chdir(cdpath.c_str()) != 0)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Cannot chdir"), cdpath + ":" + strerror(errno),
			_("The specified path to the document root could "
			  "not be entered. Check the argument and permissions."));
		return 1;
	}
	
	Config cfg;
	if (cfg.load(config.c_str()) != 0)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Cannot open config file"), "Config::load",
			cfg.error.str());
		return 1;
	}
	
	auto_ptr<ESort::Reader> db(ESort::Reader::open(cfg.dbdir + "/db"));
	if (!db.get())
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Cannot open database snapshot"), strerror(errno),
			_("The configured database 'dbdir' in the config file "
			  "could not be opened. Typically this means that it is "
			  "not readable by the user which the cgi is invoked as. "
			  "We suggest making dbdir and all files in it readable "
			  "by everyone since you are serving them on a website "
			  "anyways."));
		return 1;
	}
	
	vector<string> tokens;
	tokenize(request, tokens, "/");
	if (tokens.size() < 2)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Request malformed"), "tokenize(request)",
			_("The request does not have at least two directory "
			  "components. It must be like ..../command/param.xml"));
		return 1;
	}
	
	string param   = tokens[tokens.size()-1];
	string command = tokens[tokens.size()-2];
	
	cfg.cgiUrl = "http://" + host + ":" + port + cgipath;
	string::size_type psplit;
	if ((psplit = cfg.cgiUrl.rfind('/')) != string::npos)
		cfg.cgiUrl.resize(psplit);
	
	vector<string>::size_type tok;
	cfg.docUrl = "http://" + host + ":" + port;
	for (tok = 0; tok < tokens.size()-2; ++tok)
		cfg.docUrl += "/" + tokens[tok];
	
	// flush all request data in case user made it huge to be an ass
	tokens.clear();
	request = "";
	config = "";
	
	if      (command == "message") return handle_message(cfg, db.get(), param);
	else if (command == "thread")  return handle_thread (cfg, db.get(), param);
	else if (command == "mindex")  return handle_mindex (cfg, db.get(), param);
	else if (command == "splash")  return handle_splash (cfg, db.get(), param);
	else if (command == "search")  return handle_search (cfg, db.get(), param);
	else if (command == "attach")  return handle_attach (cfg, db.get(), param);
	else if (command == "mbox")    return handle_mbox   (cfg, db.get(), param);
	else
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Bad command"), command,
			_("The requested command is not supported."));
		return 1;
	}
}
