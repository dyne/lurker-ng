/*  $Id: mbox.h,v 1.4 2002-02-21 03:51:38 terpstra Exp $
 *  
 *  mbox.h - Knows how to follow mboxes for appends and import messages
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

#include <c-client/mail.h>
#include <c-client/rfc822.h>
#include <c-client/fs.h>

/*------------------------------------------------- Public types */

struct Lu_Mbox_Message
{
	struct	mail_envelope*		env;
	struct	mail_bodystruct*	body;
	STRING				bss;
	char*				buffer;
};

/*------------------------------------------------- Public methods */

extern char* lu_mbox_select_body(struct Lu_Mbox_Message *, 
	BODY *, size_t *, int *);

/*------------------------------------------------- Public component methods */

extern int lu_mbox_init (void);
extern int lu_mbox_open (void);
extern int lu_mbox_sync (void);
extern int lu_mbox_close(void);
extern int lu_mbox_quit (void);
