/*  $Id: Index.cpp,v 1.25 2003-06-23 14:38:41 terpstra Exp $
 *  
 *  index.cpp - Insert all the keywords from the given email
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
#include <mimelib/datetime.h>
#include <mimelib/addrlist.h>
#include <mimelib/address.h>
#include <mimelib/group.h>
#include <mimelib/mboxlist.h>
#include <mimelib/mailbox.h>
#include <mimelib/text.h>
#include <mimelib/param.h>
#include <mimelib/enum.h>
#include <mimelib/body.h>
#include <mimelib/bodypart.h>
#include <mimelib/utility.h>

#include <CharsetEscape.h>
#include <Keys.h>
#include <md5.h>

#include "Index.h"

#include <string>
#include <vector>
#include <iostream>

#include <unistd.h>
#include <iconv.h>
#include <cerrno>

using namespace std;

#define MAX_MESSAGE_ID	80

void utf8Truncate(string& str, string::size_type len)
{
	if (str.length() < len) return;
	
	// look for nasty utf-8 stuff that's dangling and crop it
	while (len && ((unsigned char)str[len-1]) >= 0x80 && 
	              ((unsigned char)str[len-1]) <= 0xBF)
		--len;
	// now rewind off potential utf-8 start bytes
	while (len && ((unsigned char)str[len-1]) >= 0xC0)
		--len;
	
	// len is now at the end of a complete multi-byte element or ascii
	
	str.resize(len);
}

// first = address, second = name
pair<string, string> pickAddress(DwAddress* a, const char* charset)
{
	for (; a != 0; a = a->Next())
	{
		if (a->IsGroup())
		{
			DwGroup* g = dynamic_cast<DwGroup*>(a);
			if (g)
			{
				pair<string, string> out = 
					pickAddress(
						g->MailboxList().FirstMailbox(),
						charset);
				if (out.first != "") return out;
			}
		}
		else
		{
			DwMailbox* m = dynamic_cast<DwMailbox*>(a);
			if (m)
			{
				string name = m->FullName().c_str();
				name = decode_header(name, charset);
				DwString addr = m->LocalPart() + "@" + m->Domain();
				
				// fucked address? (one cannot safely cut this)
				if (addr.length() > 128 || 
				    m->LocalPart() == "" || m->Domain() == "")
				{
					addr = "";
				}
				
				for (size_t i = 0; i < addr.length(); ++i)
				{
					if (addr[i] <= 0x20 || addr[i] >= 0x7f)
					{	// fucked up address
						addr = "";
						break;
					}
				}
				
				// prune any optional quotes
				if (name.length() >= 2 && name[0] == '"')
					name = name.substr(1, name.length()-2);
				
				if (addr != "")
					return pair<string, string>(addr.c_str(), name);
			}
		}
	}
	
	return pair<string, string>("", "");
}

int Index::index_author()
{
	// one always has headers, but not always this function:
	// if (message.hasHeaders())
	
	charset = "ISO-8859-1"; // a good default as any
	
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
	
	// pickAddress only gives an author_name if it gave an author_email
	
	if (message.Headers().HasReplyTo())
	{
		pair<string, string> addr = pickAddress(
			message.Headers().ReplyTo().FirstAddress(),
			charset.c_str());
		
		author_email = addr.first;
		author_name  = addr.second;
		
		// Some evil mailing lists set reply-to the list.
		if (author_email == list.address)
		{
			author_email = "";
			author_name = "";
		}
	}
	
	// Given a reply-to that is not the list, we allow the from to
	// provide a fullname under the assumption it is the same person.
	
	if (message.Headers().HasFrom())
	{
		pair<string, string> addr = pickAddress(
			message.Headers().From().FirstMailbox(),
			charset.c_str());
		
		if (!author_email.length()) author_email = addr.first;
		if (!author_name .length()) author_name  = addr.second;
	}
	
	// ditto
	
	if (message.Headers().HasSender())
	{
		pair<string, string> addr = pickAddress(
			&message.Headers().Sender(),
			charset.c_str());
		
		if (!author_email.length()) author_email = addr.first;
		if (!author_name .length()) author_name  = addr.second;
	}
	
	utf8Truncate(author_name, 100);
	//  - nothing longer than 128 could get here (from above)
	//  - one can never safely truncate an email address
	// utf8Truncate(author_email, 100);
	
	return 0;
}

// Doesn't vary with charset
inline bool lu_isspace(char x)
{
	return x == ' ' || x == '\n' || x == '\r' || x == '\t';
}

void build_message_hash(const char* str, unsigned char* hash)
{
	MD5Context ctx;
	
	MD5Init(&ctx);
	MD5Update(&ctx, (const unsigned char*)str, strlen(str));
	
	unsigned char buf[16];
	MD5Final(buf, &ctx);
	
	hash[0] = buf[0] ^ buf[4] ^ buf[ 8] ^ buf[12];
	hash[1] = buf[1] ^ buf[5] ^ buf[ 9] ^ buf[13];
	hash[2] = buf[2] ^ buf[6] ^ buf[10] ^ buf[14];
	hash[3] = buf[3] ^ buf[7] ^ buf[11] ^ buf[15];
}

int feed_writer(const char* keyword, void* arg)
{
	Index* i = (Index*)arg;
	
	string x(LU_KEYWORD);
	x += keyword;
	x += '\0';
	x += i->id.raw();
	
	return i->writer->insert(x);
}

int Index::index_id(time_t server)
{
	time_t stamp = server;
	string messageId;
	unsigned char hash[4];
	
	// if (message.hasHeaders())
	
	if (message.Headers().HasDate())
	{
		time_t user = message.Headers().Date().AsUnixTime();
		
		/* User time must be earlier; there is delivery delay!
		 * ... except by one hour due to daylight savings
		 * However, more than 3 day delivery time is unlikely.
		 */
		if ((user <= server+60*60 && server < user+3*60*60*24) ||
		    server <= 0) // server is on crack?
			stamp = user;
	}
	
	if (stamp <= 0)
	{	// this is crazy; I don't care if they agree: it's wrong
		stamp = 1; // liers all have timestamp 1970-01-01 00:00:01
	}
	
	if (message.Headers().HasMessageId())
	{
		vector<string> ids = extract_message_ids(
			message.Headers().MessageId().AsString().c_str());
		
		if (!ids.empty())
			messageId = ids.front();
	}
	
	if (messageId.length())
	{
		// Constant message-id across import, and threadable
		build_message_hash(messageId.c_str(), hash);
	}
	else if (author_email.length())
	{
		// This means no proper threading.
		// At least the message-id is constant across import.
		build_message_hash(author_email.c_str(), hash);
	}
	else
	{
		// Can't make any guarantees; just import it.
		hash[0] = random() % 256;
		hash[1] = random() % 256;
		hash[2] = random() % 256;
		hash[3] = random() % 256;
	}
	
	id = MessageId(stamp, hash);
	
	if (messageId.length() && my_keyword_digest_string(
		messageId.c_str(), messageId.length(),
		LU_KEYWORD_MESSAGE_ID, &feed_writer, this, 0) != 0)
	{
		cerr << "Failed to insert message id keyword!" << endl;
		return -1;
	}
	
	if (writer->insert(
		LU_KEYWORD +
		string(LU_KEYWORD_EVERYTHING) + 
		'\0' + 
		id.raw()) != 0)
	{
		cerr << "Failed to the any keyword!" << endl;
		return -1;
	}
	
	return 0;
}

