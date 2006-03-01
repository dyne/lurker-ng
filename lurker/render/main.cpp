/*  $Id: main.cpp,v 1.25 2006-03-01 14:55:45 terpstra Exp $
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
 *    the Free Software Foundation; version 2.
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

#define _FILE_OFFSET_BITS 64

#include <cstdlib>
#include <cerrno>
#include <cstring>

#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "commands.h"
#include "parse.h"

#include <XmlEscape.h>
#include <unistd.h>	// chdir

/* #define DEBUG 1 */

using namespace std;

void error(
	const string& main, 
	const string& sub, 
	const string& suggest,
	const string& header)
{
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << "<!DOCTYPE HTML PUBLIC \"-//IETF//DTD HTML 2.0//EN\">\r\n"
	     << "<html>\r\n"
	     << " <head><title>Lurker - "
	     <<	xmlEscape << main
	     << "</title></head>\r\n"
	     << " <body>\r\n"
	     << "  <h1>Lurker - failed to render page:</h1>\r\n"
	     << "  <h2>"
	     << xmlEscape << main << " (" << xmlEscape << sub << "):</h2><p>\r\n"
	     << xmlEscape << suggest << "\r\n"
	     << "  <p><hr>\r\n"
	     <<	" </body>\r\n"
	     << "</html>\r\n";
	exit(1);
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
	error(_("Not invoked correctly"), about,
		_("The lurker.cgi must one parameters: the requested file. "
		  "The value of QUERY_STRING is taken to be the path of the "
		  "request file, and REQUEST_URI is taken to be its URL. "
		  "Setting a 404 error handler to lurker.cgi?/frontend/path "
		  "will usually set these environment variables. "
		  "Additionally, lurker can be told to use an alternate config "
		  "file via the LURKER_CONFIG environment variable."));
}

Request parse_request(const string& param)
{
	string::size_type dot1 = param.rfind('.');
	if (dot1 == string::npos || dot1 == 0)
		error(_("Missing extension"), param,
		      _("An extension for the request was required, but missing"));
	
	string::size_type dot2 = param.rfind('.', dot1-1);
	
	// This is an attempt at backwards compatability.
	// Previously lurker had no language code in the URL, but still had
	// period delimited fields in message ids. The last part is always 8
	// characters long, so we assume english if the country code is
	// not of length in [2, 8).
	// NOTE: some search URLs will still be broken (those with .s)
	if (dot2 != string::npos && (dot1 - dot2 < 2+1 || dot1 - dot2 >= 8+1))
		dot2 = string::npos;
	
	Request out;
	
	if (dot2 == string::npos)
	{
		out.options  = param.substr(0, dot1);
		out.language = "en";
		out.ext = param.substr(dot1+1, string::npos);
	}
	else
	{
		out.options  = param.substr(0, dot2);
		out.language = param.substr(dot2+1, (dot1-dot2) - 1);
		out.ext = param.substr(dot1+1, string::npos);
	}
	
	if (!lstring::locale_normalize(out.language))
		error(_("Bogus locale"), out.language,
		      _("The specified locale is not valid."));
	
	return out;
}

