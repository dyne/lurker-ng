/*  $Id: Keyword.cpp,v 1.3 2003-04-25 10:13:52 terpstra Exp $
 *  
 *  Keyword.cpp - Helper which can stream keywords
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
 
#include "Keyword.h"

#include <cerrno>
#include <cstring>

Keyword::Keyword(Reader* r, bool f, const string& p, const string& o)
 : reader(r), walker(0), forward(f), eof(false), prefix(p), offset(o)
{
}

Keyword::Keyword(const Keyword& o)
 : reader(o.reader), walker(0), forward(o.forward), eof(o.eof), prefix(o.prefix), 
   offset(o.walker.get()?o.walker->key:o.offset)
{
}

Keyword::~Keyword()
{
}

void Keyword::swap(Keyword& o)
{
	std::swap(reader, o.reader);
	std::swap(walker, o.walker);
	std::swap(forward,o.forward);
	std::swap(eof,    o.eof);
	std::swap(prefix, o.prefix);
	std::swap(offset, o.offset);
}

Keyword& Keyword::operator = (const Keyword& o)
{
	Keyword(o).swap(*this);
	return *this;
}

string Keyword::pull(int n, vector<Summary>& o)
{
	if (eof || n == 0) return "";
	
	if (walker.get() == 0)
		walker = reader->seek(prefix + offset, true);
		// always succeeds
	
	while (n != 0)
	{
		int j = walker->advance();
		if (j == -1)
			return string("Walker::advance:") + strerror(errno);
		
		if (j < (int)prefix.length() &&
			walker->key.substr(0, prefix.length()) != prefix)

		{
			eof = true;
			return "";
		}
		
		if (walker->key.length() < prefix.length() + 8)
			return "corrupt keyword entry";
		
		MessageId id(walker->key.c_str() + prefix.length(), forward);
		o.push_back(id);
		--n;
	}
	
	return "";
}
