/*
 * Copyright 2002 Chris L. Bond.  All rights reserved.
 * $Id: stub.c,v 1.2 2002-01-28 06:15:47 terpstra Exp $
 */

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