int main(int argc, char** argv)
{
	string config, frontend, document, request, host, port, cgipath, https;
	const char* tmp;
	
#if 0	
	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
	bind_textdomain_codeset(PACKAGE, "utf-8");
#endif
	
	// Every document about CGI agrees these exist:
	if ((tmp = getenv("QUERY_STRING")) != 0) document = tmp;
	if ((tmp = getenv("SERVER_NAME" )) != 0) host     = tmp;
	if ((tmp = getenv("SERVER_PORT" )) != 0) port     = tmp;
	if ((tmp = getenv("SCRIPT_NAME" )) != 0) cgipath  = tmp;
	// Many CGI 'standards' seem to agree this one exists for https:
	if ((tmp = getenv("HTTPS"       )) != 0) https    = tmp;
	if ((tmp = getenv("LURKER_CONFIG"))!= 0) config   = tmp;
	
	// CGI guarantees this in case called as an error document
	if ((tmp = getenv("REDIRECT_URL")) != 0) request  = tmp;
	// ... however, as we aren't always called that way, try this too:
	if ((tmp = getenv("REQUEST_URI" )) != 0) request  = tmp;
	
	if (config == "") config = DEFAULT_CONFIG_FILE;
	
	if (request == "")
		help(_("no request set (REDIRECT_URL and REQUEST_URI both missing)"));
	
	Config cfg;
	if (cfg.load(config.c_str()) != 0)
		 error(_("Cannot open config file"), "Config::load",
		       cfg.getError());
	
	// Look for the matching front-end
	frontend = "";
	Config::Frontends::const_iterator i, e;
	for (i = cfg.frontends.begin(), e = cfg.frontends.end(); i != e; ++i)
	{
		if (i->first == document.substr(0, i->first.length()))
		{
			frontend = i->first;
			break;
		}
	}
	
	if (frontend == "")
		error(_("No matching frontend"), document,
		      _("The frontend specified in the webserver "
		        "configuration does not match any frontend in the "
		        "lurker config file."));
	
	cfg.set_permissions(cfg.frontends[frontend]);
	
	if (chdir(frontend.c_str()) != 0)
		error(_("Cannot chdir"), frontend + ":" + strerror(errno),
		      _("The specified frontend path could "
		        "not be entered. Check the path and permissions."));
	
	auto_ptr<ESort::Reader> db(ESort::Reader::opendb(cfg.dbdir + "/db"));
	if (!db.get())
		error(_("Cannot open database snapshot"), strerror(errno),
		      _("The configured database 'dbdir' in the config file "
		        "could not be opened. Typically this means that it is "
		        "not readable by the user which the cgi is invoked as. "
		        "We suggest making dbdir and all files in it readable "
		        "by everyone since you are serving them on a website "
		        "anyways."));
	
	vector<string> tokens;
	tokenize(request, tokens, "/");
	if (tokens.size() < 2)
		error(_("Request malformed"), "tokenize(request)",
		      _("The request does not have at least two directory "
		        "components. It must be like ..../command/param.xml"));
	
	string param   = decipherHalf(tokens[tokens.size()-1]);
	string command = tokens[tokens.size()-2];
	string server;
	
	if (document != frontend &&
	    frontend + '/' + command + '/' + param != document)
		error(_("Requested document is in error"), document,
		      _("The requested document does not match the file "
		        "lurker intends to generate: ")
		      + frontend + '/' + command + '/' + param);
	
	if (https == "on")
	{
		server = "https://" + host;
		if (port != "443") server += ":" + port;
	}
	else
	{
		server = "http://" + host;
		if (port != "80") server += ":" + port;
	}
	
	cfg.cgiUrl = server + cgipath;
	string::size_type psplit;
	if ((psplit = cfg.cgiUrl.rfind('/')) != string::npos)
		cfg.cgiUrl.resize(psplit);
	
	vector<string>::size_type tok;
	cfg.docUrl = server;
	for (tok = 0; tok < tokens.size()-2; ++tok)
		cfg.docUrl += "/" + tokens[tok];
	
	// flush all request data in case user made it huge to be an ass
	tokens.clear();
	request = "";
	config = "";
	
	cfg.command = command;
	
	if      (command == "message") return handle_message(cfg, db.get(), param);
	else if (command == "thread")  return handle_thread (cfg, db.get(), param);
	else if (command == "mindex")  return handle_mindex (cfg, db.get(), param);
	else if (command == "splash")  return handle_splash (cfg, db.get(), param);
	else if (command == "search")  return handle_search (cfg, db.get(), param);
	else if (command == "attach")  return handle_attach (cfg, db.get(), param);
	else if (command == "mbox")    return handle_mbox   (cfg, db.get(), param);
	else if (command == "list")    return handle_list   (cfg, db.get(), param);
	else if (command == "zap")     return handle_zap    (cfg, db.get(), param);
	else
		error(_("Bad command"), command, 
		      _("The requested command is not supported."));
}
