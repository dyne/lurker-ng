/*  $Id: common.c,v 1.8 2002-02-25 04:25:26 terpstra Exp $
 *  
 *  common.c - common definitions and types for all tools
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

#define _POSIX_SOURCE

#include "common.h"
#include "prefix.h"

#include <ctype.h>
#include <iconv.h>
#include <string.h>

/*------------------------------------------------ Private global vars */

static const char my_common_enbase64[] = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static int  my_common_base64_init = 0;
static char my_common_debase64[256];

/*------------------------------------------------ Public global vars */

message_id lu_common_minvalid = -1;

/*------------------------------------------------ Public helper methods */

const char* lu_common_cleanup_id(
	const char* id)
{
	/* The prefix and the id must fit within keyword length */
	static char buf[LU_KEYWORD_LEN+1-sizeof(LU_KEYWORD_MESSAGE_ID)];
	char* w;
	char* e;
	
	while (isspace(*id)) id++;
	if (*id == '<') id++;
	
	w = &buf[0];
	e = &buf[sizeof(buf) - 1];
	
	while (*id && w != e)
	{
		if (isspace(*id) || *id == '>') break;
		*w++ = *id++;
	}
	
	*w = 0;
	return &buf[0];
}

static char* my_common_quoted(
	const char*	r,
	const char*	re,
	char*		w,
	char*		we,
	iconv_t		ic)
{
	char buf[100];
	
	char* b;
	char* be;
	
	size_t wl, bl, tmp;
	
	b = &buf[0];
	be = &buf[sizeof(buf)];
	
	while (r != re && w != we)
	{
		/* Firstly, decode a hunk of the string to binary */
		for (; r != re && b != be; r++) switch (*r)
		{
		case '_':
			*b++ = ' ';
			break;
			
		case '=':
			r++;
			if (*r >= 'A')	*b = (*r - 'A' + 10)	<< 4;
			else		*b = (*r - '0')		<< 4;
			
			r++;
			if (*r >= 'A')	*b++ |= (*r - 'A' + 10);
			else		*b++ |= (*r - '0');
			
			break;
			
		default:
			*b++ = *r;
		}
		
		/* Now, transform the charset to utf-8 */
		wl = we - w;
		bl = sizeof(buf) - (be - b);
		
		b = &buf[0];
		tmp = iconv(ic, &b, &bl, &w, &wl);
		
		/* Shift any remaining partial bytes in the buffer over */
		memmove(&buf[0], b, bl);
		b = &buf[bl];
		w = we - wl;
		
		if (tmp == (size_t)-1 && errno != EINVAL) break;
	}
	
	return w;
}

static char* my_common_base64(
	const char*	r,
	const char*	re,
	char*		w,
	char*		we,
	iconv_t		ic)
{
	char buf[100];
	
	char* b;
	char* be;
	
	size_t wl, bl, tmp;
	int i;
	
	if (!my_common_base64_init)
	{
		memset(&my_common_debase64, 0x7F, 256);
		
		for (i = 0; i < 64; i++)
			my_common_debase64[(int)my_common_enbase64[i]] = i;
		
		my_common_debase64[(int)'='] = 65; /* special pad */
		
		my_common_base64_init = 1;
	}
	
	b = &buf[0];
	be = &buf[sizeof(buf)];
	
	while (r != re && *r != '=' && w != we)
	{
		/* Firstly, decode a hunk of the string to binary */
		while (r != re && be - b >= 3)
		{
			lu_quad	tmp;
			
			while (my_common_debase64[(int)*r] == 0x7F && r != re) r++;
			
			/* Invalid byte? */
			if (r == re) break;
			if (*r == '=') break;
			
			tmp = my_common_debase64[(int)*r++];
			tmp <<= 6;
			
			while (my_common_debase64[(int)*r] == 0x7F && r != re) r++;
			
			/* Invalid byte? */
			if (r == re) break;
			if (*r == '=') break;
			
			tmp |= my_common_debase64[(int)*r++];
			tmp <<= 6;
			
			while (my_common_debase64[(int)*r] == 0x7F && r != re) r++;
			
			/* Invalid byte? */
			if (r == re) break;
			
			/* Premature end? */
			if (*r == '=')
			{
				*b++ = (tmp >> 10);
				break;
			}
			
			tmp |= my_common_debase64[(int)*r++];
			tmp <<= 6;
			
			while (my_common_debase64[(int)*r] == 0x7F && r != re) r++;
			if (r == re) break;
			
			if (*r == '=')
			{
				*b++ = (tmp >> 16);
				*b++ = (tmp >> 8);
				break;
			}
			
			tmp |= my_common_debase64[(int)*r++];
			
			*b++ = (tmp >> 16);
			*b++ = (tmp >> 8);
			*b++ = tmp;
		}
		
		/* Now, transform the charset to utf-8 */
		wl = we - w;
		bl = sizeof(buf) - (be - b);
		
		b = &buf[0];
		tmp = iconv(ic, &b, &bl, &w, &wl);
		
		/* Shift any remaining partial bytes in the buffer over */
		memmove(&buf[0], b, bl);
		b = &buf[bl];
		w = we - wl;
		
		if (tmp == (size_t)-1 && errno != EINVAL) break;
	}
	
	return w;
}

