/*  $Id: ConfigFile.cpp,v 1.10 2004-08-26 16:52:10 terpstra Exp $
 *  
 *  ConfigFile.cpp - Knows how to load the config file
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: GPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; version 2.
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
#define _FILE_OFFSET_BITS 64

#include "ConfigFile.h"
#include "XmlEscape.h"

#include <fstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

using namespace std;

Config::Config()
 : list(0), group("lists"), error(), lists(), groups(),
   dbdir(""), 
   archive("Unconfigured Host"),
   admin_name("Unset admin name"),
   admin_address(""),
   xslt("cat -"),
   pgpv_mime("off"),
   pgpv_inline("off"),
   web_cache(true),
   hide_email(false),
   raw_email(true),
   modified(0)
{
}

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

int Config::load(const string& file, bool toplevel)
{
	ifstream f(file.c_str());
	if (!f.is_open())
	{
		error << file << ":open: could not open!" << endl;
		return -1;
	}
	
	struct stat sbuf;
	if (stat(file.c_str(), &sbuf) < 0)
	{
		error << file << ":stat: could not stat!" << endl;
		return -1;
	}
	
	// deal with included file's timestamps
	if (sbuf.st_mtime > modified)
		modified = sbuf.st_mtime;
	
	string dir;
	string::size_type x = file.rfind('/');
	if (x != string::npos) dir.assign(file, 0, x+1);
	
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
			if (key != "" && process_command(key, val, dir) != 0) ok = false;
			
			string::size_type leadin = skip_front(line);
			key.assign(line, leadin, eq-leadin);
			val.assign(line, skip_front(line, eq+1), string::npos);
			prune_back(key);
		}
	}
	
	if (toplevel && key == "")
	{
		error << "No values set by config file '" << file << "'!" << endl;
		ok = false;
	}
	
	if (key != "" && process_command(key, val, dir) != 0) ok = false;
	
	if (!ok) return -1;
	return 0;
}

bool isSimple(const string& s)
{
	string::size_type x;
	for (x = 0; x < s.length(); ++x)
	{
		char y = s[x];
		if (y >= 'a' && y <= 'z') continue;
		if (y >= '0' && y <= '9') continue;
		if (y == '.' || y == '-' || y == '_') continue;
		return false;
	}
	
	return true;
}

int Config::process_command(const string& key, const string& val, const string& dir)
{
//	cout << key << "-" << val << endl;
	
	string::size_type len = string::npos;
	
	if (key == "group")
	{
		len = 32;
		if (!isSimple(val) || val.length() == 0)
		{
			error << "Group id '" << val << "' is not a simple lowercase string!" << endl;
			return -1;
		}
		
		group = val;
	}
	else if (key == "heading")
	{
		len = 40;
		groups[group].heading = val;
	}
	else if (key == "list")
	{
		len = 32;
		if (!isSimple(val) || val.length() == 0)
		{
			error << "List id '" << val << "' is not a simple lowercase string!" << endl;
			return -1;
		}
		
		if (lists.find(val) != lists.end())
		{
			error << "List id '" << val << "' is already defined." << endl;
			return -1;
		}
		
		groups[group].members.insert(val);
		list = &lists[val];
		list->mbox = val;
		list->group = group;
		list->language = "en";
		list->offline = false;
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
	else if (key == "language")
	{
		if (!list)
		{
			error << "No list has been defined for language '" << val << "'!" << endl;
			return -1;
		}
		
		if (val.length() != 2 || !isSimple(val))
		{
			error << "Language is not a two digit language code!" << endl;
			error << "Regional variants are not relevant for searches." << endl;
			return -1;
		}
		
		list->language = val;
	}
	else if (key == "offline")
	{
		if (!list)
		{
			error << "No list has been defined for offline setting '" << val << "'!" << endl;
			return -1;
		}
		
		if (val == "off" || val == "false")
			list->offline = false;
		else if (val == "on" || val == "true")
			list->offline = true;
		else
		{
			error << "offline must be set to on/off or true/false!" << endl;
			return -1;
		}
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
	else if (key == "pgp_verify_mime")
	{
		pgpv_mime = val;
	}
	else if (key == "pgp_verify_inline")
	{
		pgpv_inline = val;
	}
	else if (key == "web_cache")
	{
		if (val == "off" || val == "false")
			web_cache = false;
		else if (val == "on" || val == "true")
			web_cache = true;
		else
		{
			error << "web_cache must be set to on/off or true/false!" << endl;
			return -1;
		}
	}
	else if (key == "hide_email")
	{
		if (val == "off" || val == "false")
			hide_email = false;
		else if (val == "on" || val == "true")
			hide_email = true;
		else
		{
			error << "hide_email must be set to on/off or true/false!" << endl;
			return -1;
		}
	}
	else if (key == "raw_email")
	{
		if (val == "off" || val == "false")
			raw_email = false;
		else if (val == "on" || val == "true")
			raw_email = true;
		else
		{
			error << "raw_email must be set to on/off or true/false!" << endl;
			return -1;
		}
	}
	else if (key == "include")
	{
		string file;
		
		if (val[0] == '/')
			file = val;
		else	file = dir + val;
		
		if (load(file, false) != 0)
			return -1;
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
	  << "<id>" << l.mbox << "</id>"
	  << "<group>" << l.group << "</group>"
	  << "<language>" << l.language << "</language>";
	
	if (l.offline)
		o << "<offline/>";
	
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
	
	char year[40];
	time_t end_of_archive = time(0) + 365*24*60*60;
	strftime(&year[0], sizeof(year), "%Y", gmtime(&end_of_archive));
	
	o << "<server>"
	  << "<version>" << VERSION << "</version>"
	  << "<eoa-year>" << year << "</eoa-year>"
	  << "<doc-url>" << c.docUrl << "</doc-url>"
	  << "<cgi-url>" << c.cgiUrl << "</cgi-url>"
	  << "<command>" << c.command << "</command>"
	  << "<options>" << c.options << "</options>";
	
	if (c.raw_email) o << "<raw-email/>";
	
	o << "<archive>";
	
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
