/*  $Id: Index.h,v 1.2 2003-04-21 18:25:32 terpstra Exp $
 *  
 *  Index.h - Insert all the keywords from the given email
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
 
#ifndef INDEX_H
#define INDEX_H

#include <mimelib/message.h>

#include <esort.h>
#include <MessageId.h>
#include <Config.h>

struct Index
{
	DwMessage	message;
	ESort::Writer*	writer;
	List		list;
	off_t		off;
	long		len;
	
	MessageId	id;
	string		charset;
	string		author_name;
	string		author_email;
	string		subject;
	
	Index(const DwString& str, ESort::Writer* w, const List& l, off_t o, long x)
	 : message(str), writer(w), list(l), off(o), len(x) { }
	
	int index(time_t envelope);
	
 protected:
	int index_author();
	int index_id(time_t envelope);
	int index_summary();
	int index_threading();
	int index_control();
	int index_keywords(DwEntity& e);
};

#endif
