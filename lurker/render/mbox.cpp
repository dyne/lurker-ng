/*  $Id: mbox.cpp,v 1.2 2003-04-21 18:26:20 terpstra Exp $
 *  
 *  mbox.cpp - Handle a mbox/ command
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

#include <mimelib/message.h>

#include "commands.h"
#include "Summary.h"
#include "Cache.h"

int handle_mbox(const Config& cfg, ESort::Reader* db, const string& param)
{
	MessageId id(param.c_str());
	
	if (id.timestamp() == 0)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Bad request"), param,
			_("The given parameter was not of the correct format. "
			  "A mbox request must be formatted like: "
			  "mbox/YYYYMMDD.HHMMSS.hashcode.txt"));
		return 1;
	}
	
	string ok;
	
	Summary source(id);
	if ((ok = source.load(db)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database mbox source pull failure"), ok,
			_("The specified message does not exist."));
		return 1;
	}
	
	DwMessage message;
	if ((ok = source.message(cfg.dbdir, message)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("MBox read failure"), ok,
			_("Unable to open message in the mailbox. "
			  "Perhaps it has been deleted or moved?"));
		return 1;
	}
	
	Cache cache(cfg, "mbox", param);
	cache.o << message.AsString().c_str();
	
	return 0;
}
