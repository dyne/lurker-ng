/*  $Id: MessageId.h,v 1.3 2003-04-25 15:12:21 terpstra Exp $
 *  
 *  MessageId.h - Helper class for manipulating internal message ids
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

#ifndef MESSAGE_ID_H
#define MESSAGE_ID_H

#include <ctime>
#include <string>

using std::string;

/** This class calculates lurker internal message ids.
 *  It can save them to appropriate format, and parse them.
 */
class MessageId
{
 protected:
 	unsigned char time_[4];
 	unsigned char hash_[4];
 	
 public:
 	/** Create a lurker message-id; you have to find the hash.
 	 */
 	MessageId(time_t date, const unsigned char* hash)
 	{
 		time_[3] = (date & 0xFF); date >>= 8;
 		time_[2] = (date & 0xFF); date >>= 8;
 		time_[1] = (date & 0xFF); date >>= 8;
 		time_[0] = (date & 0xFF);
 		hash_[0] = hash[0];
 		hash_[1] = hash[1];
 		hash_[2] = hash[2];
 		hash_[3] = hash[3];
 	}
 	
 	/** Create a MessageId for the purposes of seeking.
 	 *  The hash component will be zero.
 	 */
 	MessageId(time_t date = 0)
 	{
 		time_[3] = (date & 0xFF); date >>= 8;
 		time_[2] = (date & 0xFF); date >>= 8;
 		time_[1] = (date & 0xFF); date >>= 8;
 		time_[0] = (date & 0xFF);
 		hash_[0] = hash_[1] = hash_[2] = hash_[3] = 0;
 	}
 	
 	/** Create the MessageId from raw form.
 	 *  The int arg is to disambiguate from the pretty constructor
 	 */
 	MessageId(const char* str, int)
 	{
 		time_[0] = *str; ++str;
 		time_[1] = *str; ++str;
 		time_[2] = *str; ++str;
 		time_[3] = *str; ++str;
 		hash_[0] = *str; ++str;
 		hash_[1] = *str; ++str;
 		hash_[2] = *str; ++str;
 		hash_[3] = *str;
 	}
 	
 	/** Create a MessageId from the pretty serialized form.
 	 */
 	MessageId(const char* str);
 	
 	/** Extract the message timestamp.
 	 */
 	time_t timestamp() const
 	{
 		time_t out;
 		out =  time_[0]; out <<= 8;
 		out |= time_[1]; out <<= 8;
 		out |= time_[2]; out <<= 8;
 		out |= time_[3];
 		return out;
 	}
 	
 	/** Retrieve the hash for use in threading.
 	 */
 	string hash() const
 	{
 		return string((const char*)hash_, 4);
 	}
 	
 	/** Serialize the message-id for forward seeking.
 	 */
 	string raw() const
 	{
 		unsigned char tmp[8];
 		tmp[0] = time_[0];
 		tmp[1] = time_[1];
 		tmp[2] = time_[2];
 		tmp[3] = time_[3];
 		tmp[4] = hash_[0];
 		tmp[5] = hash_[1];
 		tmp[6] = hash_[2];
 		tmp[7] = hash_[3];
 		return string((const char*)tmp, 8);
 	}
 	
 	/** Serialize it pretty for the user interface.
 	 */
 	string serialize() const;
 	
 	/** Is the message id the same as another message?
 	 */
 	bool operator == (const MessageId& o) const
 	{
 		return	hash_[0] == o.hash_[0] &&
 			hash_[1] == o.hash_[1] &&
 			hash_[2] == o.hash_[2] &&
 			hash_[3] == o.hash_[3] &&
 			time_[0] == o.time_[0] &&
 			time_[1] == o.time_[1] &&
 			time_[2] == o.time_[2] &&
 			time_[3] == o.time_[3];
 	}
 	
 	/** Is the message id the earlier than another message?
 	 */
 	bool operator < (const MessageId& o) const
 	{
 		return raw() < o.raw();
 	}
 	
 	bool operator != (const MessageId& o) const { return !(*this == o); }
 	bool operator >  (const MessageId& o) const { return   o < *this;  }
 	bool operator >= (const MessageId& o) const { return !(*this < o); }
 	bool operator <= (const MessageId& o) const { return !(o < *this); }
};

#endif
