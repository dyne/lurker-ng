/*  $Id: common.h,v 1.10 2002-05-29 08:09:52 terpstra Exp $
 *  
 *  common.h - common definitions and types for all tools
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

/*------------------------------------------------ System setup methods */

#include "system.h"

#define STORAGE			PACKAGE "d"
#define RENDER			PACKAGE ".cgi"

#define DEFAULT_CONFIG_FILE	SYSCONFDIR    "/" PACKAGE ".conf"
#define	DEFAULT_PID_FILE	LOCALSTATEDIR "/" PACKAGE ".pid"

#define WORD_BREAKS		" !\"'()*,;<>[]^`{|}"

#if SIZEOF_CHAR == 1
typedef unsigned char lu_byte;
#else
#error Need a byte type
#endif

#if SIZEOF_SHORT == 2
typedef unsigned short lu_word;
#elif SIZEOF_INT == 2
typedef unsigned int lu_word;
#else
#error Need a word type
#endif

#if SIZEOF_INT == 4
typedef unsigned int lu_quad;
#elif SIZEOF_LONG == 4
typedef unsigned long lu_quad;
#else
#error Need a quad type
#endif

#if SIZEOF_LONG == 8
typedef unsigned long lu_addr;
#elif SIZEOF_LONG_LONG == 8
typedef unsigned long long lu_addr;
#else
#error Need an 64bit type
#endif

#ifndef HAVE_TIME_T
typedef long time_t;
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h> 
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_SYSLOG_H
#include <syslog.h>
#else
#define syslog(x, y, ...)
#endif

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifndef assert
#define assert(x) do { if (!x) { printf("\nASSERT FAILURE: %s:%i: '%s'\n", __FILE__, __LINE__, #x); exit(1); } } while (0)
#endif

#ifdef ICONV_CONST
#define ICONV_CAST	const char**
#else
#define ICONV_CAST	char**
#endif

/*------------------------------------------------ Public types */

typedef lu_quad message_id;

/*------------------------------------------------ Public global vars */

extern message_id lu_common_minvalid;

/*------------------------------------------------ Public helpers */

/* Decode a message hreader into unescaped utf-8.
 * This deals with =?charset?coding?str?= escaping as well as broken mail
 * clients which mistakenly use the same charset they used for the body.
 */
void lu_common_decode_header(
	const char*	r,
	char*		out,
	size_t		outlen,
	const char*	default_coding);

const char* lu_common_charset_maps(
	const char*	charset);
