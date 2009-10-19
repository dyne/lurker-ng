/*  $Id$
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

#include "MessageId.h"
#include "config.h"

#include <cstring>
#include <cstdio>
#include <cstdlib>

inline int dehex(char x)
{
	if (x >= 'a' && x <= 'f') return x - 'a' + 10;
	if (x >= 'A' && x <= 'F') return x - 'A' + 10;
	return x - '0';
}

#ifndef HAVE_TIMEGM
// Contributed by: Moritz.Eysholdt@mail.uni-oldenburg.de
static time_t my_timegm (struct tm *t)
{
	time_t tl, tb;
	struct tm *tg;
	
	tl = mktime(t);
	if (tl == -1)
	{
		t->tm_hour--;
		tl = mktime (t);
		if (tl == -1)
			return -1; /* can't deal with output from strptime */
		tl += 3600;
	}
	
	tg = gmtime (&tl);
	tg->tm_isdst = 0;
	tb = mktime (tg);
	if (tb == -1)
	{
		tg->tm_hour--;
		tb = mktime (tg);
		if (tb == -1)
			return -1; /* can't deal with output from gmtime */
		tb += 3600;
	}
	
	return (tl - (tb - tl));
}
#endif

const unsigned int MessageId::time_len = 15;
const unsigned int MessageId::full_len = 24;
const unsigned int MessageId::raw_len = 8;

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
		
#ifdef HAVE_TIMEGM
		time_t tm = timegm(&t);
#else
		time_t tm = my_timegm(&t);
#endif
		
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

static inline bool is_digit(char c)
{
	return (c >= '0' && c <= '9');
}

static inline bool is_hex(char c)
{
	return (c >= '0' && c <= '9') ||
	       (c >= 'a' && c <= 'f') ||
	       (c >= 'A' && c <= 'F');
}

bool MessageId::is_time(const char* s)
{
	return	is_digit(s[0]) && is_digit(s[1]) && 
		is_digit(s[2]) && is_digit(s[3]) &&
		is_digit(s[4]) && is_digit(s[5]) &&
		is_digit(s[6]) && is_digit(s[7]) &&
		s[8] == '.' &&
		is_digit(s[ 9]) && is_digit(s[10]) &&
		is_digit(s[11]) && is_digit(s[12]) &&
		is_digit(s[13]) && is_digit(s[14]);
}

bool MessageId::is_full(const char* s)
{
	return	is_time(s) &&
		s[15] == '.' &&
		is_hex(s[16]) && is_hex(s[17]) &&
		is_hex(s[18]) && is_hex(s[19]) &&
		is_hex(s[20]) && is_hex(s[21]) &&
		is_hex(s[22]) && is_hex(s[23]);
}
