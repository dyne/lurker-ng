/*  $Id$
 *  
 *  Cache.h - Helper which transforms xml -> html and caches files
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

#define _FILE_OFFSET_BITS 64

#include "Cache.h"
#include "commands.h"

#include <cerrno>
#include <cstring>
#include <cassert>

#include <unistd.h>

class streambug : public std::streambuf
{
 public:
 	streambug();
 	
 	void set_target(FILE* t) { target = t; }
 	
 protected:
 	int overflow(int);
 	int sync();
 
 private:
 	FILE*	target;
 	char	buf[128]; //8192];
};

streambug::streambug()
 : target(0)
{
	setp(&buf[0], &buf[sizeof(buf)-1]);
	setg(0,0,0); // output only
}

int streambug::overflow(int c)
{
	assert (target);
	
	// Make sure there is a put area
	if (!pptr()) setp(&buf[0], &buf[sizeof(buf)-1]);
	
	// Determine how many characters have been
	// inserted but not consumed. 
	std::streamsize w = pptr() - pbase();
	
	// If c is not EOF it is a character that must
	// also be consumed. 
	if (c != EOF)
	{
		// We always leave space
		*pptr() = c;
		++w;
	}
	
	// consume characters.
	if ((std::streamsize)fwrite(pbase(), 1, w, target) == w)
	{	// Set up put area. Be sure that there
		// is space at end for one extra character. 
		
		setp(&buf[0], &buf[sizeof(buf)-1]);
		
		if (c == EOF) return 0;
		return c;
	}
	else
	{	// Indicate error.
		setp(0, 0);
		return EOF;
	}
}

int streambug::sync()
{
	if (pptr() && pptr() > pbase())
	{	// Flush waiting output
		return overflow(EOF);
	}
	
	return 0;
}

Cache::Cache(const Config& cfg, const string& command, const string& parameter, const string& ext)
 : bug(new streambug), o(bug)
{
	if (chdir(command.c_str()) != 0)
		error(_("Entering command dir"), command + ":" + strerror(errno),
		      _("Perhaps the directory does not exist to which "
		        "the cache file is being written."));
	
	if (cfg.web_cache)
	{
		cache = fopen(parameter.c_str(), "w+");
		if (!cache)
			error(_("Creating cache file"), parameter + ":" + strerror(errno),
			      _("Perhaps the user which runs lurker.cgi does not have write "
			        "permissions to this directory."));
	}
	else
	{
		cache = stdout;
	}
	
	if (ext == "html")
	{
		char buf[10] = "";
		if (cache != stdout) sprintf(buf, " >&%d", fileno(cache));
		string command = cfg.xslt + buf;
		
		output = popen(command.c_str(), "w");
		if (!output)
			error(_("Opening xslt pipeline"), strerror(errno),
			      _("The specified xslt pipeline in your config "
			        "file, entry xslt, could not be opened. "
			        "Please ensure that the command correctly "
			        "streams xml into html for you."));
		
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/html; charset=UTF-8\r\n\r\n";
	}
	else if (ext == "txt")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/plain\r\n\r\n";
		output = cache;
	}
	else if (ext == "rfc822")
	{
		cout << "Status: 200 OK\r\n";
		cout << "Content-Type: message/rfc822\r\n\r\n";
		output = cache;
	}
	else if (ext == "xml")
	{
		cout << "Status: 200 OK\r\n";
		cout <<	"Content-Type: text/xml; charset=UTF-8\r\n\r\n";
		output = cache;
	}
	else
	{
		error(_("Unknown file type"), ext,
		      _("The requested extension is not supported by lurker. "
		        "Please reformulate your request."));
	}
	
	cout.flush(); // in case of stdout writing next (ick)
	bug->set_target(output);
}

Cache::~Cache()
{
	o.flush();
	
	if (output != cache)
		pclose(output);
	
	if (cache != stdout)
	{
		// reset to the start of the cache file
		fflush(cache);
		fseek(cache, 0, SEEK_SET);
		fflush(cache);
		
		// Begin streaming to cout
		char buf[4096];
		size_t got;
		
		while ((got = fread(buf, 1, sizeof(buf), cache)) != 0)
			cout.write(buf, got);
	}
	
	// All done!
}
