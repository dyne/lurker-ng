/*
 * $Id: message.h,v 1.3 2002-01-28 06:23:41 cbond Exp $
 *  
 *  message.h - header file for mail parsing routines.
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

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <sys/types.h>
#include <c-client/mail.h>
#include <c-client/rfc822.h>
#include <c-client/fs.h>

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
