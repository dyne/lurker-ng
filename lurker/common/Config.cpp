/*  $Id: Config.cpp,v 1.5 2003-06-04 12:35:28 terpstra Exp $
 *  
 *  Config.cpp - Knows how to load the config file
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

#include "Config.h"
#include "XmlEscape.h"

#include <fstream>
#include <iostream>

using namespace std;

void prune_back(string& line)
{
	// Trim off eol and whitespace
	string::size_type whitespace = line.length();
	while (whitespace > 0 &&
		(line[whitespace-1] == ' '  ||
		 line[whitespace-1] == '\r' ||
		 line[whitespace-1] == '\n' ||
		 line[whitespace-1] == '\t'))
		whitespace--;
	
	line.resize(whitespace);
}

string::size_type skip_front(const string& line, string::size_type x = 0)
{
	// Trim off eol and whitespace
	for (; x < line.length(); ++x)
		if (line[x] != ' '  &&
		    line[x] != '\r' &&
		    line[x] != '\n' &&
		    line[x] != '\t')
			break;
	
	return x;
}

int Config::load(const char* file)
{
	ifstream f(file);
	if (!f.is_open())
	{
		error << file << ":open: could not open!" << endl;
		return -1;
	}
	
	string line;
	bool ok = true;
	int c = 0;
	
	string val, key;
	
	while (getline(f, line))
	{
		// Increment line number
		++c;
		
		// Trim off the comments
		string::size_type comment = line.find('#');
		if (comment != string::npos) line.resize(comment);
		
		// Clear off trailing whitespace
		prune_back(line);
		
		// skip empty lines
		if (line.length() == 0) continue;
		
		string::size_type eq = line.find('=');
		if (eq == string::npos)
		{	// this line continues the previous one.
			if (key == "")
			{
				error << "No key for value '" << line << "'!" << endl;
				ok = false;
			}
			else
			{
				string::size_type fe = skip_front(line);
				val.append(" ");
				val.append(line, fe, string::npos);
			}
		}
		else
		{
			if (key != "" && process_command(key, val) != 0) ok = false;
			
			string::size_type leadin = skip_front(line);
			key.assign(line, leadin, eq-leadin);
			val.assign(line, skip_front(line, eq+1), string::npos);
			prune_back(key);
		}
	}
	
	if (key == "")
	{
		error << "No values set by config file!" << endl;
		ok = false;
	}
	
	if (key != "" && process_command(key, val) != 0) ok = false;
	
	if (!ok) return -1;
	return 0;
}

bool isSimple(const string& s)
{
	string::size_type x;
	for (x = 0; x < s.length(); ++x)
	{
		char y = s[x];
		if (y >= 'A' && y <= 'Z') continue;
		if (y >= 'a' && y <= 'z') continue;
		if (y >= '0' && y <= '9') continue;
		if (y == '.' || y == '-' || y == '_') continue;
		return false;
	}
	
	return true;
}

int Config::process_command(const string& key, const string& val)
{
//	cout << key << "-" << val << endl;
	
	string::size_type len = string::npos;
	
	if (key == "list")
	{
		len = 32;
		if (!isSimple(val) || val.length() == 0)
		{
			error << "List id '" << val << "' is not a simple string!" << endl;
			return -1;
		}
		
		lists[val].mbox = val;
		list = &lists[val];
	}
	else if (key == "title")
	{
		len = 40;
		
		if (!list)
		{
			error << "No list has been defined for title '" << val << "'!" << endl;
			return -1;
		}
		
		list->title = val;
	}
	else if (key == "address")
	{
		if (!list)
		{
			error << "No list has been defined for address '" << val << "'!" << endl;
			return -1;
		}
		
		list->address = val;
	}
	else if (key == "link")
	{
		if (!list)
		{
			error << "No list has been defined for address '" << val << "'!" << endl;
			return -1;
		}
		
		list->link = val;
	}
	else if (key == "description")
	{
		if (!list)
		{
			error << "No list has been defined for address '" << val << "'!" << endl;
			return -1;
		}
		
		list->description = val;
	}
	else if (key == "dbdir")
	{
		dbdir = val;
	}
	else if (key == "admin_name")
	{
		admin_name = val;
	}
	else if (key == "admin_address")
	{
		admin_address = val;
	}
	else if (key == "archive")
	{
		archive = val;
	}
	else if (key == "xslt")
	{
		xslt = val;
	}
	else
	{
		error << "Unknown configuration directive '" << key << "'!" << endl;
		return -1;
	}
	
	if (val.length() > len)
	{
		error << "Value '" << val << "' is too long for directive '" << key << "'!" << endl;
		return -1;
	}
	
	return 0;
}

ostream& operator << (ostream& o, const List& l)
{
	o << "<list>"
	  << "<id>" << l.mbox << "</id>";
	
	if (l.link.length() > 0)
		o << "<link>" << xmlEscape << l.link << "</link>";
		
	if (l.description.length() > 0)
		o << "<description>" << xmlEscape << l.description << "</description>";
	
	o  << "<email";
	if (l.address.length() > 0)
		o << " address=\"" << xmlEscape << l.address << "\"";
	if (l.title.length() > 0)
		o << " name=\"" << xmlEscape << l.title << "\"";
	else
		o << " name=\"" << l.mbox << "\"";
	
	o << "/></list>";
	
	return o;
}

ostream& operator << (ostream& o, const Config& c)
{
	// expire = time(0) + 60*5; // 5 minute cache
	//
	// tm = gmtime(&expire);
	// strftime(&timebuf[0], sizeof(timebuf),
	//	"%a, %d %b %Y %H:%M:%S GMT", tm);
	
	o << "<server>"
	  << "<version>" << VERSION << "</version>"
	  << "<doc-url>" << c.docUrl << "</doc-url>"
	  << "<cgi-url>" << c.cgiUrl << "</cgi-url>"
	  << "<archive>";
	
	if (c.archive.length() > 0)
		o << xmlEscape << c.archive;
	else	o << "Some Mailing List Archive";
	
	o << "</archive><email";
	if (c.admin_address.length() > 0)
		o << " address=\"" << xmlEscape << c.admin_address << "\"";
	if (c.admin_name.length() > 0)
		o << " name=\"" << xmlEscape << c.admin_name << "\"";
	o << "/></server>";
	
	return o;
}
