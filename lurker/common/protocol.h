/*  $Id: protocol.h,v 1.6 2002-02-22 00:51:43 terpstra Exp $
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
#define LU_PROTO_SEARCH		"search "
#define	LU_PROTO_LISTS		"lists "

/* Messages to a page */
#define LU_PROTO_INDEX	20