int Index::index_summary(bool check, bool& exist)
{
	string prefix = LU_SUMMARY + id.raw();
	
	if (message.Headers().HasSubject())
	{
		subject = message.Headers().Subject().AsString().c_str();
		subject = decode_header(subject, charset.c_str());
	}
	
	if (subject == "")
		subject = "[...]";
	
	string mbox = prefix + LU_MESSAGE_MBOX + list.mbox + '\0';
	
	if (check)
	{
		// Check for existance
		auto_ptr<ESort::Walker> w(writer->seek(mbox, "", ESort::Forward));
		
		if (w->advance() == -1)
		{	// was it just eof?
			if (errno != 0) return -1;
		}
		else
		{	// if it suceeded. then ... it is already in there
			exist = true;
			return 0;
		}
	}
	
	unsigned char buf[12];
	off_t o = off;
	long l = len;
	int i;
	
	for (i = 7; i >= 0; --i)
	{
		buf[i] = (o & 0xFF);
		o >>= 8;
	}
	for (i = 11; i >= 8; --i)
	{
		buf[i] = (l & 0xFF);
		l >>= 8;
	}
	
	// Don't let crazy stuff in there.
	utf8Truncate(subject, 200);
	
	if (writer->insert(prefix + LU_MESSAGE_AUTHOR_EMAIL + author_email) != 0 ||
	    writer->insert(prefix + LU_MESSAGE_AUTHOR_NAME  + author_name)  != 0 ||
	    writer->insert(prefix + LU_MESSAGE_SUBJECT      + subject)      != 0 ||
	    writer->insert(mbox + string((char*)buf, 12)) != 0)
	{
		cerr << "Failed to insert summary keys" << endl;
		return -1;
	}
	
	return 0;
}

