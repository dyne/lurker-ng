/*
 * $Id: main.c,v 1.6 2002-02-25 06:10:38 terpstra Exp $
 *  
 *  main.c - debugging test for message parsing routines.
 *  
 *  Copyright 2002 Chris L. Bond.  All rights reserved.
 *  
 *  License: GPL
 *  
 *  Authors: 'Chris L. Bond' <cbond@twistedcircus.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "common.h"

int
main(int argc, char **argv)
{
	char buf[80];
	lu_common_decode_header(
		"peanuts =?euc-jp?q?aa=A4=A8bb?= your death",
		&buf[0],
		20,
		0);
	
	puts(&buf[0]);
	return 0;
}
