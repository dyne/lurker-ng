/*  $Id: mindex.cpp,v 1.4 2003-04-25 20:12:32 terpstra Exp $
 *  
 *  mindex.cpp - Handle a mindex/ command
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

#include <iostream>
#include <cerrno>

#include <MessageId.h>
#include <Keys.h>

#include "commands.h"
#include "Keyword.h"
#include "Cache.h"

using namespace std;

int mindex_format_error(const string& param)
{
	cout << "Status: 200 OK\r\n";
	cout <<	"Content-Type: text/html\r\n\r\n";
	cout << error(_("Bad request"), param,
		_("The given parameter was not of the correct format. "
		  "A mindex request must be formatted like: "
		  "mindex/list@YYYYMMDD.HHMMSS.hashcode.xml where list "
		  "is the id of an indexed mailing list."));
	return 1;
}

int handle_mindex(const Config& cfg, ESort::Reader* db, const string& param)
{
	string::size_type o = param.find('@');
	if (o == string::npos || param.length() - o < 1 + 15 + 4)
		return mindex_format_error(param);
	
	MessageId id(param.c_str()+o+1);
	
	if (id.timestamp() == 0)
		return mindex_format_error(param);
	
	string listn(param, 0, o);
	
	if (cfg.lists.find(listn) == cfg.lists.end())
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("No such list"), listn,
			_("The specified mailing list is not available in this "
			  "archive. Perhaps you misspelled it or went to the "
			  "wrong server?"));
		return 1;
	}
	
	const List& list = cfg.lists.find(listn)->second;
	
	// Right! Everything the user did is ok.
	
	vector<Summary> forward, backward, queue;
	
	Keyword backwardk(db, Backward, LU_KEYWORD_LIST + list.mbox, id);
	Keyword forwardk (db, Forward,  LU_KEYWORD_LIST + list.mbox, id);
	
	string ok;
	
	if ((ok = forwardk .pull(35, forward )) != "" ||
	    (ok = backwardk.pull(35, backward)) != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database mindex seek failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "furth assistence."));
		return 1;
	}
	
	vector<Summary>::size_type left, right, i;
	if (forward.size() + backward.size() < 20)
	{
		left = backward.size();
		right = forward.size();
	}
	else if (forward.size() < 10)
	{
		right = forward.size();
		left = 20 - right;
	}
	else if (backward.size() < 10)
	{
		left = backward.size();
		right = 20 - left;
	}
	else
	{
		left = right = 10;
	}
	
	assert (left  <= backward.size());
	assert (right <= forward .size());
	
	for (i = left; i > 0; --i)  queue.push_back(backward[i-1]);
	for (i = 0; i < right; ++i) queue.push_back(forward[i]);
	
	for (i = 0; i < queue.size(); ++i)
		if ((ok = queue[i].load(db)) != "")
			break;
	
	if (ok != "")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html\r\n\r\n";
		cout << error(_("Database mindex pull failure"), ok,
			_("Something internal to the database failed. "
			  "Please contact the lurker user mailing list for "
			  "further assistence."));
		return 1;
	}
	
	Cache cache(cfg, "mindex", param);
	
	cache.o << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
		<< "<?xml-stylesheet type=\"text/xsl\" href=\"../fmt/render.xsl\"?>\n"
		<< "<mindex>\n"
		<< " " << cfg << "\n"
		<< " " << list << "\n";
	
	if (right < forward.size())
	{	// we need a next link
		i = min(right+9, forward.size()-1);
		MessageId nd(forward[i].id());
		nd.increment(); // hope that it doesn't exist (-> skips one)
		cache.o << " <next>" << nd.serialize() << "</next>\n";
	}
	
	if (left < backward.size())
	{	// we need a prev link
		i = min(left+10, backward.size()-1);
		MessageId pd(backward[i].id());
		pd.increment();
		cache.o << " <prev>" << pd.serialize() << "</prev>\n";
	}
	
	for (i = 0; i < queue.size(); ++i)
	{
		if (queue[i].id() == id) 
			cache.o << " <row selected=\"true\">";
		else	cache.o << " <row>";
		cache.o << queue[i] << "</row>\n";
	}
	
	cache.o << "</mindex>\n";
	
	return 0;
}