void lu_common_decode_header(
	const char*	r,
	char*		out,
	size_t		outlen)
{
	/* Now, the plan is to just pass it through as though it were utf-8
	 * until such a time as we see an =?charset?coding?str?=. If we think
	 * that the string is invalid as an encoding, we will pass it through
	 * unprocessed. This function has no failure case.
	 */
	
	char* w;
	char* e;
	
	const char* rewind;
	const char* close;
	
	char  charset[30];
	char* b;
	
	iconv_t ic;
	
	w = out;
	e = out + outlen - 1; /* leave space for a null */
	
	rewind = r;
	while (1)
	{
		r = rewind;
		
		/* Handle normal string content */
		for (; *r && w != e; r++)
		{
			if (*r == '=' && *(r+1) == '?')
				break;
			*w++ = *r;
		}
		
		if (!*r || w == e) break;
		
		/* If we have to start over (bad coding) do so from here */
		rewind = r+1;
		
		/* fall through to processing charset */
		r += 2;
		
		/* Find the end of the charset */
		b = &charset[0];
		for (; *r && b != &charset[sizeof(charset)-1]; r++)
		{
			if (*r == '?')
				break;
			*b++ = *r;
		}
		
		/* Null terminate the charset */
		*b = 0;
		
		/* Oops - it wasn't really a charset-coded str */
		if (!*r || b == &charset[sizeof(charset)-1])
		{
			*w++ = '=';
			continue;	/* rewind will cover us */
		}
		
		r++;
		if (*(r+1) != '?')
		{	/* Not really encoded properly */
			*w++ = '=';
			continue;
		}
		
		/* Look for the end */
		for (close = r+2; *close; close++)
		{
			if (*close == '?' && *(close+1) == '=')
				break;
			if (*close == ' ' || *close == '\t')
				break;
		}
		
		if (*close != '?')
		{	/* No proper closing sequence */
			*w++ = '=';
			continue;
		}
		
		/* Try to open the charset */
		ic = iconv_open("utf-8", &charset[0]);
		
		/* Is this charset supported? */
		if (ic == (iconv_t)-1)
		{
			*w++ = '=';
			continue;
		}
		
		if ((*r == 'q' || *r == 'Q') && *(r+1) == '?')
		{
			w = my_common_quoted(r+2, close, w, e, ic);
			rewind = close + 2;
		}
		else if ((*r == 'b' || *r == 'B') && *(r+1) == '?')
		{	/* base64 encoded content */
			w = my_common_base64(r+2, close, w, e, ic);
			rewind = close + 2;
		}
		else
		{	/* Not a valid coding, must not be real coded str */
			*w++ = '=';
		}
		
		iconv_close(ic);
	}
	
	*w = 0;
}
