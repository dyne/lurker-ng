/*  $Id: protocol.h,v 1.3 2002-02-10 08:20:26 terpstra Exp $
 *  
 *  protocol.h - What do we use to communicate
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

#define LU_PROTO_ENDREQ		'\n'
#define LU_PROTO_GETMSG		"getmsg "
#define LU_PROTO_MINDEX		"mindex "

typedef struct Lu_Proto_Message_T
{
	lu_word		list;
	lu_word		mbox;
	
	char 		list_name[80];
	char		list_address[80];
	char		list_desc[200];
	char		mbox_name[100];
	
	lu_quad		timestamp;
	message_id	in_reply_to;
	message_id	thread;
	
	lu_word		replys;		/* How many replys are there */
	
	message_id	reply[0];	/* All the replys to this message */
	char		body[0];	/* The message follows - terminated by
					 * the connection closing.
					 */
} Lu_Proto_Message;

/* Messages to a page */
#define LU_PROTO_INDEX	20
