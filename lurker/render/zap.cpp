/*  $Id: zap.cpp,v 1.4 2006-03-01 14:55:45 terpstra Exp $
 *  
 *  zap.cpp - Handle a zap/ command
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

#include <mimelib/message.h>
#include <mimelib/headers.h>
#include <mimelib/bodypart.h>
#include <mimelib/body.h>
#include <mimelib/enum.h>
#include <mimelib/mediatyp.h>
#include <mimelib/utility.h>

#include "commands.h"
#include "parse.h"
#include "Summary.h"

#include <iostream>
#include <iomanip>
#include <cstdio>

using std::cout;

static void find_and_replace(string& target, const string& token, const string& value)
{
	string::size_type x = 0;
	while ((x = target.find(token, x)) != string::npos)
		target.replace(x, token.length(), value);
}

int handle_zap(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	
	if (!MessageId::is_full(req.options.c_str()))
		error(_("Bad request"), param,
		      _("The given parameter was not of the correct format. "
		        "A zap request must be formatted like: "
		        "zap/YYYYMMDD.HHMMSS.hashcode.lc.xml"));
	
	if (cfg.delete_message == "")
		error(_("Permission Denied"), param,
		      _("Deleting email (option delete_message) has been disabled. "
		        "Contact the site administrator if this is a problem."));
	
	MessageId id(req.options.c_str());
	string ok;
	
	Summary source(id);
	// Identical error if missing or not allowed (security)
	if ((ok = source.load(db, cfg)) != "" || !source.allowed())
	{
		if (ok == "") ok = "not in a mailbox"; // fake
		error(_("Database zap source pull failure"), ok,
		      _("The specified message does not exist."));
	}
	
	if (source.deleted())
		error(_("Database zap source pull failure"), "not found",
		      _("The specified message has been deleted."));
	
	string cmd = cfg.delete_message;
	find_and_replace(cmd, "%c", cfg.file);
	find_and_replace(cmd, "%i", id.serialize());
	
	map<string, string> cookies = getCookies();
	string pass = cookies["lurker-pass"];
	
#if 0
	cmd += " 2>&1";
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/plain\r\n\r\n";
	cout << cmd << "\n";
	cout << pass << "\n";
	cout << std::flush;
#endif
	
	FILE* f = popen(cmd.c_str(), "w");
	if (!f)
		error(_("Executing delete command failed"), cmd,
		      _("Perhaps the command is in error?"));
	
	fputs(pass.c_str(), f);
	if (pclose(f) != 0)
		error(_("Delete command failed"), cmd,
		      _("Perhaps you prodived a bad password?"),
		      "Set-Cookie: lurker-pass=wrong; path=/; expires=Wed, 10 Jan 1990 20:00:00 GMT\r\n");
	
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << "<html><body>\n";
	cout << _("The message has been deleted.<br>\n");
	cout << _("It will disappear from the web when the cronjob next runs.<br>\n");
	cout << _("Going back two pages in 1 second...\n");
	cout << "<script language=\"JavaScript\"><!--\n";
	cout << "  function goBack() { history.go(-2); }\n";
	cout << "  setTimeout('goBack()', 1000);\n";
	cout << "//--></script>\n";
	cout << "</body></html>\n";
	
	return 0;
}
