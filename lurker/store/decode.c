/*  $Id: decode.c,v 1.2 2002-06-12 22:50:47 terpstra Exp $
 *  
 *  decode.c - decode definitions and types for all tools
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

#include "common.h"
#include "decode.h"

#include <ctype.h>
#include <string.h>
#include <iconv.h>

/*------------------------------------------------ Private global vars */

static const char my_decode_enbase64[] = 
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static int  my_decode_base64_init = 0;
static char my_decode_debase64[256];

/*------------------------------------------------ Public helper methods */

const char* lu_decode_id(
	const char* id)
{
	static char buf[80];
	char* w;
	char* e;
	const char* scan;
	
	/* Try to find a '<' in preference to anything else */
	for (scan = id; *scan; scan++)
	{
		if (*scan == '<')
		{
			id = scan+1;
		}
	}
	
	while (isspace(*id)) id++;
	
	w = &buf[0];
	e = &buf[sizeof(buf) - 1];
	
	while (*id && w != e)
	{
		if (isspace(*id) || *id == '>') break;
		
		/* We can handle anything in filenames but a / */
		if (*id == '/')
		{
			*w++ = '_';
			id++;
		}
		else
		{
			*w++ = *id++;
		}
	}
	
	*w = 0;
	return &buf[0];
}

static char* my_decode_quoted(
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
		tmp = iconv(ic, (ICONV_CONST char**)&b, &bl, &w, &wl);
		
		/* Shift any remaining partial bytes in the buffer over */
		memmove(&buf[0], b, bl);
		b = &buf[bl];
		w = we - wl;
		
		if (tmp == (size_t)-1 && errno != EINVAL) break;
	}
	
	return w;
}

static char* my_decode_base64(
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
	
	if (!my_decode_base64_init)
	{
		memset(&my_decode_debase64, 0x7F, 256);
		
		for (i = 0; i < 64; i++)
			my_decode_debase64[(int)my_decode_enbase64[i]] = i;
		
		my_decode_debase64[(int)'='] = 65; /* special pad */
		
		my_decode_base64_init = 1;
	}
	
	b = &buf[0];
	be = &buf[sizeof(buf)];
	
	while (r != re && *r != '=' && w != we)
	{
		/* Firstly, decode a hunk of the string to binary */
		while (r != re && be - b >= 3)
		{
			lu_quad	tmp;
			
			while (my_decode_debase64[(int)*r] == 0x7F && r != re) r++;
			
			/* Invalid byte? */
			if (r == re) break;
			if (*r == '=') break;
			
			tmp = my_decode_debase64[(int)*r++];
			tmp <<= 6;
			
			while (my_decode_debase64[(int)*r] == 0x7F && r != re) r++;
			
			/* Invalid byte? */
			if (r == re) break;
			if (*r == '=') break;
			
			tmp |= my_decode_debase64[(int)*r++];
			tmp <<= 6;
			
			while (my_decode_debase64[(int)*r] == 0x7F && r != re) r++;
			
			/* Invalid byte? */
			if (r == re) break;
			
			/* Premature end? */
			if (*r == '=')
			{
				*b++ = (tmp >> 10);
				break;
			}
			
			tmp |= my_decode_debase64[(int)*r++];
			tmp <<= 6;
			
			while (my_decode_debase64[(int)*r] == 0x7F && r != re) r++;
			if (r == re) break;
			
			if (*r == '=')
			{
				*b++ = (tmp >> 16);
				*b++ = (tmp >> 8);
				break;
			}
			
			tmp |= my_decode_debase64[(int)*r++];
			
			*b++ = (tmp >> 16);
			*b++ = (tmp >> 8);
			*b++ = tmp;
		}
		
		/* Now, transform the charset to utf-8 */
		wl = we - w;
		bl = sizeof(buf) - (be - b);
		
		b = &buf[0];
		tmp = iconv(ic, (ICONV_CONST char**)&b, &bl, &w, &wl);
		
		/* Shift any remaining partial bytes in the buffer over */
		memmove(&buf[0], b, bl);
		b = &buf[bl];
		w = we - wl;
		
		if (tmp == (size_t)-1 && errno != EINVAL) break;
	}
	
	return w;
}

const char* lu_decode_charset(
	const char* charset)
{
	if (	(charset[0] == 'k' || charset[0] == 'K') &&
		(charset[1] == 's' || charset[1] == 'S') &&
		(charset[2] == '_') &&
		(charset[3] == 'c' || charset[3] == 'C') &&
		(charset[4] == '_') &&
		(charset[5] == '5') &&
		(charset[6] == '6') &&
		(charset[7] == '0') &&
		(charset[8] == '1'))
	{
		return "euc-kr";
	}
	
	return charset;
}

void lu_decode_header(
	const char*	r,
	char*		out,
	size_t		outlen,
	const char*	default_coding)
{
	/* Now, the plan is to just pass it through as though it were utf-8
	 * until such a time as we see an =?charset?coding?str?=. If we think
	 * that the string is invalid as an encoding, we will pass it through
	 * unprocessed. This function has no failure case.
	 */
	
	char* w;
	char* e;
	
	const char* start;
	const char* rewind;
	const char* close;
	
	char  charset[30];
	char* b;
	
	size_t	wlen, rlen; 
	
	iconv_t dc;
	iconv_t ic;
	
	if (!default_coding)
		default_coding = "iso-8869-1";
	
	dc = iconv_open("utf-8", default_coding);
	if (dc == (iconv_t)-1)
		dc = iconv_open("utf-8", "iso-8859-1");
	
	w = out;
	e = out + outlen - 1; /* leave space for a null */
	
	rewind = r;
	while (1)
	{
		start = r = rewind;
		
		/* Handle normal string content */
		for (; *r; r++)
		{
			if (*r == '=' && *(r+1) == '?')
				break;
		}
		
		rlen = r - start;
		wlen = e - w;
		iconv(dc, (ICONV_CONST char**)&start, &rlen, &w, &wlen);
		
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
		ic = iconv_open("utf-8", lu_decode_charset(&charset[0]));
		
		/* Is this charset supported? */
		if (ic == (iconv_t)-1)
		{
			syslog(LOG_WARNING, "Unknown header coding: %s\n", &charset[0]);
			ic = iconv_open("utf-8", "iso-8859-1");
		}
		
		if ((*r == 'q' || *r == 'Q') && *(r+1) == '?')
		{
			w = my_decode_quoted(r+2, close, w, e, ic);
			rewind = close + 2;
		}
		else if ((*r == 'b' || *r == 'B') && *(r+1) == '?')
		{	/* base64 encoded content */
			w = my_decode_base64(r+2, close, w, e, ic);
			rewind = close + 2;
		}
		else
		{	/* Not a valid coding, must not be real coded str */
			*w++ = '=';
		}
		
		iconv_close(ic);
	}
	
	*w = 0;
	iconv_close(dc);
}
