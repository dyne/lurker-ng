/*  $Id: Index.cpp,v 1.14 2003-06-04 15:29:19 terpstra Exp $
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

string pickFullName(DwAddress* a, const char* charset)
{
	for (; a != 0; a = a->Next())
	{
		if (a->IsGroup())
		{
			DwGroup* g = dynamic_cast<DwGroup*>(a);
			if (g)
			{
				string out = pickFullName(
					g->MailboxList().FirstMailbox(),
					charset);
				if (out != "") return out;
			}
		}
		else
		{
			DwMailbox* m = dynamic_cast<DwMailbox*>(a);
			if (m)
			{
				string out = m->FullName().c_str();
				out = decode_header(out, charset);
				
				if (out != "" && out.length() < 128) 
				{
					if (out[0] == '"' && out.length() > 2)
						out = out.substr(1, out.length()-2);
					
					return out;
				}
			}
		}
	}
	
	return "";
}

string pickAddress(DwAddress* a)
{
	for (; a != 0; a = a->Next())
	{
		if (a->IsGroup())
		{
			DwGroup* g = dynamic_cast<DwGroup*>(a);
			if (g)
			{
				string out = pickAddress(
					g->MailboxList().FirstMailbox());
				if (out != "") return out;
			}
		}
		else
		{
			DwMailbox* m = dynamic_cast<DwMailbox*>(a);
			if (m)
			{
				// we ditch the routing; I will not be party
				// to the security problems they entail
				DwString out =
					m->LocalPart() +
					"@" +
					m->Domain();
				if (out.length() < 128)
					return out.c_str();
			}
		}
	}
	
	return "";
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
	
	if (message.Headers().HasReplyTo())
	{
		author_name = pickFullName(
			message.Headers().ReplyTo().FirstAddress(),
			charset.c_str());
		author_email = pickAddress(
			message.Headers().ReplyTo().FirstAddress());
		
		// Some evil mailing lists set reply-to the list.
		if (author_email == list.address)
		{
			author_email = "";
			author_name = "";
		}
	}
	
	if (message.Headers().HasFrom())
	{
		if (!author_name.length()) author_name = pickFullName(
			message.Headers().From().FirstMailbox(),
			charset.c_str());
		if (!author_email.length()) author_email = pickAddress(
			message.Headers().From().FirstMailbox());
	}
	
	if (message.Headers().HasSender())
	{
		if (!author_name.length()) author_name = pickFullName(
			&message.Headers().Sender(),
			charset.c_str());
		if (!author_email.length()) author_email = pickAddress(
			&message.Headers().Sender());
	}
	
	utf8Truncate(author_name, 100);
	utf8Truncate(author_email, 100);
	
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
		if (user <= server+60*60 && server < user+3*60*60*24)
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
	
	if (messageId.length() && writer->insert(
		LU_KEYWORD
		LU_KEYWORD_MESSAGE_ID + 
		messageId + 
		'\0' + 
		id.raw()) != 0)
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
	
	return 0;
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
			if (writer->insert(
				LU_KEYWORD 
				LU_KEYWORD_REPLY_TO +
				*i + 
				'\0' + 
				id.raw()) != 0) ok = false;
	}
	
#if 0	// this is questionable...
	if (message.Headers().HasReferences())
	{
		vector<string> ids = extract_message_ids(
			message.Headers().References().AsString().c_str());
		for (vector<string>::iterator i = ids.begin(); i != ids.end(); ++i)
			if (writer->insert(
				LU_KEYWORD 
				LU_KEYWORD_REPLY_TO +
				*i + 
				'\0' + 
				id.raw()) != 0) ok = false;
	}
#endif
	
	if (!ok)
	{
		cerr << "Failed to insert control keys" << endl;
		return -1;
	}
	
	return 0;
}

int Index::index_keywords(DwEntity& e)
{
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& t = e.Headers().ContentType();
		switch (t.Type())
		{
		case DwMime::kTypeMessage:
			if (e.Body().Message()) 
				index_keywords(*e.Body().Message());
			break;
			
		case DwMime::kTypeMultipart:
			for (DwBodyPart* p = e.Body().FirstBodyPart(); p != 0; p = p->Next())
				index_keywords(*p);
			break;
			
		case DwMime::kTypeText:
			if (t.Subtype() == DwMime::kSubtypePlain)
			{
				if (my_keyword_digest_string(
					e.Body().AsString().c_str(), e.Body().AsString().length(),
					LU_KEYWORD_WORD, &feed_writer, this, 1) != 0)
				{
					cerr << "Failed to index text/plain segment" << endl;
					return -1;
				}
			}
			break;
		}
	}
	else
	{
		if (my_keyword_digest_string(
			e.Body().AsString().c_str(), e.Body().AsString().length(),
			LU_KEYWORD_WORD, &feed_writer, this, 1) != 0)
		{
			cerr << "Failed to index un-typed segment" << endl;
			return -1;
		}
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
	
	if (index_threading(       ) < 0) return -1;
	if (index_control  (import ) < 0) return -1;
	if (index_keywords (message) < 0) return -1;
	
	return 0;
}
