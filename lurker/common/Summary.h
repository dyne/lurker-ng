/*  $Id: Summary.h,v 1.4 2006-06-28 16:26:50 terpstra Exp $
 *  
 *  Summary.h - Helper which can load a message given MessageId
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
 
#ifndef SUMMARY_H
#define SUMMARY_H

#include <esort.h>
#include "MessageId.h"
#include "ConfigFile.h"

#include <set>

#include <sys/types.h> // off_t
#include <unistd.h>

class DwMessage;

using std::set;
using std::ostream;
using namespace ESort;

// Convert all linear white space into a single space
string whitespace_sanitize(const string& x);

class Summary
{
 protected:
 	MessageId	id_;
 	bool		deleted_;
 	bool		allowed_;
 	string		author_email_;
 	string		author_name_;
 	string		subject_;
 	string		mbox_;
 	off_t		offset_;
 	unsigned long	length_;
 	
 	set<string>	mboxs_;
 
 public:
 	Summary() { }
 	Summary(const MessageId& id) : id_(id), deleted_(false), allowed_(true) { }
 	
 	string load(Reader* r, const Config& cfg); // "" is success
 	bool loaded()  const { return mbox_ != ""; }
 	bool deleted() const { return deleted_; }
 	bool allowed() const { return allowed_; }
 	
 	const MessageId& id() const { return id_; }
 	
 	const string& author_email() const { return author_email_; }
 	const string& author_name () const { return author_name_;  }
 	const string& subject     () const { return subject_;      }
 	
 	const set<string>& mboxs() const { return mboxs_; }
 	
 	string message(const string& dbdir, DwMessage& out) const; // "" is success
 	
 	bool operator < (const Summary& o) const { return id_ < o.id_; }
};

ostream& operator << (ostream& o, const Summary& s);

#endif
