/*  $Id: mindex.c,v 1.1.1.1 2002-01-21 00:03:06 terpstra Exp $
 *  
 *  mindex.c - output results from a mindex/ lookup
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

#include "config.h"
#include "handler.h"

int lu_mindex_handler(char* parameter)
{
	FILE* f;
	
	if ((f = lu_render_open(parameter)) == 0)
		return -1;
	
	fprintf(f, "This is an example page");
	
	return lu_render_close(f);
}
