/*  $Id: KeyReader.cpp,v 1.1 2003-05-02 11:18:41 terpstra Exp $
 *  
 *  KeyReader.cpp - Helper which can stream keywords
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
 
#include "KeyReader.h"
#include <Keys.h>

#include <cerrno>
#include <cstring>

KeyReader::KeyReader(Reader* db, Direction dir, const string& kw, const MessageId& id)
 : walker(db->seek(LU_KEYWORD + kw + '\0', id.raw(), dir)),
   skip(1+kw.length()+1)
{
}

string KeyReader::pull(int n, vector<Summary>& o)
{
	int ok = 0;
	while (n && (ok = walker->advance()) != -1)
	{
		if (walker->key.length() != skip + 8)
			return "corrupt keyword entry";
		
		MessageId id(walker->key.c_str() + skip, 1);
		o.push_back(id);
		--n;
	}
	if (ok == -1 && errno != 0)
		return string("Walker::advance:") + strerror(errno);
	
	return "";
}
