/*  $Id: MessageId.cpp,v 1.5 2003-06-12 20:37:25 terpstra Exp $
 *  
 *  MessageId.cpp - Helper class for manipulating internal message ids
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
#define _FILE_OFFSET_BITS 64

#include "MessageId.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

inline int dehex(char x)
{
	if (x >= 'a' && x <= 'z') return x - 'a' + 10;
	if (x >= 'A' && x <= 'Z') return x - 'A' + 10;
	return x - '0';
}

time_t my_timegm(struct tm* tm)
{
	static bool	initd = false;
	static time_t	delta;
	
	if (!initd)
	{	// use 'x' to calculate local time zone offset portably
		time_t x = time(0);
		time_t y = mktime(gmtime(&x));
		
		delta = x - y;
		initd = true;
	}
	
	struct tm hack = *tm;
	hack.tm_sec += delta;
	return mktime(&hack);
}

/** Note; the serialized time is always in UTC!
 *  
 *  This is so that the same message will have the same lurker id across
 *  all servers with the message, regardless of their timezone.
 */

MessageId::MessageId(const char* str)
{
	if (strlen(str) > 14 && str[8] == '.')
	{
		struct tm t;
		memset(&t, 0, sizeof(t));
		
		t.tm_year =	(str[ 0] - '0') * 1000 +
				(str[ 1] - '0') * 100 +
				(str[ 2] - '0') * 10 +
				(str[ 3] - '0')
				- 1900;
		t.tm_mon =	(str[ 4] - '0') * 10 +
				(str[ 5] - '0')
				- 1;
		t.tm_mday =	(str[ 6] - '0') * 10 +
				(str[ 7] - '0');
		t.tm_hour = 	(str[ 9] - '0') * 10 +
				(str[10] - '0');
		t.tm_min =	(str[11] - '0') * 10 +
				(str[12] - '0');
		t.tm_sec =	(str[13] - '0') * 10 +
				(str[14] - '0');
		
		time_t tm = my_timegm(&t);
		
		time_[3] = (tm & 0xFF); tm >>= 8;
		time_[2] = (tm & 0xFF); tm >>= 8;
		time_[1] = (tm & 0xFF); tm >>= 8;
		time_[0] = (tm & 0xFF);
	}
	else
	{
		time_[0] = time_[1] = time_[2] = time_[3] = 0;
	}
	
	if (strlen(str) > 23 && str[15] == '.')
	{
		hash_[0] = dehex(str[16]) * 16 + dehex(str[17]);
		hash_[1] = dehex(str[18]) * 16 + dehex(str[19]);
		hash_[2] = dehex(str[20]) * 16 + dehex(str[21]);
		hash_[3] = dehex(str[22]) * 16 + dehex(str[23]);
	}
	else
	{
		hash_[0] = hash_[1] = hash_[2] = hash_[3] = 0;
	}
}

string MessageId::serialize() const
{
	char buf[26];
	
	time_t then = timestamp();
	strftime(buf, 25, "%Y%m%d.%H%M%S", gmtime(&then));
	snprintf(&buf[15], 10, ".%02x%02x%02x%02x",
		hash_[0], hash_[1], hash_[2], hash_[3]);
	
	return buf;
}
