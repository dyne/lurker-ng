/*  $Id: attach.cpp,v 1.8 2004-01-06 20:02:05 terpstra Exp $
 *  
 *  attach.cpp - Handle a attach/ command
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

#define _XOPEN_SOURCE 500
#define _FILE_OFFSET_BITS 64

#include <mimelib/message.h>
#include <mimelib/headers.h>
#include <mimelib/bodypart.h>
#include <mimelib/body.h>
#include <mimelib/enum.h>
#include <mimelib/mediatyp.h>
#include <mimelib/utility.h>

#include "commands.h"
#include "Summary.h"

#include <iostream>

using std::cout;

int attach_format_error(const string& param)
{
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << error(_("Bad request"), param,
		_("The given parameter was not of the correct format. "
		  "An attach request must be formatted like: "
		  "attach/id@YYYYMMDD.HHMMSS.hashcode.xml where id "
		  "is the number of the attachment."));
	return 1;
}

DwEntity& attach_find(DwEntity& e, long& x)
{
	// We are the requested entity.
	if (--x == 0) return e;
	
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& t = e.Headers().ContentType();
		switch (t.Type())
		{
		case DwMime::kTypeMessage:
			if (e.Body().Message())
				return attach_find(*e.Body().Message(), x);
			break;
		
		case DwMime::kTypeMultipart:
			for (DwBodyPart* p = e.Body().FirstBodyPart(); p != 0; p = p->Next())
			{
				DwEntity& o = attach_find(*p, x);
				if (x == 0) return o;
			}
			break;
		}
	}
	
	return e;
}

string unfold_header(const char* hdr)
{
	string out;
	
	while (*hdr != 0)
	{
		if (*hdr == '\r' || *hdr == '\n' || *hdr == '\t')
			out += ' ';
		else	out += *hdr;
		++hdr;
	}
	
	return out;
}

int handle_attach(const Config& cfg, ESort::Reader* db, const string& param)
{
	string::size_type o = param.find('@');
	if (o == string::npos || param.length() - o < 1 + 15 + 4)
		return attach_format_error(param);
	
	long n = atol(param.c_str());
	MessageId id(param.c_str()+o+1);
	
	if (n <= 0 || id.timestamp() == 0)
		return attach_format_error(param);
	
	string ok;
	
	Summary source(id);
	if ((ok = source.load(db, cfg)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database attach source pull failure"), ok,
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
	
	DwEntity& e = attach_find(message, n);
	
	// Cannot cache an attachment because they have strange content-type
	
	cout << "Status: 200 OK\r\n"
	     << "Content-Type: ";
	
	// if (e.hasHeaders() &&
	if (e.Headers().HasContentType())
	{
		cout << unfold_header(
			e.Headers().ContentType().AsString().c_str());
	}
	else	cout <<	"text/plain";
	
	cout << "\r\n\r\n";
	
	DwString out;
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentTransferEncoding())
	{
		switch (e.Headers().ContentTransferEncoding().AsEnum())
		{
		case DwMime::kCteQuotedPrintable:
			DwDecodeQuotedPrintable(e.Body().AsString(), out);
			break;
			
		case DwMime::kCteBase64:
			DwDecodeBase64(e.Body().AsString(), out);
			break;
		
		case DwMime::kCteNull:
		case DwMime::kCteUnknown:
		case DwMime::kCte7bit:
		case DwMime::kCte8bit:
		case DwMime::kCteBinary:
			out = e.Body().AsString();
			break;
		}
		
	}
	else
	{
		out = e.Body().AsString();
	}
	
	cout.write(out.c_str(), out.length());
	return 0;
}
