/*
 * $Id: message.c,v 1.12 2002-01-28 12:05:49 cbond Exp $
 *  
 *  message.c - parse mail.
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

#include <sys/types.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "message.h"

static char *strnstr(const char *, const char *, size_t);
static int message_offset(char *, size_t *);

/*
 * Parse a message at the specified offset.  (We'll have
 * to determine its length and such first, however.)
 */
struct msg *
mail_parse(int fd, off_t offset)
{
	char *buffer, *end, *off;
	size_t maplog, offc;
	struct msg *out;
	struct stat sb;

	if (fstat(fd, &sb) < 0)
		return (NULL);

	if ((out = (struct msg *)calloc(1, sizeof(struct msg))) == NULL)
		return (NULL);

	/*
	 * Round offset down to the nearest page size multiple.
	 */
	offc = offset & ~(getpagesize() - 1);

	buffer = end = NULL;

	/*
	 * Map larger and larger segments of the file, searching
	 * for the end of the message.  (We will assume, perhaps
	 * incorrectly, that `\n^From' denotes the start of a new
	 * message.)  Start by mapping 2^10 bytes, doubling it for
	 * each iteration.
	 */
	for (maplog = 1 << 10; end == NULL; maplog <<= 1) {
		if (buffer)
			munmap((void *)buffer, maplog >> 1);

		if (maplog + offset >= sb.st_size)
			maplog = sb.st_size - offset;

		out->region = maplog + offset - offc;
		buffer = (char *)mmap(NULL, out->region, PROT_READ | PROT_WRITE,
					MAP_PRIVATE, fd, offc);
		if (buffer == MAP_FAILED)
			return (NULL);

		out->buffer = buffer + (offset - offc);
		out->aaddr = buffer;

		/*
		 * If the end of file has been reached, assume that's
		 * the end of this message as well.
		 */
		if (maplog > 1 << 10)
			end = strnstr(out->buffer + (maplog / 2), "\nFrom ",
				maplog / 2);
		else
			end = strnstr(out->buffer, "\nFrom ", maplog);
		if (end == NULL && maplog >= sb.st_size - offset)
			end = out->buffer + maplog;
	}

	/*
	 * Seek to the end of this message.
	 */
	if (lseek(fd, end - out->buffer + 1, SEEK_CUR) < 0)
		return (NULL);

	end[0] = 0;
	if (message_offset(out->buffer, &out->offset)) {
		mail_free(out);
		return (NULL);
	}

	/*
	 * ``off'' points to the beginning of the actual message,
	 * just past the headers.  Parse the headers and the MIME
	 * structure and so forth.
	 */
	off = out->buffer + out->offset;
	INIT(&out->bss, mail_string, off, (size_t)(end - off));
	rfc822_parse_msg(&out->env, &out->body, out->buffer,
		(size_t)(off - out->buffer), &out->bss, "localhost", 0);

	return (out);
}

/*
 * Calculate the offset of the actual message.
 */
static int
message_offset(char *buffer, size_t *offset)
{
	char *calc;

	if ((calc = strstr(buffer, "\r\n\r\n")) != NULL)
		calc += 4;
	else
		if ((calc = strstr(buffer, "\n\n")) != NULL)
			calc += 2;
	*offset = (size_t)(calc - buffer);
	return (calc == NULL);
}

/*
 * Pick out a part of a message, decoding it if necessary.
 * [If the result is allocated with fs_get() by c-client,
 *  nfree is set to 1 and the block must be deallocated by
 *  fs_give().  Otherwise no deallocation is necessary.]
 */
char *
mail_select(struct msg *in, struct mail_bodystruct *body, size_t *len, int *nfr)
{
	char *(*output)(unsigned char *, unsigned long, unsigned long *);
	char *input;

	switch ((int)body->encoding) {
		case ENCQUOTEDPRINTABLE:
			output = (char *(*)(unsigned char*, unsigned long,
					unsigned long*))rfc822_qprint;
			break;
		case ENCBASE64:
			output = (char *(*)())rfc822_base64;
			break;
		default:
			output = NULL;
			break;
	}

	input = in->buffer + in->offset + body->contents.offset;
	if (output) {
		nfr[0] = 1;
		return (output((unsigned char *)input, body->contents.text.size,
				(unsigned long *)len));
	}
	nfr[0] = 0;
	len[0] = body->contents.text.size;
	return (input);
}

/*
 * Free a message structure and the associated mapping.
 */
void
mail_free(struct msg *in)
{
	munmap((void *)in->aaddr, in->region);
	if (in->body)
		mail_free_body(&in->body);
	if (in->env)
		mail_free_envelope(&in->env);
	free(in);
}

/*
 * Find substring s2 in s, which is not NULL-terminated.
 */
char *
strnstr(const char *s, const char *s2, size_t len)
{
	const char *orig;
	size_t len2;

	len2 = strlen(s2);
	for (orig = s; len > s - orig; ++s)
		if (*s == *s2 && strncmp(s + 1, s2 + 1, len2 - 1) == 0)
			return ((char *)s);
	return (NULL);
}
