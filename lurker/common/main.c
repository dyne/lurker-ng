/*
 * Copyright 2002 Chris L. Bond.  All rights reserved.
 * $Id: main.c,v 1.2 2002-01-28 06:11:27 cbond Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>

#include "message.h"

int
main(int argc, char **argv)
{
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
}
