/*  $Id: main.cpp,v 1.34 2003-06-26 21:35:01 terpstra Exp $
 *  
 *  main.cpp - Read the fed data into our database
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
#include <mimelib/body.h>
#include <mimelib/bodypart.h>
#include <mimelib/enum.h>
#include <mimelib/utility.h>

#include <Config.h>
#include <esort.h>

#include <iostream>
#include <cstdio>
#include <ctime>
#include <cstdlib>

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <zlib.h>

#include "Index.h"

// Our friendly neighbourhood date parser (getdate.y / getdate.cpp)
time_t get_date (const char *p, const time_t *now);

using namespace std;

auto_ptr<ESort::Writer> db;

string         append;
List*          list = 0;
int            mbox = -1;
off_t          length = 0;

void help(const char* name)
{
	cerr << "Lurker-index (v" << VERSION << ") imports messages into the archive.\n";
	cerr << "\n";
	cerr << "Usage: " << name << " -c <config-file> -l <list> (-m | -b <count>)\n";
	cerr << "                                              [-v -d -n -f]\n";
	cerr << "\n";
	cerr << "\t-c <config-file> Use this config file for lurker settings\n";
	cerr << "\t-l <list>        Import messages to the named list\n";
	cerr << "\t-b <count>       Import a batch of messages; flush every count messages\n";
	cerr << "\t-m               Import a single message\n";
	cerr << "\t-v               Verbose operation\n";
	cerr << "\t-d               Drop duplicates per list\n";
	cout << "\t-n               Don't compress messages\n";
	cerr << "\t-f               Fast import (but vulnerable to power-failure)\n";
	cerr << "\n";
	cerr << "Index messages from standard input and store them in the lurker database.\n";
	cerr << "Either import a single message, or import a batch of messages in mbox format.\n";
	cerr << "\n";
}

int commit()
{
	unsigned long done = 0;
	while (done < append.length())
	{
		long did = write(mbox, append.c_str()+done, append.length()-done);
		if (did <= 0)
		{
			perror("write");
			ftruncate(mbox, length);
			cerr << "Mbox write failed; aborting..." << endl;
			return -1;
		}
		done += did;
	}
	
	if (db->commit() != 0)
	{
		perror("commit");
		ftruncate(mbox, length);
		cerr << "Commit failed; aborting..." << endl;
		return -1;
	}
	
	length += append.length();
	append = "";
	
	return 0;
}

void look_for_from(DwEntity& e)
{
	if (e.Headers().HasContentTransferEncoding() &&
	    e.Headers().ContentTransferEncoding().AsEnum() == DwMime::kCteBase64)
		return; // base64 is already fine
	
	const DwString& body = e.Body().AsString();
	if (body.substr(0, 5) != "From " && body.find("\nFrom ") == DwString::npos)
		return; // no From to kill
	
	if (e.Headers().HasContentTransferEncoding() &&
	    e.Headers().ContentTransferEncoding().AsEnum() == DwMime::kCteQuotedPrintable)
	{	// the thing already is quoted-printable encoded
		// we will try to minimize the changes
		DwString newbody;
		
		size_t s, x;
		if (body.substr(0, 5) == "From ")
		{
			newbody = "=46rom ";
			s = 5;
		}
		else
		{
			s = 0;
		}
		
		while ((x = body.find("\nFrom ", s)) != string::npos)
		{
			newbody.append(body, s, x-s);
			newbody += "\n=46";
			s = x + 2;
		}
		
		newbody.append(body, s, body.length() - s);
		e.Body().FromString(newbody);
	}
	else
	{
		// We have to kill the 'From '.
		// Let's just make it quoted-printable.
		// This will preserve the content (no '>From').
		
		DwString coded;
		e.Headers().ContentTransferEncoding().FromEnum(DwMime::kCteQuotedPrintable);
		DwEncodeQuotedPrintable(body, coded);
		e.Body().FromString(coded);
	}
}

void recursively_kill_from(DwEntity& e)
{
	// if (e.hasHeaders() && 
	if (e.Headers().HasContentType())
	{
		DwMediaType& t = e.Headers().ContentType();
		switch (t.Type())
		{
		case DwMime::kTypeMessage:
			if (e.Body().Message()) 
				recursively_kill_from(*e.Body().Message());
			break;
			
		case DwMime::kTypeMultipart:
			// index all alternatives in multipart
			for (DwBodyPart* p = e.Body().FirstBodyPart(); p != 0; p = p->Next())
				recursively_kill_from(*p);
			break;
			
		default:
			look_for_from(e);
			break;
		}
	}
	else
	{
		look_for_from(e);
	}
}

int index(DwString& msg, long batch, bool check, bool compress)
{
//	cout << msg.c_str() << endl;
	static int count = 0;
	if (++count == batch)
	{
		count = 0;
		if (commit() != 0) return -1;
	}
	
	DwMessage message(msg);
	
	message.Parse();
	recursively_kill_from(message);
	message.Assemble();
	msg = message.AsString(); // does not copy b/c the str is ref counted
	
	off_t start = length + append.length();
	string::size_type unwind = append.length();
	
	time_t import = time(0);
	time_t arrival;
	
	char prefix[100];
	const char* d = msg.c_str();
	if (d[0] == 'F' && d[1] == 'r' && d[2] == 'o' && d[3] == 'm' && d[4] == ' ')
	{
		while (*d && *d != ' ' && *d != '\t') ++d;
		while (*d && (*d == ' ' || *d == '\t')) ++d;
		while (*d && *d != ' ' && *d != '\t') ++d;
		while (*d && (*d == ' ' || *d == '\t')) ++d;
		
		const char* e;
		for (e = d; *e; ++e) if (*e == '\r' || *e == '\n') break;
		
		string date(d, long(e)-long(d));
		
		arrival = get_date(date.c_str(), &import);
		prefix[0] = 0;
	}
	else
	{
		arrival = import;
		
		/** Make a local-timezone timestamp for the message envelope.
		 */
		strftime(prefix, sizeof(prefix),
		  "From lurker-index@localhost %a %b %2d %02H:%02M:%02S %Y\r\n",
		  localtime(&arrival));
	}
	
	// Err, messages from the future? I don't think so.
	// We have a day's worth of tolerance in case of a timezone issue
	if (arrival > import + 24*60*60) arrival = import;
	
	if (compress)
	{
		// Write out a gzip header field
		append.append(string(
			"\x1f\x8b\x08\x00\x00\x00\x00\x00\x00\x0c", 10));
		
		// Compress the data (can't use zlib's compress b/c need -WBITS)
		char buf[4096];
		z_stream stream;
		int err;
		
		stream.next_in   = (unsigned char*)&prefix[0];
		stream.avail_in  = strlen(&prefix[0]);
		stream.next_out  = (unsigned char*)buf;
		stream.avail_out = sizeof(buf);
		
		stream.zalloc = 0;
		stream.zfree  = 0;
		stream.opaque = 0;
		
		// -MAX_WBITS is an undocumented feature needed to omit header
		err = deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
			-MAX_WBITS, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
		
		do
		{
			assert (err == Z_OK);
			if (stream.avail_out == 0)
			{
				append.append(buf, sizeof(buf));
				stream.next_out  = (unsigned char*)buf;
				stream.avail_out = sizeof(buf);
			}
			err = deflate(&stream, Z_NO_FLUSH);
		} while (stream.avail_in != 0);
		
		// comprses the real body
		stream.next_in   = (unsigned char*)msg.c_str();
		stream.avail_in  = msg.length();
		err = Z_OK;
		
		do
		{
			assert (err == Z_OK);
			if (stream.avail_out == 0)
			{
				append.append(buf, sizeof(buf));
				stream.next_out  = (unsigned char*)buf;
				stream.avail_out = sizeof(buf);
			}
			err = deflate(&stream, Z_FINISH);
		} while (err != Z_STREAM_END);
		
		err = deflateEnd(&stream);
		assert (err == Z_OK);
		
		append.append(buf, sizeof(buf) - stream.avail_out);
		
		// Write out the annoying gzip tail field
		unsigned long crc = crc32(0, Z_NULL, 0);
		crc = crc32(crc, (unsigned char*)msg.c_str(), msg.length());
		int n;
		for (n = 0; n < 4; ++n)
		{
			append += ((char)(crc & 0xFF));
			crc >>= 8;
		}
		unsigned long len = msg.length();
		for (n = 0; n < 4; ++n)
		{
			append += ((char)(len & 0xFF));
			len >>= 8;
		}
	}
	else
	{
		append.append(prefix);
		append.append(msg.c_str(), msg.length());
	}
	
	Index i(message, db.get(), *list, start, msg.length() + strlen(prefix));
	
	bool exist;
	if (i.index(arrival, import, check, exist) != 0)
	{
		cerr << "Import failed; aborting..." << endl;
		return -1;
	}
	
	if (exist)
	{	// already indexed -> don't append it
		append.resize(unwind);
	}
	
	return 0;
}

