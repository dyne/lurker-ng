/*  $Id: wbuffer.c,v 1.1 2002-02-03 03:10:53 terpstra Exp $
 *  
 *  wbuffer.c - Implements a buffering system that delays appends to the flatfile
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

#define _GNU_SOURCE
// #define DEBUG 1

#include "common.h"
#include "io.h"

#include "flatfile.h"
#include "wbuffer.h"

/*------------------------------------------------- Public component methods */

int lu_wbuffer_init()
{
	return 0;
}

int lu_wbuffer_open()
{
	return 0;
}

int lu_wbuffer_sync()
{
	return 0;
}

int lu_wbuffer_close()
{
	return 0;
}

int lu_wbuffer_quit()
{
	return 0;
}

/*-------------------------------------------------- Buffer methods */

int lu_wbuffer_append(
	const char* keyword,
	message_id  msg)
{
	return lu_flatfile_write_keyword_block(
		keyword, &msg, 1);
}

int lu_wbuffer_flush(
	const char* keyword)
{
	return 0;
}
