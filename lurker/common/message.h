/*
 * Copyright 2002 Chris L. Bond.  All rights reserved.
 * $Id: message.h,v 1.1 2002-01-28 06:03:46 cbond Exp $
 */

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <sys/types.h>
#include <mail.h>
#include <rfc822.h>
#include <fs.h>

/*
 * The bare structure of a message, as well as the envelope,
 * is contained here.  The main purpose is to store the
 * offsets of message ``parts,'' coupled with their types.
 */
struct	msg {
	struct	mail_envelope *env;
	struct	mail_bodystruct *body;
	size_t	offset;			/* Offset of the actual message */
	size_t	region;			/* Size of the buffer */
	char	*buffer;		/* Message contents */
	STRING	bss;
};

/* Parse the message at the specified offset. */
extern struct msg *mail_parse(int, off_t);

/* Return the selected part of a message, decoding if necessary. */
extern char *mail_select(struct msg *, BODY *, size_t *, int *);

/* Free a msg structure and the region it's mmapped. */
extern void mail_free(struct msg *);

/* Traverse a message structure and index the plaintext words. */
extern void index_traverse(struct msg *, struct mail_bodystruct *);

#endif