void status(off_t did, long messages, time_t start)
{
	time_t amt = time(0) - start;
	if (amt == 0) amt = 1;
	cout << "Imported " << messages << " messages (" << (did/(1024*1024)) << "MB)"
	     << " at " << (messages / amt) << " messages (" << (did/(1024*amt)) << "KB) / second" << endl;
}

string::size_type find_message_boundary(const string& str, string::size_type o)
{
	while ((o = str.find("\nFrom ", o)) != string::npos)
	{
		// We have a potential message break.
		// Our philosophy is: only reject it if it seems to be REALLY
		// broken; default to believing message breaks. If we ever
		// choose to ignore a break, given a warning. (after all the
		// mailbox is corrupt)
		
		++o; // step onto the 'From '
		string::size_type eol = str.find('\n', o);
		
		// we have not buffered the end of line? then believe that
		// it is a real break; this is only a heuristic
		if (eol == string::npos) return o;
		
		string header(str, o, eol - o);
		// A real message break will look like:
		// From <addr> <date>
		// If either 'addr' is an email address or 'date' a date
		// then we decide it is a valid message break. Only if
		// both are broken do we reject it.
		
		const char* d = header.c_str()+4;
		
		// skip whitespace
		while (*d && (*d == ' ' || *d == '\t')) ++d;
		
		bool email = false;
		// we should be on an email address.
		// if there is an '@' sign then this must be ok
		while (*d && *d != ' ' && *d != '\t')
		{
			if (*d == '@') email = true;
			++d;
		}
		
		// skip whitespace
		while (*d && (*d == ' ' || *d == '\t')) ++d;
		
		time_t now = 9999999; // don't care - just no syscall
		time_t arrival = get_date(d, &now);
		bool date = (arrival != (time_t)-1 && arrival != 0);
		
		// it sounds like this is probably a message break
		if (*d && (date || email)) return o;
		
		// well, we gave it the benefit of the doubt, but ...
		// this just don't look like a message break
		
		cerr << "warning: treating '" << header << "' as part of a message because it does not have an @ in the username or a valid date." << endl;
		// implicitly continue the loop
	}
	
	return o;
}

