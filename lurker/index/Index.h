/*  $Id: Index.h,v 1.10 2004-08-25 21:39:46 terpstra Exp $
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
#include <sys/types.h>	// off_t

#include <esort.h>
#include <MessageId.h>
#include <ConfigFile.h>

struct Index
{
	DwMessage&	message;
	ESort::Writer*	writer;
	List		list;
	off_t		off;
	long		len;
	
	MessageId	id;
	string		charset;
	string		author_name;
	string		author_email;
	string		subject;
	
	Index(DwMessage& m, ESort::Writer* w, const List& l, off_t o, long x)
	 : message(m), writer(w), list(l), off(o), len(x) { }
	
	int index(bool userdate, time_t envelope, time_t import, bool check, bool& exist);
	
 protected:
	int index_author();
	int index_id(bool userdate, time_t envelope);
	int index_summary(bool check, bool& exist);
	int index_threading();
	int index_control(time_t import);
	int index_entity(DwEntity& e, const string& charset); // decode + utf-8 transform
	int index_keywords(DwEntity& e, const string& parentCharset);
};

#endif
