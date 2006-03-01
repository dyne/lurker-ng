/*  $Id: mbox.cpp,v 1.13 2006-03-01 14:55:45 terpstra Exp $
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

#include "commands.h"
#include "Summary.h"
#include "Cache.h"

int handle_mbox(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	
	if (!MessageId::is_full(req.options.c_str()))
		error(_("Bad request"), param,
		      _("The given parameter was not of the correct format. "
		        "A mbox request must be formatted like: "
		        "mbox/YYYYMMDD.HHMMSS.hashcode.txt"));
	
	if (!cfg.raw_email)
		error(_("Permission Denied"), param,
		      _("Access to raw email text has been disabled. "
		        "Contact the site administrator if this is a problem."));
	
	MessageId id(req.options.c_str());
	string ok;
	
	Summary source(id);
	// Identical error if missing or not allowed (security)
	if ((ok = source.load(db, cfg)) != "" || !source.allowed())
	{
		if (ok == "") ok = "not in a mailbox"; // fake
		error(_("Database mbox source pull failure"), ok,
		      _("The specified message does not exist."));
	}
	
	if (source.deleted())
		error(_("Database mbox source pull failure"), "not found",
		      _("The specified message has been deleted."));
	
	DwMessage message;
	if ((ok = source.message(cfg.dbdir, message)) != "")
		error(_("MBox read failure"), ok,
		      _("Unable to open message in the mailbox. "
		        "Perhaps it has been deleted or moved?"));
	
	Cache cache(cfg, "mbox", param, req.ext);
	cache.o << message.AsString().c_str();
	
	return 0;
}
