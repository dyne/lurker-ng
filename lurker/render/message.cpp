/*  $Id: message.cpp,v 1.4 2003-04-25 16:38:19 terpstra Exp $
 *  
 *  message.cpp - Handle a message/ command
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

#include <mimelib/headers.h>
#include <mimelib/message.h>
#include <mimelib/datetime.h>
#include <mimelib/addrlist.h>
#include <mimelib/address.h>
#include <mimelib/group.h>
#include <mimelib/mboxlist.h>
#include <mimelib/mailbox.h>
#include <mimelib/text.h>
#include <mimelib/enum.h>
#include <mimelib/body.h>
#include <mimelib/bodypart.h>
#include <mimelib/utility.h>
#include <mimelib/param.h>

#include <CharsetEscape.h>
#include <XmlEscape.h>
#include <Keys.h>

#if __GNUC__ == 2
#include <strstream>
#else
#include <sstream>
#endif

#include "commands.h"
#include "Threading.h"
#include "Keyword.h"
#include "Cache.h"

void    art_scan(const unsigned char** s, const unsigned char** e);
void    url_scan(const unsigned char** s, const unsigned char** e);
void mailto_scan(const unsigned char** s, const unsigned char** e);
void  quote_scan(const unsigned char** s, const unsigned char** e);

#define us(x) ((const unsigned char**)x)

void my_service_mailto(
	ostream&		o,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, mailto_scan(us(&sp), us(&ep)), sp)
	{
		o << xmlEscape << string(s, sp-s);
		o << "<mailto>";
		o << xmlEscape << string(sp, ep-sp);
		o << "</mailto>";
		
		s = ep;
	}
	
	o << xmlEscape << string(s, e-s);
}

void my_service_url(
	ostream&		o,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, url_scan(us(&sp), us(&ep)), sp)
	{
		my_service_mailto(o, s, sp-s);
		o << "<url>";
		o << xmlEscape << string(sp, ep-sp);
		o << "</url>";
		
		s = ep;
	}
	
	my_service_mailto(o, s, e-s);
}

void my_service_pic(
	ostream&		o,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, art_scan(us(&sp), us(&ep)), sp)
	{
		my_service_url(o, s, sp-s);
		o << "<art>";
		my_service_url(o, sp, ep-sp);
		o << "</art>";
		
		s = ep;
	}
	
	my_service_url(o, s, e-s);
}

void my_service_quote(
	ostream&		o,
	const char*		buf, 
	long			len)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, quote_scan(us(&sp), us(&ep)), sp)
	{
		my_service_pic(o, s, sp-s);
		o << "<quote>";
		my_service_pic(o, sp, ep-sp);
		o << "</quote>";
		
		s = ep;
	}
	
	my_service_pic(o, s, e-s);
}

void message_display(ostream& o, DwEntity& e)
{
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
	
	string charset;
	if (e.Headers().HasContentType())
	{
		DwParameter* p = e.Headers().ContentType().FirstParameter();
		while (p)
		{
			if (p->Attribute() == "charset")
				charset = p->Value().c_str();
			p = p->Next();
		}
	}
	
	CharsetEscape decode(charset.c_str());
	
#if __GNUC__ == 2
	strstream utf8;
#else
	std::stringstream utf8;
#endif
	
	decode.write(utf8, out.c_str(), out.length());
	out.clear();
	
#if __GNUC__ == 2
	my_service_quote(o, utf8.str(), utf8.rdbuf()->pcount());
#else
	my_service_quote(o, utf8.str().c_str(), utf8.str().length());
#endif
}

void message_build(ostream& o, DwEntity& e, long& x)
{
	// We are the requested entity.
	++x;
	
	string type = "text/plain";
	string name = "";
	
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& mt = e.Headers().ContentType();
		type = (mt.TypeStr() + "/" + mt.SubtypeStr()).c_str();
		name = mt.Name().c_str();
	}
	
	o << "<mime id=\"" << x << "\" type=\"" << type << "\"";
	if (name != "") o << " name=\"" << xmlEscape << name << "\"";
	o << ">";
	
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& t = e.Headers().ContentType();
		switch (t.Type())
		{
		case DwMime::kTypeMessage:
			if (e.Body().Message())
				message_build(o, *e.Body().Message(), x);
			break;
		
		case DwMime::kTypeMultipart:
			for (DwBodyPart* p = e.Body().FirstBodyPart(); p != 0; p = p->Next())
				message_build(o, *p, x);
			break;
		
		case DwMime::kTypeText:
			message_display(o, e);
			break;
		}
	}
	else
	{
		message_display(o, e);
	}
	
	o << "</mime>";
}

void message_format_address(ostream& o, DwAddress* a, const string& charset)
{
	for (; a != 0; a = a->Next())
	{
		if (a->IsGroup())
		{
			DwGroup* g = dynamic_cast<DwGroup*>(a);
			if (g)
				message_format_address(
					o, 
					g->MailboxList().FirstMailbox(), 
					charset);
		}
		else
		{
			DwMailbox* m = dynamic_cast<DwMailbox*>(a);
			if (m)
			{
				string name = m->FullName().c_str();
				if (name[0] == '"' && name.length() > 2)
					name = name.substr(1, name.length()-2);
				if (name == "")
					name = m->LocalPart().c_str();
				
				// Deal with the horror
				name = decode_header(name, charset.c_str());
				
				o << "<email name=\"" << xmlEscape 
				  << name << "\" address=\"" << xmlEscape
				  << m->LocalPart().c_str() << "@" << xmlEscape
				  << m->Domain().c_str() << "\"/>";
			}
		}
	}
}

struct MBox
{
	List	cfg;
	Summary prev;
	Summary next;
	
	MBox() { }
	MBox(const List& cfg_) : cfg(cfg_) { }
	
	string load(ESort::Reader* db, const MessageId& rel);
};

string MBox::load(ESort::Reader* db, const MessageId& rel)
{
	string ok;
	vector<Summary> sum;
	
	Keyword n(db, Forward, LU_KEYWORD_LIST + cfg.mbox, rel);
	if ((ok = n.pull(2, sum)) != "") return ok;
	
	if (sum.size() < 1 || sum[0].id() != rel)
		return "Relative message does not exist";
	
	if (sum.size() >= 2)
	{
		next = sum[1];
		if ((ok = next.load(db)) != "") return ok;
	}
	
	sum.clear();
	
	Keyword p(db, Backward, LU_KEYWORD_LIST + cfg.mbox, rel);
	if ((ok = p.pull(1, sum)) != "") return ok;
	
	if (sum.size() >= 1)
	{
		prev = sum[0];
		if ((ok = prev.load(db)) != "") return ok;
	}
	
	return "";
}

int handle_message(const Config& cfg, ESort::Reader* db, const string& param)
{
	MessageId id(param.c_str());
	
	if (id.timestamp() == 0)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Bad request"), param,
			_("The given parameter was not of the correct format. "
			  "A message request must be formatted like: "
			  "message/YYYYMMDD.HHMMSS.hashcode.xml"));
		return 1;
	}
	
	string ok;
	
	Summary source(id);
	if ((ok = source.load(db)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database message source pull failure"), ok,
			_("The specified message does not exist."));
		return 1;
	}
	
	Threading::Key spot;
	Threading thread;
	if ((ok = thread.load(db, source, spot)) != "" ||
	    (ok = thread.draw_snippet(db, spot)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database message tree load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	Summary& thread_prev = thread.getSummary(thread.prev(spot));
	Summary& thread_next = thread.getSummary(thread.next(spot));
	
	if ((!thread_prev.loaded() && (ok = thread_prev.load(db)) != "") ||
	    (!thread_next.loaded() && (ok = thread_next.load(db)) != ""))
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database message prev/next load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
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
	
	map<string, Summary> followups; // these are all followups NOT in the tree
	if (message.Headers().HasMessageId())
	{
		vector<string> mids = extract_message_ids(
			message.Headers().MessageId().AsString().c_str());
		
		vector<Summary> sums;
		vector<Summary>::iterator sum;
		vector<string>::iterator mid;
		
		for (mid = mids.begin(); mid != mids.end(); ++mid)
		{
			// cout << "MID: " << *mid << "\n";
			Keyword k(db, Forward, LU_KEYWORD_REPLY_TO + *mid);
			if ((ok = k.pull(1000, sums)) != "")
				break;
		}
		
		if (ok == "")
		for (sum = sums.begin(); sum != sums.end(); ++sum)
		{
			// cout << "SUM: " << *sum << "\n";
			string hash = sum->id().hash();
			if (thread.hasMessage(hash)) continue;
			if (followups.find(hash) != followups.end()) continue;
			followups[hash] = *sum;
			if ((ok = followups[hash].load(db)) != "")
				break;
		}
	}
	if (ok != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database followups load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	map<string, Summary> repliesTo; // what messages this one replies to
	if (message.Headers().HasInReplyTo())
	{
		vector<string> mids = extract_message_ids(
			message.Headers().InReplyTo().AsString().c_str());
		
		vector<Summary> sums;
		vector<Summary>::iterator sum;
		vector<string>::iterator mid;
		
		for (mid = mids.begin(); mid != mids.end(); ++mid)
		{
			Keyword k(db, Forward, LU_KEYWORD_MESSAGE_ID + *mid);
			if ((ok = k.pull(1000, sums)) != "")
				break;
		}
		
		if (ok == "")
		for (sum = sums.begin(); sum != sums.end(); ++sum)
		{
			string hash = sum->id().hash();
			if (repliesTo.find(hash) != repliesTo.end()) continue;
			repliesTo[hash] = *sum;
			if ((ok = repliesTo[hash].load(db)) != "")
				break;
		}
	}
	if (ok != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database replies load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	vector<MBox> boxes;
	set<string>::iterator mbox;
	for (mbox = source.mboxs().begin(); mbox != source.mboxs().end(); ++mbox)
	{
		Config::Lists::const_iterator j = cfg.lists.find(*mbox);
		if (j == cfg.lists.end()) continue;
		
		boxes.push_back(MBox(j->second));
		if ((ok = boxes.back().load(db, id)) != "") break;
	}
	if (ok != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database list links load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	Cache cache(cfg, "message", param);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/render.xsl\"?>\n"
		<< "<message>\n"
		<< " " << cfg << "\n"
		<< " " << source << "\n";
	
	vector<MBox>::iterator m;
	for (m = boxes.begin(); m != boxes.end(); ++m)
	{
		cache.o	<< " <mbox>\n"
			<< "  " << m->cfg << "\n";
		
		if (m->next.id().timestamp() != 0)
			cache.o << "  <next>" << m->next << "</next>\n";
		if (m->prev.id().timestamp() != 0)
			cache.o << "  <prev>" << m->prev << "</prev>\n";
		
		cache.o	<< " </mbox>\n";
	}
	
	// Find the charset for the overall message, if any.
	string charset;
	if (message.Headers().HasContentType())
	{
		DwParameter* p = message.Headers().ContentType().FirstParameter();
		while (p)
		{
			if (p->Attribute() == "charset")
				charset = p->Value().c_str();
			p = p->Next();
		}
	}
	
	// if (message.hasHeaders() && 
	if (message.Headers().HasTo())
	{
		cache.o	<< " <to>";
		message_format_address(
			cache.o, 
			message.Headers().To().FirstAddress(),
			charset);
		cache.o	<< "</to>\n";
	}
	
	// if (message.hasHeaders() && 
	if (message.Headers().HasCc())
	{
		cache.o	<< " <cc>";
		message_format_address(
			cache.o, 
			message.Headers().Cc().FirstAddress(),
			charset);
		cache.o	<< "</cc>\n";
	}
	
	// Output the snippet
	cache.o << " <threading>\n  <snippet>\n   <tree>";
	int head = -2; // magic, don't ask.
	thread.draw_snippet_row(cache.o, &head, 0, spot);
	cache.o << "</tree>\n   <tree>";
	thread.draw_snippet_row(cache.o, &head, 1, spot);
	cache.o << "</tree>\n   <tree>";
	thread.draw_snippet_row(cache.o, &head, 2, spot);
	cache.o << "</tree>\n";
	
	// Output all summaries needed by the snippet
	Threading::Key i;
	for (i = 0; i < thread.size(); ++i)
	{
		Summary& sum = thread.getSummary(i);
		if (sum.loaded())
			cache.o << "   " << sum << "\n";
	}
	
	cache.o << "  </snippet>\n";
	if (thread_prev.id() != id)
		cache.o << "  <prev>" << thread_prev << "</prev>\n";
	if (thread_next.id() != id)
		cache.o << "  <next>" << thread_next << "</next>\n";
	
	if (!repliesTo.empty())
	{
		cache.o << "  <inreplyto>\n";
		
		map<string, Summary>::iterator irt;
		for (irt = repliesTo.begin(); irt != repliesTo.end(); ++irt)
			cache.o << "   " << irt->second << "\n";
		
		cache.o << "  </inreplyto>\n";
	}
	
	if (!followups.empty())
	{
		cache.o << "  <drift>\n";
		
		map<string, Summary>::iterator drift;
		for (drift = followups.begin(); drift != followups.end(); ++drift)
			cache.o << "   " << drift->second << "\n";
		
		cache.o << "  </drift>\n";
	}
	
#if 0
	// These are already included; don't print twice
	set<Summary> replies = thread.replies(spot);
	if (!replies.empty())
	{
		cache.o << "  <replies>\n";
		
		set<Summary>::iterator rep;
		for (rep = replies.begin(); rep != replies.end(); ++rep)
			cache.o << "   " << *rep << "\n";
		
		cache.o << "  </replies>\n";
		
	}
#endif
	
	cache.o << " </threading>\n";
	
	long aid = 0;
	message_build(cache.o, message, aid);
	
	cache.o	<< "</message>\n";
	
	return 0;
}