int main(int argc, char** argv)
{
	int c;
	
	const char* config  = 0;
	const char* listn   = 0;
	long        batch   = 0;
	int         verbose = 0;
	int         dropdup = 0;
	bool        synced  = true;
	bool        compress= true;
	
	srandom(time(0));
	
	while ((c = getopt(argc, (char*const*)argv, "c:l:b:mvndf?")) != -1)
	{
		switch ((char)c)
		{
		case 'c':
			config = optarg;
			break;
		case 'l':
			listn = optarg;
			break;
		case 'b':
			batch = atol(optarg);
			break;
		case 'm':
			batch = -1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 'd':
			dropdup = 1;
			break;
		case 'f':
			synced = false;
			break;
		case 'n':
			compress = false;
			break;
		default:
			help(argv[0]);
			return 1;
		}
	}
	
	if (!config || !listn || batch == 0 || optind < argc)
	{
		help(argv[0]);
		return 1;
	}
	
	Config cfg;
	if (cfg.load(config) != 0)
	{
		cerr << cfg.getError() << flush;
		return 1;
	}
	
	Config::Lists::iterator i = cfg.lists.find(listn);
	if (i == cfg.lists.end())
	{
		cerr << "No such list '" << listn << "' defined; cannot import to it." << endl;
		return 1;
	}
	list = &i->second;
	
	ESort::Parameters params(synced);
	// work around g++ 2.95 borkage
	auto_ptr<ESort::Writer> dbt(
		ESort::Writer::open(cfg.dbdir + "/db", params));
	db = dbt;
	
	if (!db.get())
	{
		perror("opening database");
		return 1;
	}
	
	string mboxf = cfg.dbdir + "/" + listn;
	mbox = open(mboxf.c_str(), O_RDWR|O_CREAT, 0666);
	if (mbox == -1)
	{
		perror(mboxf.c_str());
		return 1;
	}
	length = lseek(mbox, 0, SEEK_END);
	
	/** Begin processing standard input.
	 */
	string buf;
	char sect[8192];
	
	off_t did = 0;
	time_t start = time(0);
	long messages = 0;
	
	while (1)
	{
		int got = read(0, sect, sizeof(sect));
		if (got == -1)
		{
			perror("read");
			cerr << "Standard input failed!" << endl;
			return 1;
		}
		did += got;
		
		string::size_type was = buf.length();
		buf.append(sect, got);
		
		if (was >= 6)
			was -= 6;
		else	was = 0;
		
		string::size_type eos;
		while (batch != -1 && 
		       (eos = find_message_boundary(buf, was)) != string::npos)
		{
			DwString msg(buf.c_str(), eos);
			buf = buf.substr(eos, string::npos);
			
			if (msg[0] == 'F') // strlen > 1 b/c of linefeed prior to From
			{	// ignore potential leading blanks
				if (index(msg, batch, dropdup, compress) != 0) return 1;
				++messages;
				was = 0; 
			
				if (verbose && messages % 1000 == 0)
					status(did, messages, start);
			}
		}
		
		if (got == 0)
		{
			// Don't index the empty string!
			if (buf.length() && (batch == -1 || buf[0] == 'F'))
			{
				DwString msg(buf.c_str(), buf.length());
				buf = "";
				if (index(msg, batch, dropdup, compress) != 0) return 1;
				++messages;
			}
			
			break;
		}
	}
	
	if (commit() != 0) return 1;
	
	if (verbose) status(did, messages, start);
	
	return 0;
}
