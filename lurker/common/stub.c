/*
 * $Id: stub.c,v 1.4 2002-05-11 19:24:59 terpstra Exp $
 *  
 *  stub.c - stub out c-client logging interfaces.
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE

#include <sys/param.h>
#include <sys/types.h>

#include <c-client/mail.h>

/*
 * We'll stub out c-client's ``callback'' functions, since we
 * don't need them anyway.  (What a horrible interface.)
 */
void mm_dlog	(char *p1) {}
void mm_fatal	(char *p1) {}
void mm_log	(char *p1, long p2) {}
void mm_status	(MAILSTREAM *p1, char *p2, MAILSTATUS *p3) {}
void mm_searched(MAILSTREAM *p1, unsigned long p2) {}
void mm_exists	(MAILSTREAM *p1, unsigned long p2) {}
void mm_expunged(MAILSTREAM *p1, unsigned long p2) {}
void mm_flags	(MAILSTREAM *p1, unsigned long p2) {}
void mm_list	(MAILSTREAM *p1, int p2, char *p3, long p4) {}
void mm_lsub	(MAILSTREAM *p1, int p2, char *p3, long p4) {}
void mm_login	(NETMBX *p1, char *p2, char *p3, long p4) {}
void mm_notify	(MAILSTREAM *p1, char *p2, long p3) {}
void mm_critical(MAILSTREAM *p1) {}

void mm_nocritical(MAILSTREAM *p1) {}
long mm_diskerror (MAILSTREAM *p1, long p2, long p3)
{
	return (0);
}
