/*  $Id: breader.c,v 1.1 2002-02-03 03:10:53 terpstra Exp $
 *  
 *  breader.c - Knows how to use the abstracted read interface for buffered access
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
#include "breader.h"

/*------------------------------------------------- Public component methods */

int lu_breader_init()
{
	return 0;
}

int lu_breader_open()
{
	return 0;
}

int lu_breader_sync()
{
	return 0;
}

int lu_breader_close()
{
	return 0;
}

int lu_breader_quit()
{
	return 0;
}
