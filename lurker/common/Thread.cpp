/*  $Id: Thread.cpp,v 1.4 2003-06-12 20:37:25 terpstra Exp $
 *  
 *  Thread.h - Helper class for calculating threading
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

#include <cstdio>
#include <string>

#include "md5.h"
#include "Keys.h"

using std::string;

#define LU_SQUISHY_MAX	512

// Doesn't vary with charset
inline bool lu_isspace(char x)
{
	return x == ' ' || x == '\n' || x == '\r' || x == '\t';
}

// Doesn't vary with charset
inline bool lu_isalnum(char x)
{
	return	(x >= 'a' && x <= 'z') || 
		(x >= 'A' && x <= 'Z') ||
		(x >= '0' && x <= '9');
}

inline char lu_tolower(char x)
{
	if (x >= 'A' && x <= 'Z')
		return x - 'A' + 'a';
	return x;
}

// Stolen from lurker 0.1g:
int my_summary_squishy_subject(
	const char* subject, 
	char* target)
{
	/* Alright, we want to drop 're:', 'fwd:', etc.
	 * Also drop [...] chunks
	 * Anything after a 'was:' should be cut.
	 * Changes in case shouldn't be confusing.
	 * Punctuation is disregarded.
	 * We want a maximum length. (LU_SQUISHY_MAX)
	 */
	
	const char* r;
	const char* s;
	char* e;
	char* w;
	int state, ws;
	
	if (!subject)
	{
		*target = 0;
		return 0;
	}
	
	/* Skip past any number of: ' *[^ :]{0, 8}:' sequences
	 * Also, any number of '\[[^\]]{0,16}\]' sequences
	 */
	state = 1;
	
	r = s = subject;
	while (*s)
	{
		if (state == 0)
		{	/* We are scanning a word that could be ...: */
			if (*s == '[' && s == r)
			{
				state = 2;
			}
			if (*s == ':')
			{
				state = 1;
			}
			else if (*s == ' ' || s - r > 8)
			{
				break;
			}
			
			s++;
		}
		else if (state == 1)
		{	/* We're skiping past whitespace */
			if (lu_isspace(*s))
			{
				s++;
			}
			else
			{
				r = s;
				state = 0;
			}
		}
		else
		{	/* We're skipping past a [...] */
			if (*s == ']')
			{
				state = 1;
			}
			else if (s - r > 16)
			{
				break;
			}
			
			s++;
		}
	}
	
	/* Ok, begin writing the string out to target.
	 * We compress whitespace to a single space.
	 * Change everything to lower case.
	 * Drop punctuation on the floor.
	 * Stop on a ':' and discard any word that preceded it. (was:)
	 */
	ws = 0;
	for (	w = target, e = w + LU_SQUISHY_MAX-1; 
		*r && w != e; r++)
	{
		if (lu_isspace(*r))
		{
			ws = 1;
		}
		else
		{
			if (*r == ':' && (w - target) > 8 &&
				lu_tolower(*(r-1)) == 's' &&
				lu_tolower(*(r-2)) == 'a' &&
				lu_tolower(*(r-3)) == 'w')
			{	/* Nasty colons! */
				break;
			}
			
			if (lu_isalnum(*r))
			{
				if (ws)
				{
					*w++ = ' ';
					ws = 0;
					
					/* Need to retest since we are doing a double
					 * write
					 */
					if (w == e) break;
				}
				
				*w++ = lu_tolower(*r);
			}
		}
	}
	
	if (*r == ':')
	{	/* Rewind 'w' by one word */
		if (w != target) w--;
		
		for (; w != target; w--)
			if (lu_isspace(*w))
				break;
	}
	
	*w = 0;
	return w - target;
}

string subject_hash(const char* subject)
{
	char sbj[LU_SQUISHY_MAX];
	my_summary_squishy_subject(subject, sbj);
	
	MD5Context ctx;
	unsigned char buf[16];
	
	MD5Init(&ctx);
	MD5Update(&ctx, (unsigned char*)sbj, strlen(sbj));
	MD5Final(buf, &ctx);
	
	unsigned char xord[4];
	xord[0] = buf[0] ^ buf[4] ^ buf[ 8] ^ buf[12];
	xord[1] = buf[1] ^ buf[5] ^ buf[ 9] ^ buf[13];
	xord[2] = buf[2] ^ buf[6] ^ buf[10] ^ buf[14];
	xord[3] = buf[3] ^ buf[7] ^ buf[11] ^ buf[15];
	
	char code[9];
	snprintf(code, sizeof(code), "%02x%02x%02x%02x",
		xord[0], xord[1], xord[2], xord[3]);
	
	return code;
}

vector<string> extract_message_ids(const char* str)
{
	vector<string> out;
	
	while (1)
	{
		while (*str && *str != '<') ++str;
		if (!*str) break;
		
		const char* start = ++str;
		while (*str && *str != '>' && !lu_isspace(*str)) ++str;
		if (!*str) break;
		if (lu_isspace(*str)) continue;
		
		// this is a valid msg-id?
		string maybe(start, str - start);
		if (maybe.find('@'))
		{	// cut stupid message-ids off AFTER we know it's an id
			// (this can remove the @ sign)
			if (maybe.length() > 100) maybe.resize(100);
			out.push_back(maybe);
		}
	}
	
	return out;
}

