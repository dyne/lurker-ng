/*  $Id: message.cpp,v 1.38 2004-08-24 21:52:39 terpstra Exp $
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
#include <mimelib/disptype.h>
#include <mimelib/param.h>

#include <CharsetEscape.h>
#include <XmlEscape.h>
#include <Keys.h>

#include <fstream>
#include <cstdio>
#include <cstring>
#include <cerrno>

#include <unistd.h>
#include <sys/wait.h>

#include "commands.h"
#include "Threading.h"
#include "Search.h"
#include "Cache.h"

#define OLD_PGP_HEADER	"-----BEGIN PGP SIGNED MESSAGE-----\n"
#define OLD_PGP_DIVIDER	"-----BEGIN PGP SIGNATURE-----\n" 
#define OLD_PGP_ENDER	"-----END PGP SIGNATURE-----\n"

void    art_scan(const unsigned char** s, const unsigned char** e);
void    url_scan(const unsigned char** s, const unsigned char** e);
void mailto_scan(const unsigned char** s, const unsigned char** e);
void  quote_scan(const unsigned char** s, const unsigned char** e);

#define us(x) ((const unsigned char**)x)


void my_service_mailto(
	ostream&		o,
	const char*		buf, 
	long			len,
	const Config&		cfg)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, mailto_scan(us(&sp), us(&ep)), sp)
	{
		o << xmlEscape << string(s, sp-s);
		if (cfg.hide_email)
		{
			string addr(sp, ep-sp);
			string::size_type l = addr.find('@');
			if (l != string::npos) addr.resize(l);
			o << xmlEscape << addr << "@???";
		}
		else
		{
			o << "<mailto>";
			o << xmlEscape << string(sp, ep-sp);
			o << "</mailto>";
		}
		
		s = ep;
	}
	
	o << xmlEscape << string(s, e-s);
}

void my_service_url(
	ostream&		o,
	const char*		buf, 
	long			len,
	const Config&		cfg)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, url_scan(us(&sp), us(&ep)), sp)
	{
		my_service_mailto(o, s, sp-s, cfg);
		o << "<url>";
		o << xmlEscape << string(sp, ep-sp);
		o << "</url>";
		
		s = ep;
	}
	
	my_service_mailto(o, s, e-s, cfg);
}

void my_service_pic(
	ostream&		o,
	const char*		buf, 
	long			len,
	const Config&		cfg)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, art_scan(us(&sp), us(&ep)), sp)
	{
		my_service_url(o, s, sp-s, cfg);
		o << "<art>";
		my_service_url(o, sp, ep-sp, cfg);
		o << "</art>";
		
		s = ep;
	}
	
	my_service_url(o, s, e-s, cfg);
}

void my_service_quote(
	ostream&		o,
	const char*		buf, 
	long			len,
	const Config&		cfg)
{
	const char* s = buf;
	const char* e = buf+len;
	
	const char* sp;
	const char* ep;
	
	while (sp = s, ep = e, quote_scan(us(&sp), us(&ep)), sp)
	{
		my_service_pic(o, s, sp-s, cfg);
		o << "<quote>";
		my_service_pic(o, sp, ep-sp, cfg);
		o << "</quote>";
		
		s = ep;
	}
	
	my_service_pic(o, s, e-s, cfg);
}

void find_and_replace(string& target, const string& token, const string& value)
{
	string::size_type x = 0;
	while ((x = target.find(token, x)) != string::npos)
		target.replace(x, token.length(), value);
}

const Config* pgp_config = 0;
string pgp_name_prefix;
int pgp_part = 0;

string pgp_tmpfile(const string& type)
{
	char buf[10];
	sprintf(buf, "%d", pgp_part);
	return string("../attach/") + buf + "@" + pgp_name_prefix + "." + type;
}

void pgp_writefile(ostream& o, const DwString& data)
{
	size_t s, e;
	
	s = 0;
	while (1) // signed data must have CRLF endcoding
	{
		e = data.find_first_of("\r\n", s);
		if (e == DwString::npos) break;
		
		o.write(data.c_str() + s, e - s);
		if (data[e] == '\n') o << "\r\n"; 
		s = e+1;
	}
	
	o.write(data.c_str() + s, data.length() - s);
}

void run_pgp(ostream& o, string& command)
{
	string photo = pgp_tmpfile("photo");
	find_and_replace(command, "%p", photo);
	
	string details;
	int status;
	
	FILE* pgp = popen(command.c_str(), "r");
	if (pgp != 0)
	{
		char buf[1024];
		size_t got;
		while ((got = fread(buf, 1, sizeof(buf), pgp)) > 0)
		{
			details.append(buf, got);
			if (got != sizeof(buf)) break;
		}
		
		status = pclose(pgp);
		if (WIFEXITED(status))
		{
			status = WEXITSTATUS(status);
		}
		else
		{
			details += "\n" + command + " exited abnormally";
			status = 2;
		}
	}
	else
	{
		details = command + " failed with " + strerror(errno);
		status = 2;
	}
	
	o << "<signed ok=\"";
	
	if      (status == 0) o << "yes";
	else if (status == 1) o << "no";
	else                  o << "unknown";
	
	o << "\">"
	  << "<details>" << xmlEscape << details << "</details>";
	
	if (access(photo.c_str(), R_OK) == 0)
	{
		o << "<photo>" << photo << "</photo>";
	}
}

bool handle_signed_inline(ostream& o, const DwString& s)
{
	string command = pgp_config->pgpv_inline;
	if (command == "off") return false;
	
	string cleartext = pgp_tmpfile("cleartext");
	find_and_replace(command, "%b", cleartext);
	
	if (1)
	{ // create the cleartext
		std::ofstream body(cleartext.c_str());
		pgp_writefile(body, s);
	}
	
	run_pgp(o, command);
	return true;
}

bool handle_signed_mime(ostream& o, DwEntity& e)
{
	// rfc 1847 says we have 2 bodyparts:
	//  1. the original data
	//  2. the signature
	
	DwBodyPart* body = e.Body().FirstBodyPart();
	if (!body) return false;
	DwBodyPart* sig = body->Next();
	if (!sig) return false;
	if (sig->Next() != 0) return false;
	
	// signature has no type
	if (!sig->Headers().HasContentType() ||
	    sig->Headers().ContentType().Type() != DwMime::kTypeApplication)
		return false;
	
	DwString st = sig->Headers().ContentType().SubtypeStr();
	st.ConvertToLowerCase();
	if (st != "pgp-signature")
		return false;
	
	string command = pgp_config->pgpv_mime;
	if (command == "off") return false;
	
	string cleartext = pgp_tmpfile("cleartext");
	string signature = pgp_tmpfile("signature");
	find_and_replace(command, "%b", cleartext);
	find_and_replace(command, "%s", signature);
	
	if (1)
	{ // create the cleartext
		std::ofstream bodyf(cleartext.c_str());
		pgp_writefile(bodyf, body->AsString());
	}
	
	if (1)
	{ // create the signature
		std::ofstream sigf(signature.c_str());
		pgp_writefile(sigf, sig->Body().AsString());
	}
	
	run_pgp(o, command);
	return true;
}

void process_text(ostream& o, bool html, const string& charset, const DwString& out, const Config& cfg)
{
	CharsetEscape decode(charset.c_str());
	string utf8 = decode.write(out.c_str(), out.length());
	
	if (!decode.valid())
	{
		utf8 = "<-- Warning: charset '" + charset + "' is not supported -->\n\n"
		     + utf8;
	}
	
	if (html)
	{
		string::size_type start, end;
		
		start = 0;
		while ((end = utf8.find('<', start)) != string::npos)
		{
			my_service_quote(o, utf8.c_str()+start, end-start, cfg);
			start = utf8.find('>', end);
			
			if (start == string::npos) break;
			++start;
		}
		
		// deal with half-open tag at end of input
		if (start != string::npos)
			my_service_quote(o, utf8.c_str()+start, utf8.length()-start, cfg);
	}
	else
	{
		my_service_quote(o, utf8.c_str(), utf8.length(), cfg);
	}
}

void message_display(ostream& o, DwEntity& e, const string& charset, bool html, const Config& cfg)
{
	// Oldschool pgp usually works by invoking a helper program which
	// cannot control how the email client then encodes the signed data.
	// Hence we nede to decode the transfer-encoding for verification
	// to get back what the helper program probably gave the MUA.
	
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
	
	// We do NOT convert the charset because the user probably signed
	// the text in the charset as which it was delivered. If it wasn't,
	// we wouldn't be able to help anyways because we don't know what 
	// to convert to.
	
	size_t pgp_last, pgp_header, pgp_divider, pgp_ender;
	for (pgp_last = 0;
	     ((pgp_header  = out.find(OLD_PGP_HEADER,  pgp_last))   != DwString::npos) &&
	     ((pgp_divider = out.find(OLD_PGP_DIVIDER, pgp_header)) != DwString::npos) &&
	     ((pgp_ender   = out.find(OLD_PGP_ENDER,   pgp_divider))!= DwString::npos);
	     pgp_last = pgp_ender)
	{
		pgp_ender += sizeof(OLD_PGP_ENDER)-1; // include endline, not null
		
		// deal with leading text (substr is copy-free)
		process_text(o, html, charset,
			out.substr(pgp_last, pgp_header-pgp_last), cfg);
		
		bool signOpen = false;
		if (handle_signed_inline(o, 
			out.substr(pgp_header, pgp_ender-pgp_header)))
		{
			signOpen = true;
			o << "<data>";
		}
		
		// skip the header + hash line + blank line
		// (safe b/c we have 3 \n s for sure)
		pgp_header += sizeof(OLD_PGP_HEADER)-1; // eol, !null
		pgp_header = out.find('\n', pgp_header) + 1;
		pgp_header = out.find('\n', pgp_header) + 1;
		
		if (pgp_header < pgp_divider)
		{
			// signed text
			process_text(o, html, charset,
				out.substr(pgp_header, pgp_divider-pgp_header), cfg);
		}
		
		if (signOpen)
		{
			o << "</data></signed>";
		}
	}
	// trailing text
	process_text(o, html, charset,
		out.substr(pgp_last, out.length()-pgp_last), cfg);
}

// this will only output mime information if the dump is false
void message_build(ostream& o, DwEntity& e, 
	const string& parentCharset, bool dump, long& x, const Config& cfg)
{
	// We are the requested entity.
	pgp_part = ++x;
	
	string charset = parentCharset;
	string type = "text/plain";
	string name = "";
	
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& mt = e.Headers().ContentType();
		
		DwString ftype = mt.TypeStr() + "/" + mt.SubtypeStr();
		ftype.ConvertToLowerCase();
		type = ftype.c_str();
		name = mt.Name().c_str();
		
		for (DwParameter* p = mt.FirstParameter(); p; p = p->Next())
		{
			DwString attr = p->Attribute();
			attr.ConvertToLowerCase(); // case insens
			if (attr == "charset") charset = p->Value().c_str();
		}
	}
	
	if (e.Headers().HasContentDisposition())
	{
		DwDispositionType& dt = e.Headers().ContentDisposition();
		if (dt.Filename() != "")
			name = dt.Filename().c_str();
	}
	
	// The question is: which charset affects the headers?
	// I claim that the parent charset does - this is being friendly
	// anyways since one shouldn't have non us-ascii in the headers
	CharsetEscape ches(parentCharset.c_str());
	o << "<mime id=\"" << x << "\" type=\"" << xmlEscape << ches.write(type) << "\"";
	if (name != "") o << " name=\"" << xmlEscape << ches.write(name) << "\"";
	o << ">";
	
	bool signedopen = false;
	
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& t = e.Headers().ContentType();
		switch (t.Type())
		{
		case DwMime::kTypeMessage:
			if (e.Body().Message())
				message_build(o, *e.Body().Message(), charset, dump, x, cfg);
			break;
		
		case DwMime::kTypeMultipart:
			if (1)
			{ // scope in the string
				DwString s = t.SubtypeStr();
				s.ConvertToLowerCase();
				if (s == "signed")
				{	// verify the signature
					signedopen = handle_signed_mime(o, e);
				}
			}
			
			// first body part is the signed data
			if (signedopen)	o << "<data>";
				
			for (DwBodyPart* p = e.Body().FirstBodyPart(); p != 0; p = p->Next())
			{
				bool plain = false;
				if (p->Headers().HasContentType())
				{
					DwMediaType& mt = p->Headers().ContentType();
					
					plain =	mt.Type()    == DwMime::kTypeText &&
						mt.Subtype() == DwMime::kSubtypePlain;
				}
				
				if (t.Subtype() != DwMime::kSubtypeAlternative || 
				    p->Next() == 0 || plain)
				{	// display all parts, or plain, or last
					message_build(o, *p, charset, dump, x, cfg);
					
					// if we printed something, we are done
					if (t.Subtype() == DwMime::kSubtypeAlternative)
						dump = false;
				}
				else
				{
					message_build(o, *p, charset, false, x, cfg);
				}
				
				if (signedopen)
				{	// done the first section which was signed
					o << "</data></signed>";
					signedopen = false;
				}
			}
			break;
		
		case DwMime::kTypeText:
			if (dump) message_display(o, e, charset, t.Subtype() == DwMime::kSubtypeHtml, cfg);
			break;
		}
	}
	else
	{
		if (dump) message_display(o, e, charset, false, cfg);
	}
	
	o << "</mime>";
}

void message_format_address(ostream& o, DwAddress* a, const string& charset, const Config& cfg)
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
					charset, cfg);
		}
		else
		{
			DwMailbox* m = dynamic_cast<DwMailbox*>(a);
			if (m)
			{
				string name = m->FullName().c_str();
				if (name.length() >= 2 && name[0] == '"')
					name = name.substr(1, name.length()-2);
				if (name == "")
					name = m->LocalPart().c_str();
				
				// Deal with the horror
				name = decode_header(name, charset.c_str());
				if (name.length() >= 2 && name[0] == '"')
					name = name.substr(1, name.length()-2);
				
				DwString addr = m->LocalPart() + "@" + m->Domain();
				for (size_t i = 0; i < addr.length(); ++i)
				{
					if (addr[i] <= 0x20 || addr[i] >= 0x7f)
					{	// fucked up address
						addr = "";
						break;
					}
				}
				if (addr.length() > 128) addr = "";
				
				o << "<email";
				if (name != "")
					o << " name=\"" << xmlEscape 
					  << name << "\"";
				if (addr != "" && !cfg.hide_email)
					o << " address=\"" << xmlEscape
					  << addr.c_str() << "\"";
				o << "/>";
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
	
	string load(ESort::Reader* db, const MessageId& rel, const Config& cfg);
};

string MBox::load(ESort::Reader* db, const MessageId& rel, const Config& conf)
{
	string ok;
	vector<Summary> sum;
	
	Search n(conf, db, Forward, rel);
	n.keyword(LU_KEYWORD_LIST + cfg.mbox);
	
	if (!n.pull(2, sum)) return "Pulling next two failed";
	
	if (sum.size() < 1 || sum[0].id() != rel)
		return "Relative message does not exist";
	
	if (sum.size() >= 2)
	{
		next = sum[1];
		if ((ok = next.load(db, conf)) != "") return ok;
	}
	
	sum.clear();
	
	Search p(conf, db, Backward, rel);
	p.keyword(LU_KEYWORD_LIST + cfg.mbox);
	
	if (!p.pull(1, sum)) return "Pulling previous failed";
	
	if (sum.size() >= 1)
	{
		prev = sum[0];
		if ((ok = prev.load(db, conf)) != "") return ok;
	}
	
	return "";
}

int handle_message(const Config& cfg, ESort::Reader* db, const string& param)
{
	Request req = parse_request(param);
	cfg.options = req.options;
	
	if (!MessageId::is_full(req.options.c_str()) ||
	    req.options.length() != MessageId::full_len)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Bad request"), param,
			_("The given parameter was not of the correct format. "
			  "A message request must be formatted like: "
			  "message/YYYYMMDD.HHMMSS.hashcode.lc.xml"));
		return 1;
	}
	
	MessageId id(req.options.c_str());
	
	pgp_config = &cfg; // hackish
	pgp_name_prefix = id.serialize();
	
	string ok;
	
	Summary source(id);
	if ((ok = source.load(db, cfg)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database message source pull failure"), ok,
			_("The specified message does not exist."));
		return 1;
	}
	
	// No keywords means implicitly search for everything
	vector<Summary> dead;
	Search e(cfg, db, Forward, id);
	if (!e.pull(1, dead) || dead.empty() || dead[0].id() != id)
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database message source pull failure"), "not found",
			_("The specified message has been deleted."));
		return 1;
	}
	
	Threading::Key spot;
	Threading thread;
	if ((ok = thread.load(db, source, spot)) != "" ||
	    (ok = thread.draw_snippet(db, spot, cfg)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database message tree load failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	Summary thread_prev;
	Summary thread_next;
	
	if ((ok = thread.findprev(spot, db, cfg, thread_prev)) != "" ||
	    (ok = thread.findnext(spot, db, cfg, thread_next)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Thread prev/next load failure"), ok,
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
			Search k(cfg, db, Forward);
			k.keyword(LU_KEYWORD_REPLY_TO + *mid);
			
			if (!k.pull(1000, sums))
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
			if ((ok = followups[hash].load(db, cfg)) != "")
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
			Search k(cfg, db, Forward);
			k.keyword(LU_KEYWORD_MESSAGE_ID + *mid);
			
			if (!k.pull(1000, sums))
				break;
		}
		
		if (ok == "")
		for (sum = sums.begin(); sum != sums.end(); ++sum)
		{
			string hash = sum->id().hash();
			// only things not in the tree
			if (thread.hasMessage(hash)) continue;
			if (repliesTo.find(hash) != repliesTo.end()) continue;
			repliesTo[hash] = *sum;
			if ((ok = repliesTo[hash].load(db, cfg)) != "")
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
		if ((ok = boxes.back().load(db, id, cfg)) != "") break;
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
	
	Cache cache(cfg, "message", param, req.ext);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/message.xsl\"?>\n"
		<< "<message xml:lang=\"" << req.language << "\">\n"
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
			charset, cfg);
		cache.o	<< "</to>\n";
	}
	
	// if (message.hasHeaders() && 
	if (message.Headers().HasCc())
	{
		cache.o	<< " <cc>";
		message_format_address(
			cache.o, 
			message.Headers().Cc().FirstAddress(),
			charset, cfg);
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
	// default charset is ISO-8859-1
	message_build(cache.o, message, "ISO-8859-1", true, aid, cfg);
	
	cache.o	<< "</message>\n";
	
	return 0;
}
