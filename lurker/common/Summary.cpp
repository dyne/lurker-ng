/*  $Id$
 *  
 *  Summary.cpp - Helper which can load a message given MessageId
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

#include <zlib.h>
#include <unistd.h>
#include <fcntl.h>

#include <Keys.h> 
#include <XmlEscape.h>

#include <memory>
#include <cerrno>
#include <cstring>

#include "Summary.h"

using namespace std;

string Summary::load(Reader* r, const Config& cfg)
{
	// Use the prefix limited search
	auto_ptr<Walker> w(r->seek(LU_SUMMARY + id_.raw(), "", Forward));
	Config::Lists::const_iterator li;
	
	allowed_ = false;
	
	// This will only walk records matching this id
	int ok;
	mbox_ = "";
	while ((ok = w->advance()) != -1)
	{
		if (w->key.length() < 1 + 8 + 1)
			return "invalid mbox entry -- way too short";
		
		// We use this for getting an unsigned value below.
		const unsigned char* k = (const unsigned char*)w->key.c_str()+1+8;
		
		// read all the values
		switch (*k)
		{
		case LU_MESSAGE_DELETED:
			deleted_ = true;
			break;
			
		case LU_MESSAGE_AUTHOR_EMAIL:
			author_email_ = w->key.substr(1+8+1, string::npos);
			if (cfg.hide_email)
			{
				string::size_type x = author_email_.find('@');
				if (x != string::npos) author_email_.resize(x);
				if (!author_name_.length()) author_name_ = author_email_;
				author_email_ = "";
			}
			break;
			
		case LU_MESSAGE_AUTHOR_NAME:
			if (w->key.length() > 1+8+1)
				author_name_ = w->key.substr(1+8+1, string::npos);
			break;
			
		case LU_MESSAGE_SUBJECT:
			subject_ = w->key.substr(1+8+1, string::npos);
			break;
			
		case LU_MESSAGE_MBOX:
			if (w->key.length() < 1+8+1+1+12)
				return "invalid mbox entry -- too short";
			
			//!!! could be more careful about corrupt dbs here
			
			++k;
			mbox_ = (const char*)k; // null terminated
			k += mbox_.length();
			
			int i;
			offset_ = 0;
			
			for (i = 0; i < 8; ++i)
			{
				offset_ <<= 8;
				offset_ |= *++k;
			}
			length_ = 0;
			for (i = 8; i < 12; ++i)
			{
				length_ <<= 8;
				length_ |= *++k;
			}
			
			mboxs_.insert(mbox_);
			li = cfg.lists.find(mbox_);
			if (li == cfg.lists.end()) return "referenced list is missing: " + mbox_;
			if (li->second.allowed) allowed_ = true;
			
			break;
		
		default:
			return "unknown mbox summary control code";
		}
	}
	
	if (mbox_ == "")
		return "not in a mailbox";
	
	if (ok == -1 && errno != 0)
		return string("Walker::advance:") + strerror(errno);
	
	return "";
}

string Summary::message(const string& dbdir, DwMessage& message) const
{
	string name = dbdir + "/" + mbox_;
	int fd = open(name.c_str(), O_RDONLY);
	if (fd == -1)
		return name + ":open:" + strerror(errno);
	
	if (lseek(fd, offset_, SEEK_SET) != offset_)
	{
		close(fd);
		return name + ":lseek:" + strerror(errno);
	}
	
	gzFile gzf = gzdopen(fd, "rb");
	if (gzf == 0)
	{
		close(fd);
		return name + ":gzdopen:" + strerror(errno);
	}
	
	DwString str;
	char buf[8192];
	unsigned long want = length_;
	
	while (want)
	{
		long get;
		if (want < sizeof(buf))
			get = want;
		else	get = sizeof(buf);
		
		if (gzread(gzf, buf, get) != get)
		{
			gzclose(gzf);
			return name + ":gzread:" + strerror(errno);
		}
		
		str.append(buf, get);
		want -= get;
	}
	
	gzclose(gzf); // also closes fd
	
	message.FromString(str);
	message.Parse();
	
	return "";
}

ostream& operator << (ostream& o, const Summary& s)
{
	o << "<summary>"
	  << "<id>" << s.id().serialize() << "</id>"
	  << "<timestamp>" << s.id().timestamp() << "</timestamp>";
	
	if (s.deleted() || !s.allowed())
	{
		o << "<deleted/>";
	}
	else
	{
		if (s.subject().length() > 0)
			o << "<subject>" << xmlEscape << s.subject() << "</subject>";
	
		o  << "<email";
		if (s.author_email().length() > 0)
			o << " address=\"" << xmlEscape << s.author_email() << "\"";
		if (s.author_name().length() > 0)
			o << " name=\"" << xmlEscape << s.author_name() << "\"";
		o << "/>";
	}
	
	o << "</summary>";
	return o;
}

// Locale independent
static inline bool my_isspace(char x)
{
	return x == '\t' || x == '\n' || x == '\v' || 
	       x == '\f' || x == '\r' || x == ' ';
}

string whitespace_sanitize(const string& x)
{
	string out;
	string::size_type i = 0;
	
	// Trim leading whitespace
	while (i < x.length() && my_isspace(x[i])) i++;
	
	while (1)
	{
		// Copy non-whitespace
		while (i < x.length() && !my_isspace(x[i])) out += x[i++];
		while (i < x.length() && my_isspace(x[i])) i++;
		if (i == x.length()) break;
		// If we have more non-whitespace, and passed some whitespace,
		// then output a single space
		out += " ";
	}
	
	return out;
}