int Index::index_threading()
{
	string shash = subject_hash(subject.c_str());
	string suffix;
	
	unsigned char hash[4];
	
	if (writer->insert(
		LU_KEYWORD
		LU_KEYWORD_THREAD + 
		shash + 
		'\0' + 
		id.raw()) != 0)
	{
		cerr << "Failed to insert threading keyword" << endl;
		return -1;
	}
	
	// if (message.hasHeaders())
	
	if (message.Headers().HasInReplyTo())
	{
		vector<string> ids = extract_message_ids(
			message.Headers().InReplyTo().AsString().c_str());
		
		// first in-reply-to is most relevant
		for (vector<string>::iterator i = ids.begin(); i != ids.end(); ++i)
		{
			build_message_hash(i->c_str(), hash);
			
			// keep it reasonable; too many reply-tos is bad
			if (suffix.length() < 200)
				suffix.append((const char*)hash, 4);
		}
	}
	
	if (message.Headers().HasReferences())
	{
		vector<string> ids = extract_message_ids(
			message.Headers().References().AsString().c_str());
		
		// last references is most recently added (most likely irt)
		for (vector<string>::reverse_iterator i = ids.rbegin(); 
		     i != ids.rend(); ++i)
		{
			build_message_hash(i->c_str(), hash);
			// keep it reasonable; too many reply-tos is bad
			if (suffix.length() < 200)
				suffix.append((const char*)hash, 4);
		}
	}
	
	if (writer->insert(
		LU_THREADING
		+ shash
		+ id.raw()
		+ suffix) != 0)
	{
		cerr << "Failed to insert threading keys" << endl;
		return -1;
	}
	
	if (writer->insert(
		LU_NEW_TOPICS
		+ list.mbox + '\0'
		+ id.raw().substr(0, 4)
		+ shash) != 0)
	{
		cerr << "Failed to insert new topics keys" << endl;
		return -1;
	}
	
	return 0;
}

int Index::index_control(time_t import)
{
	bool ok = true;
	if (writer->insert(
		LU_KEYWORD 
		LU_KEYWORD_LIST +
		list.mbox + 
		'\0' + 
		id.raw()) != 0) ok = false;
	
	if (writer->insert(
		LU_KEYWORD 
		LU_KEYWORD_GROUP +
		list.group + 
		'\0' + 
		id.raw()) != 0) ok = false;
	
	MessageId importStamp(import);
	if (writer->insert(
		LU_CACHE +
		importStamp.raw().substr(0, 4) +
		id.raw()) != 0) ok = false;
	
	if (author_email.length())
	{
		if (my_keyword_digest_string(
			author_email.c_str(), author_email.length(),
			LU_KEYWORD_AUTHOR, &feed_writer, this, 1) != 0)
			ok = false;
	}
	
	if (author_name.length())
	{
		if (my_keyword_digest_string(
			author_name.c_str(), author_name.length(),
			LU_KEYWORD_AUTHOR, &feed_writer, this, 1) != 0)
			ok = false;
	}
	
	if (subject.length())
	{
		if (my_keyword_digest_string(
			subject.c_str(), subject.length(),
			LU_KEYWORD_SUBJECT, &feed_writer, this, 1) != 0)
			ok = false;
	}
	
	if (message.Headers().HasInReplyTo())
	{
		vector<string> ids = extract_message_ids(
			message.Headers().InReplyTo().AsString().c_str());
		for (vector<string>::iterator i = ids.begin(); i != ids.end(); ++i)
			if (my_keyword_digest_string(
				i->c_str(), i->length(),
				LU_KEYWORD_REPLY_TO, &feed_writer, this, 0) != 0)
				ok = false;
	}
	
#if 0	// this is questionable...
	if (message.Headers().HasReferences())
	{
		vector<string> ids = extract_message_ids(
			message.Headers().References().AsString().c_str());
		for (vector<string>::iterator i = ids.begin(); i != ids.end(); ++i)
			if (my_keyword_digest_string(
				i->c_str(), i->length(),
				LU_KEYWORD_REPLY_TO, &feed_writer, this, 0) != 0)
				ok = false;
	}
#endif
	
	if (!ok)
	{
		cerr << "Failed to insert control keys" << endl;
		return -1;
	}
	
	return 0;
}

