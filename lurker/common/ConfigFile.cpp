/*  $Id$
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

#define _FILE_OFFSET_BITS 64

#include "ConfigFile.h"
#include "XmlEscape.h"
#include "Summary.h"

#include <fstream>
#include <iostream>
#include <algorithm>
#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <dirent.h>
#include <cstring>

using namespace std;

#define ERROR	error << file << ":" << c << ": "

map<string, string>* lstring::c = 0;

lstring::lstring(const string& fallback)
{
	s[""] = fallback;
}

string simplifyPath(string& path)
{
	string error = "";
	
#ifdef HAVE_REALPATH
	char clean[PATH_MAX];
	
	if (realpath(path.c_str(), clean) != 0)
	{
		path = clean;
		return "";
	}
	else
	{
		// on linux, realpath files if the last part DNE
		string dir(path, 0, path.rfind('/'));
		string tag(path, dir.length(), string::npos);
		
		if (realpath(dir.c_str(), clean) != 0)
		{
			path = clean + tag;
			return "";
		}
	}
	
	// realpath failed, report the problem path
	error = clean;
	// fall through with best attempt to clean it up anyways
#endif
	
	// The least we can do is to trim trailing '/'s and simplify '//'s
	while (path.length() && path[path.length()-1] == '/')
		path.resize(path.length()-1);
	
	string::size_type x = 0;
	while ((x = path.find("//", x)) != string::npos)
		path.erase(x, 1);
	
	return error;
}

void lstring::prep_c()
{
	c = new map<string, string>();
	
	// source: http://www.w3.org/WAI/ER/IG/ert/iso639.htm
	static const char* code[][2] =
	{	{"abk", "ab"},
		{"aar", "aa"},
		{"afr", "af"},
		{"alb", "sq"},
		{"amh", "am"},
		{"ara", "ar"},
		{"arm", "hy"},
		{"asm", "as"},
		{"aym", "ay"},
		{"aze", "az"},
		{"bak", "ba"},
		{"baq", "eu"},
		{"ben", "bn"},
		{"bih", "bh"},
		{"bis", "bi"},
		{"bre", "be"},
		{"bul", "bg"},
		{"bur", "my"},
		{"bel", "be"},
		{"cat", "ca"},
		{"chi", "zh"},
		{"cos", "co"},
		{"ces", "cs"},
		{"dan", "da"},
		{"dut", "nl"},
		{"dzo", "dz"},
		{"eng", "en"},
		{"epo", "eo"},
		{"est", "et"},
		{"fao", "fo"},
		{"fij", "fj"},
		{"fin", "fi"},
		{"fra", "fr"},
		{"fry", "fy"},
		{"glg", "gl"},
		{"geo", "ka"},
		{"deu", "de"},
		{"ell", "el"},
		{"kal", "kl"},
		{"grn", "gn"},
		{"guj", "gu"},
		{"hau", "ha"},
		{"heb", "he"},
		{"hin", "hi"},
		{"hun", "hu"},
		{"ice", "is"},
		{"ind", "id"},
		{"ina", "ia"},
		{"iku", "iu"},
		{"ipk", "ik"},
		{"gai", "ga"},
		{"ita", "it"},
		{"jpn", "ja"},
		{"jav", "jv"},
		{"kan", "kn"},
		{"kas", "ks"},
		{"kaz", "kk"},
		{"khm", "km"},
		{"kin", "rw"},
		{"kir", "ky"},
		{"kor", "ko"},
		{"kur", "ku"},
		{"oci", "oc"},
		{"lao", "lo"},
		{"lat", "la"},
		{"lav", "lv"},
		{"lin", "ln"},
		{"lit", "lt"},
		{"mac", "mk"},
		{"mlg", "mg"},
		{"may", "ms"},
		{"mlt", "ml"},
		{"mao", "mi"},
		{"mar", "mr"},
		{"mol", "mo"},
		{"mon", "mn"},
		{"nau", "na"},
		{"nep", "ne"},
		{"nor", "no"},
		{"ori", "or"},
		{"orm", "om"},
		{"pan", "pa"},
		{"fas", "fa"},
		{"pol", "pl"},
		{"por", "pt"},
		{"pus", "ps"},
		{"que", "qu"},
		{"roh", "rm"},
		{"ron", "ro"},
		{"run", "rn"},
		{"rus", "ru"},
		{"smo", "sm"},
		{"sag", "sg"},
		{"san", "sa"},
		{"scr", "sh"},
		{"sna", "sn"},
		{"snd", "sd"},
		{"sin", "si"},
		{"ssw", "ss"},
		{"slk", "sk"},
		{"slv", "sl"},
		{"som", "so"},
		{"sot", "st"},
		{"esl", "es"},
		{"sun", "su"},
		{"swa", "sw"},
		{"sve", "sv"},
		{"tgl", "tl"},
		{"tgk", "tg"},
		{"tam", "ta"},
		{"tat", "tt"},
		{"tel", "te"},
		{"tha", "th"},
		{"bod", "bo"},
		{"tir", "ti"},
		{"tog", "to"},
		{"tso", "ts"},
		{"tsn", "tn"},
		{"tur", "tr"},
		{"tuk", "tk"},
		{"twi", "tw"},
		{"uig", "ug"},
		{"ukr", "uk"},
		{"urd", "ur"},
		{"uzb", "uz"},
		{"vie", "vi"},
		{"vol", "vo"},
		{"cym", "cy"},
		{"wol", "wo"},
		{"xho", "xh"},
		{"yid", "yi"},
		{"yor", "yo"},
		{"zha", "za"},
		{"zul", "zu"},
		{"sqi", "sq"},
		{"hye", "hy"},
		{"eus", "eu"},
		{"mya", "my"},
		{"zho", "zh"},
		{"cze", "cs"},
		{"nla", "nl"},
		{"fre", "fr"},
		{"kat", "ka"},
		{"ger", "de"},
		{"gre", "el"},
		{"isl", "is"},
		{"iri", "ga"},
		{"jaw", "jv"},
		{"mak", "mk"},
		{"msa", "ms"},
		{"mri", "mi"},
		{"per", "fa"},
		{"rum", "ro"},
		{"slo", "sk"},
		{"spa", "es"},
		{"swe", "sv"},
		{"tib", "bo"},
		{"wel", "cy"},
		// I think these are the same?
		{"jw",  "jv"},
		// ditto
		{"hr", "scc"},
		{"sr", "scc"},
		{"srp", "scc"},
		{"", ""}
	};
	
	for (unsigned int i = 0; code[i][0][0]; ++i)
		(*c)[code[i][0]] = code[i][1];
}

bool lstring::lang_normalize(string& lang)
{
	if (!c) prep_c();
	
	if (lang.length() != 2 && lang.length() != 3)
		return false;
	
	// lower-case it:
	string iso(lang);
	for (string::size_type i = 0; i < iso.length(); ++i)
	{
		if (iso[i] >= 'A' && iso[i] <= 'Z')
			iso[i] = iso[i] - 'A' + 'a';
		if (iso[i] < 'a' || iso[i] > 'z')
			return false; // not an ISO 639 code!
	}
	
	// Resolve different language spellings to one spelling
	if (c->find(iso) != c->end())
		lang = (*c)[iso];
	else	lang = iso;
	
	return true;
}
	
bool lstring::locale_normalize(string& locale)
{
	string::size_type i, e;
	
	if ((e = locale.find('-')) == string::npos &&
	    (e = locale.find('_')) == string::npos)
		e = locale.length();
	
	string lang(locale, 0, e);
	if (!lang_normalize(lang)) return false;
	
	string region;
	if (e != locale.length())
	{
		region.assign(locale, e+1, string::npos);
		if (region.length() != 2) return false; // not an ISO 3166 code
		
		for (i = 0; i < 2; ++i)
		{
			if (region[i] >= 'a' && region[i] <= 'z')
				region[i] = region[i] - 'a' + 'A';
			if (region[i] < 'A' || region[i] > 'Z')
				return false; // not an ISO 3166 code!
		}
		
		locale = lang + "-" + region;
	}
	else
	{
		locale = lang;
	}
	
	return true;
}

bool lstring::translate(const string& language_, const string& translation)
{
	string language(language_);
	if (language != "" && !locale_normalize(language)) return false;
	
	string::size_type i;
	
	// this overrides whatever we have got so far
	s[language] = translation;
	
	// maybe a localized string for which we lack a language setting?
	if ((i = language.find('-')) != string::npos)
	{
		string iso(language, 0, i);
		if (s.find(iso) == s.end()) translate(iso, translation);
	}
	
	// maybe we lack a fallback language completely
	if (s.find("") == s.end()) translate("", translation);
	
	return true;
}

string lstring::localize(const string& language_) const
{
	string language(language_);
	if (language != "" && !locale_normalize(language)) return "(bug) bad locale";
	
	map<string, string>::const_iterator o;
	string::size_type i;
	
	// correct locale? use it
	if ((o = s.find(language)) != s.end()) return o->second;
	
	// correct language? use it
	if ((i = language.find('-')) != string::npos)
	{
		string iso(language, 0, i);
		if ((o = s.find(iso)) != s.end()) return o->second;
	}
	
	// fallback? use it
	if ((o = s.find("")) != s.end()) return o->second;
	
	return ""; // not set!
}

bool lstring::is_set() const
{
	return !s.empty();
}

Config::Config()
 : list(0), frontend(0), group(""), error(), lists(), groups(),
   file(""),
   dbdir(""), 
   db_umask(-1),
   xslt("cat -"),
   delete_message(""),
   pgpv_mime("off"),
   pgpv_inline("off"),
   admin_address(""),
   
   archive("Unconfigured Archivew"),
   admin_name("Unset admin name"),
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
				ERROR << "No key for value '" << line << "'!" << endl;
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
			if (key != "" && process_command(file, c, key, val, dir) != 0) ok = false;
			
			string::size_type leadin = skip_front(line);
			key.assign(line, leadin, eq-leadin);
			val.assign(line, skip_front(line, eq+1), string::npos);
			prune_back(key);
		}
	}
	
	if (toplevel && key == "")
	{
		error << file << ":eof: No values set!" << endl;
		ok = false;
	}
	
	if (key != "" && process_command(file, c, key, val, dir) != 0) ok = false;
	
	if (toplevel)
	{
		// do some consistency checks
		
		// language field is required
		Lists::const_iterator i, e;
		for (i = lists.begin(), e = lists.end(); i != e; ++i)
		{
			if (i->second.languages.empty()) 
			{
				error << file << ":eof: List '" << i->first << "' has no language!" << endl;
				return -1;
			}
		}
		
		// cache directories may not be prefixes
		Frontends::const_iterator fi, fn, fe;
		fe = frontends.end();
		fi = frontends.begin();
		
		fn = fi;
		if (fi != fe) while (1)
		{
			++fn;
			if (fn == fe) break;
			
			if (fi->first + "/" == fn->first.substr(0, fi->first.length()+1))
			{
				error << file << ":eof: Frontend '" << fi->first << "' is a prefix of '" << fn->first << "', which is forbidden!" << endl;
				return -1;
			}
			
			fi = fn;
		}
		
		this->file = file;
	}
	
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

int Config::process_command(const string& file, int c, const string& keys, const string& val, const string& dir)
{
//	cout << key << "-" << val << endl;
	
	string lc; // locale code
	string key(keys);
	
	string::size_type o, d;
	if ((o = key.find('[')) != string::npos &&
	    (d = key.find(']')) != string::npos &&
	    d > o)
	{
		// localization option
		lc.assign(key, o+1, (d-o) - 1);
		key.erase(o, (d-o) + 1);
		
		if (!lstring::locale_normalize(lc))
		{
			ERROR << "Localization code '" << lc << "' is not valid." << endl;
			return -1;
		}
	}
	
	string::size_type len = string::npos;
	
	if (key == "group")
	{
		len = 128;
		if (!isSimple(val) || val.length() == 0)
		{
			ERROR << "Group id '" << val << "' is not a simple lowercase string!" << endl;
			return -1;
		}
		
		if (lc != "")
		{
			ERROR << "group id cannot be localized" << endl;
			return -1;
		}
		
		if (groups.find(val) != groups.end())
		{
			ERROR << "Group id '" << val << "' already exists!" << endl;
			return -1;
		}
		
		group = val;
		groups[group]; // make sure it exists
	}
	else if (key == "heading")
	{
		len = 180;
		groups[group].heading.translate(lc, val);
	}
	else if (key == "list")
	{
		if (group == "")
		{
			ERROR << "List id '" << val << "' is not a member of any group!" << endl;
			return -1;
		}
		
		len = 128;
		if (!isSimple(val) || val.length() == 0)
		{
			ERROR << "List id '" << val << "' is not a simple lowercase string!" << endl;
			return -1;
		}
		if (lc != "")
		{
			ERROR << "list id cannot be localized" << endl;
			return -1;
		}
		
		if (lists.find(val) == lists.end())
		{
			groups[group].members.insert(val);
			list = &lists[val];
			list->mbox = val;
			list->group = group;
			list->offline = false;
		}
		else
		{
			ERROR << "List id '" << val << "' already exists!" << endl;
			return -1;
		}
	}
	else if (key == "title")
	{
		len = 180;
		
		if (!list)
		{
			ERROR << "No list has been defined for title '" << val << "'!" << endl;
			return -1;
		}
		
		list->title.translate(lc, val);
	}
	else if (key == "address")
	{
		if (!list)
		{
			ERROR << "No list has been defined for address '" << val << "'!" << endl;
			return -1;
		}
		if (lc != "")
		{
			ERROR << "list address cannot be localized" << endl;
			return -1;
		}
		
		list->address = val;
	}
	else if (key == "link")
	{
		if (!list)
		{
			ERROR << "No list has been defined for address '" << val << "'!" << endl;
			return -1;
		}
		
		list->link.translate(lc, val);
	}
	else if (key == "language")
	{
		if (!list)
		{
			ERROR << "No list has been defined for language '" << val << "'!" << endl;
			return -1;
		}
		if (lc != "")
		{
			ERROR << "list language cannot be localized" << endl;
			return -1;
		}
		
		string lval(val);
		if (!lstring::lang_normalize(lval))
		{
			ERROR << "Language '" << val << "' is not an ISO 639 language code!" << endl;
			error << "Regional variants are not relevant for searches." << endl;
			return -1;
		}
		
		list->languages.insert(lval);
	}
	else if (key == "offline")
	{
		if (!list)
		{
			ERROR << "No list has been defined for offline setting '" << val << "'!" << endl;
			return -1;
		}
		if (lc != "")
		{
			ERROR << "list offline cannot be localized" << endl;
			return -1;
		}
		
		if (val == "off" || val == "false")
			list->offline = false;
		else if (val == "on" || val == "true")
			list->offline = true;
		else
		{
			ERROR << "offline must be set to on/off or true/false!" << endl;
			return -1;
		}
	}
	else if (key == "description")
	{
		if (!list)
		{
			ERROR << "No list has been defined for address '" << val << "'!" << endl;
			return -1;
		}
		
		list->description.translate(lc, val);
	}
	else if (key == "frontend")
	{
		len = 10240; // Long paths are ... ok
		
		if (lc != "")
		{
			ERROR << "frontend path '" <<  val << "' cannot be localized" << endl;
			return -1;
		}
		
		if (val.length() == 0 || val[0] != '/')
		{
			ERROR << "frontend path is not absolute '" << val << "' (must start with a '/')!" << endl;
			return -1;
		}
		
		// Cleanup the path, but ignore any problems with the path
		string sval(val);
		simplifyPath(sval);
		
		if (frontends.find(sval) == frontends.end())
		{
			frontend = &frontends[sval];
			
			// Inherit global values
			frontend->admin_name = admin_name;
			frontend->admin_address = admin_address;
			frontend->archive = archive;
			frontend->hide_email = hide_email;
			frontend->raw_email = raw_email;
			frontend->web_cache = web_cache;
		}
		else
		{
			ERROR << "Frontend '" << val << "' already exists!" << endl;
			return -1;
		}
	}
	else if (key == "allow_list")
	{
		if (frontend == 0)
		{
			ERROR << "No frontend defined for allow_list = '" << val << "'" << endl;
			return -1;
		}
		
		if (lists.find(val) == lists.end())
		{
			ERROR << "List '" << val << "' does not exist for allow_list" << endl;
			return -1;
		}
		
		Frontend::Entry e;
		e.perm = Frontend::ALLOW;
		e.what = Frontend::LIST;
		e.key = val;
		frontend->entries.push_back(e);
	}
	else if (key == "deny_list")
	{
		if (frontend == 0)
		{
			ERROR << "No frontend defined for deny_list = '" << val << "'" << endl;
			return -1;
		}
		
		if (lists.find(val) == lists.end())
		{
			ERROR << "List '" << val << "' does not exist for deny_list" << endl;
			return -1;
		}
		
		Frontend::Entry e;
		e.perm = Frontend::DENY;
		e.what = Frontend::LIST;
		e.key = val;
		frontend->entries.push_back(e);
	}
	else if (key == "allow_group")
	{
		if (frontend == 0)
		{
			ERROR << "No frontend defined for allow_group = '" << val << "'" << endl;
			return -1;
		}
		
		if (groups.find(val) == groups.end())
		{
			ERROR << "Group '" << val << "' does not exist for allow_group" << endl;
			return -1;
		}
		
		Frontend::Entry e;
		e.perm = Frontend::ALLOW;
		e.what = Frontend::GROUP;
		e.key = val;
		frontend->entries.push_back(e);
	}
	else if (key == "deny_group")
	{
		if (frontend == 0)
		{
			ERROR << "No frontend defined for deny_group = '" << val << "'" << endl;
			return -1;
		}
		
		if (groups.find(val) == groups.end())
		{
			ERROR << "Group '" << val << "' does not exist for deny_group" << endl;
			return -1;
		}
		
		Frontend::Entry e;
		e.perm = Frontend::DENY;
		e.what = Frontend::GROUP;
		e.key = val;
		frontend->entries.push_back(e);
	}
	else if (key == "dbdir")
	{
		if (lc != "")
		{
			ERROR << "dbdir cannot be localized" << endl;
			return -1;
		}
		
		if (val[0] == '/')
			dbdir = val;
		else	dbdir = dir + val;
	}
	else if (key == "db_umask")
	{
		if (lc != "")
		{
			ERROR << "db_umask cannot be localized" << endl;
			return -1;
		}
		
		char* e;
		db_umask = strtol(val.c_str(), &e, 8);
		if (val.length() == 0 || *e != 0)
		{
			ERROR << "db_mask must be given an octal number, not '" << val << "'" << endl;
			return -1;
		}
	}
	else if (key == "admin_name")
	{
		if (frontend == 0)
			admin_name.translate(lc, val);
		else	frontend->admin_name.translate(lc, val);
	}
	else if (key == "admin_address")
	{
		if (lc != "")
		{
			ERROR << "admin_address cannot be localized" << endl;
			return -1;
		}
		if (frontend == 0)
			admin_address = val;
		else	frontend->admin_address = val;
	}
	else if (key == "archive")
	{
		if (frontend == 0)
			archive.translate(lc, val);
		else	frontend->archive.translate(lc, val);
	}
	else if (key == "xslt")
	{
		if (lc != "")
		{
			ERROR << "xslt command cannot be localized" << endl;
			return -1;
		}
		xslt = val;
	}
	else if (key == "delete_message")
	{
		if (lc != "")
		{
			ERROR << "delete_message command cannot be localized" << endl;
			return -1;
		}
		delete_message = val;
	}
	else if (key == "pgp_verify_mime")
	{
		if (lc != "")
		{
			ERROR << "pgp_verify_mime command cannot be localized" << endl;
			return -1;
		}
		pgpv_mime = val;
	}
	else if (key == "pgp_verify_inline")
	{
		if (lc != "")
		{
			ERROR << "pgp_verify_inline command cannot be localized" << endl;
			return -1;
		}
		pgpv_inline = val;
	}
	else if (key == "web_cache")
	{
		if (lc != "")
		{
			ERROR << "web_cache cannot be localized" << endl;
			return -1;
		}
		
		bool newval;
		if (val == "off" || val == "false")
			newval = false;
		else if (val == "on" || val == "true")
			newval = true;
		else
		{
			ERROR << "web_cache must be set to on/off or true/false!" << endl;
			return -1;
		}
		
		if (frontend == 0)
			web_cache = newval;
		else	frontend->web_cache = newval;
	}
	else if (key == "hide_email")
	{
		if (lc != "")
		{
			ERROR << "hide_email cannot be localized" << endl;
			return -1;
		}
		
		bool newval;
		if (val == "off" || val == "false")
			newval = false;
		else if (val == "on" || val == "true")
			newval = true;
		else
		{
			ERROR << "hide_email must be set to on/off or true/false!" << endl;
			return -1;
		}
		
		if (frontend == 0)
			hide_email = newval;
		else	frontend->hide_email = newval;
	}
	else if (key == "raw_email")
	{
		if (lc != "")
		{
			ERROR << "raw_email cannot be localized" << endl;
			return -1;
		}
		
		bool newval;
		if (val == "off" || val == "false")
			newval = false;
		else if (val == "on" || val == "true")
			newval = true;
		else
		{
			ERROR << "raw_email must be set to on/off or true/false!" << endl;
			return -1;
		}
		
		if (frontend == 0)
			raw_email = newval;
		else	frontend->raw_email = newval;
	}
	else if (key == "include")
	{
		if (lc != "")
		{
			ERROR << "include cannot be localized" << endl;
			return -1;
		}
		string file;
		
		if (val[0] == '/')
			file = val;
		else	file = dir + val;
		
		DIR* d = opendir(file.c_str());
		if (d)
		{
			struct dirent* e;
			struct stat s;
			vector<string> paths;
			while ((e = readdir(d)) != 0)
			{
				int len = strlen(e->d_name);
				if (e->d_name[0] == '.') continue;
				if (len <= 5) continue;
				if (strcmp(".conf", &e->d_name[len-5]))
					continue;
				
				string path = file + '/' + e->d_name;
				if (stat(path.c_str(), &s) != 0) continue;
				if (!S_ISREG(s.st_mode)) continue;
				
				paths.push_back(path);
			}
			closedir(d);
			
			// Include them in sorted order
			sort(paths.begin(), paths.end());
			for (vector<string>::const_iterator i = paths.begin();
			    i != paths.end(); ++i)
			{
				if (load(*i, false) != 0)
					return -1;
			}
		}
		else
		{
			if (load(file, false) != 0)
				return -1;
		}
	}
	else
	{
		ERROR << "Unknown configuration directive '" << key << "'!" << endl;
		return -1;
	}
	
	if (val.length() > len)
	{
		ERROR << "Value '" << val << "' is too long for directive '" << key << "'!" << endl;
		return -1;
	}
	
	return 0;
}

ostream& operator << (ostream& o, const List::SerializeMagic& lm)
{
	const List& m = lm.m;
	const string& l = lm.l;
	
	o << "<list>"
	  << "<id>" << m.mbox << "</id>"
	  << "<group>" << m.group << "</group>";
	
	set<string>::const_iterator i, e;
	for (i = m.languages.begin(), e = m.languages.end(); i != e; ++i)
		o << "<language>" << *i << "</language>";
	
	if (m.offline)
		o << "<offline/>";
	
	if (m.link.is_set())
		o << "<link>" << xmlEscape << m.link(l) << "</link>";
		
	if (m.description.is_set())
		o << "<description>" << xmlEscape << m.description(l) << "</description>";
	
	o  << "<email";
	if (m.address.length() > 0)
		o << " address=\"" << xmlEscape << m.address << "\"";
	if (m.title.is_set())
		o << " name=\"" << xmlEscape << whitespace_sanitize(m.title(l)) << "\"";
	else
		o << " name=\"" << m.mbox << "\"";
	
	o << "/></list>";
	
	return o;
}

ostream& operator << (ostream& o, const Config::SerializeMagic& cm)
{
	const Config& c = cm.c;
	const string& l = cm.l;
	
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
	  << "<doc-url>" << xmlEscape << c.docUrl << "</doc-url>"
	  << "<cgi-url>" << xmlEscape << c.cgiUrl << "</cgi-url>"
	  << "<command>" << c.command << "</command>"
	  << "<options>" << xmlEscape << c.options << "</options>";
	
	if (c.raw_email) o << "<raw-email/>";
	
	o << "<archive>";
	
	if (c.archive.is_set())
		o << xmlEscape << c.archive(l);
	else	o << "Some Mailing List Archive";
	
	o << "</archive><email";
	if (c.admin_address.length() > 0)
		o << " address=\"" << xmlEscape << c.admin_address << "\"";
	if (c.admin_name.is_set())
		o << " name=\"" << xmlEscape << whitespace_sanitize(c.admin_name(l)) << "\"";
	o << "/></server>";
	
	return o;
}

void Config::set_permissions(const Frontend& f)
{
	vector<Frontend::Entry>::const_iterator ei,
		es = f.entries.begin(),
		ee = f.entries.end();
	
	Lists::iterator li,
		ls = lists.begin(),
		le = lists.end();
	
	// Empty list or first entry deny means default to allow
	bool def = (es == ee) || (es->perm == Frontend::DENY);
	for (li = ls; li != le; ++li) li->second.allowed = def;
	
	// Walk each entry toggling permissions as we go
	for (ei = es; ei != ee; ++ei)
	{
		bool allowed = (ei->perm == Frontend::ALLOW);
		
		if (ei->what == Frontend::LIST)
		{
			lists[ei->key].allowed = allowed;
		}
		else
		{ // group
			Members::const_iterator mi,
				ms = groups[ei->key].members.begin(),
				me = groups[ei->key].members.end();
			for (mi = ms; mi != me; ++mi)
			{
				lists[*mi].allowed = allowed;
			}
		}
	}
	
	// Take the specific settings for the globals
	archive = f.archive;
	admin_name = f.admin_name;
	admin_address = f.admin_address;
	hide_email = f.hide_email;
	raw_email = f.raw_email;
	web_cache = f.web_cache;
}
