/*
 * $Id: main.c,v 1.4 2002-01-28 06:23:41 cbond Exp $
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

#include "message.h"

int
main(int argc, char **argv)
{
	return 0;
#if 0
	int fd, count;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <mailbox> <messages>\n", *argv);
		return (1);
	}

	/*
	 * Open a mailbox and parse the first N messages.
	 */
	if ((fd = open(argv[1], O_RDONLY)) < 0) {
		perror("open");
		return (1);
	}

	count = atoi(argv[2]);
	fprintf(stderr, "Parsing %d message(s) from `%s'...\n", count, argv[1]);

	while (count-- > 0) {
		struct msg *input;

		input = mail_parse(fd, lseek(fd, 0, SEEK_CUR));
		if (input) {
			index_traverse(input, input->body);
			mail_free(input);
		}
	}

	return (0);
#endif
}