int Index::index_entity(DwEntity& e, const string& charset)
{
	DwString text;
	if (e.Headers().HasContentTransferEncoding())
	{
		switch (e.Headers().ContentTransferEncoding().AsEnum())
		{
		case DwMime::kCteQuotedPrintable:
			DwDecodeQuotedPrintable(e.Body().AsString(), text);
			break;
		
		case DwMime::kCteBase64:
			DwDecodeBase64(e.Body().AsString(), text);
			break;
		
		case DwMime::kCteNull:
		case DwMime::kCteUnknown:
		case DwMime::kCte7bit:
		case DwMime::kCte8bit:
		case DwMime::kCteBinary:
			text = e.Body().AsString();
			break;
		}
	}
	else
	{
		text = e.Body().AsString();
	}
	
	CharsetEscape decode(charset.c_str());
	string utf8 = decode.write(text.c_str(), text.length());
	
	if (my_keyword_digest_string(
		utf8.c_str(), utf8.length(),
		LU_KEYWORD_WORD, &feed_writer, this, 1) != 0)
	{
		cerr << "Failed to index un-typed segment" << endl;
		return -1;
	}
	
	return 0;
}

int Index::index_keywords(DwEntity& e, const string& parentCharset)
{
	string charset = parentCharset;
	
	if (e.Headers().HasContentType())
	{
		DwMediaType& mt = e.Headers().ContentType();
		
		for (DwParameter* p = mt.FirstParameter(); p; p = p->Next())
		{
			DwString attr = p->Attribute();
			attr.ConvertToLowerCase(); // case insens
			if (attr == "charset") charset = p->Value().c_str();
		}
	}
	
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& t = e.Headers().ContentType();
		switch (t.Type())
		{
		case DwMime::kTypeMessage:
			if (e.Body().Message()) 
				index_keywords(*e.Body().Message(), charset);
			break;
			
		case DwMime::kTypeMultipart:
			// index all alternatives in multipart
			for (DwBodyPart* p = e.Body().FirstBodyPart(); p != 0; p = p->Next())
				index_keywords(*p, charset);
			break;
			
		case DwMime::kTypeText:
			if (t.Subtype() == DwMime::kSubtypePlain)
			{
				if (index_entity(e, charset) != 0) return -1;
			}
			break;
		}
	}
	else
	{
		if (index_entity(e, charset) != 0) return -1;
	}
	
	return 0;
}

int Index::index(time_t envelope, time_t import, bool check, bool& exist)
{
	exist = false;
	message.Parse();
	
//	cout << message.Headers().Subject().AsString().c_str() << endl;
	
	if (index_author()    < 0) return -1;
	if (index_id(envelope) < 0) return -1;
	if (index_summary(check, exist) < 0) return -1;
	
	if (exist) return 0;
	
	if (index_threading(      )                < 0) return -1;
	if (index_control  (import)                < 0) return -1;
	if (index_keywords (message, "ISO-8859-1") < 0) return -1;
	
	return 0;
}
