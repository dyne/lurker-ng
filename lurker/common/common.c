/*  $Id: common.c,v 1.3 2002-02-10 21:00:18 terpstra Exp $
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

/*------------------------------------------------ Public global vars */

message_id lu_common_minvalid = -1;

const char* lu_common_cleanup_id(
	const char* id)
{
	static char buf[80];
	char* w;
	char* e;
	
	while (isspace(*id)) id++;
	if (*id == '<') id++;
	
	w = &buf[0];
	e = &buf[sizeof(buf) - 1];
	
	while (w != e)
	{
		if (isspace(*id) || *id == '>') break;
		*w++ = *id++;
	}
	
	*w = 0;
	return &buf[0];
}
