/*  $Id: PTable.cpp,v 1.2 2003-05-13 21:38:29 terpstra Exp $
 *  
 *  PTable.cpp - Prune table records state for pruning
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

#include "PTable.h"

#include <unistd.h>

#include <iostream>

#include <cerrno>
#include <cstring>

PTable::PTable(ESort::Reader* reader_, time_t config_, time_t stamp_, bool verbose_)
 : reader(reader_), config(config_), stamp(stamp_), verbose(verbose_)
{
}

string PTable::pull()
{
	// !!! pull the data
	return "";
}

string PTable::prep()
{
#if 0
	string ok;
	if ((ok = prep_message()) != "") return ok;
	if ((ok = prep_thread ()) != "") return ok;
	if ((ok = prep_mindex ()) != "") return ok;
	if ((ok = prep_splash ()) != "") return ok;
	if ((ok = prep_search ()) != "") return ok;
	if ((ok = prep_attach ()) != "") return ok;
	if ((ok = prep_mbox   ()) != "") return ok;
#endif
	return "";
}

string PTable::load()
{
	// !!! actually load summaries
	return "";
}

string PTable::calc()
{
#if 0
	string ok;
	if ((ok = calc_message()) != "") return ok;
	if ((ok = calc_thread ()) != "") return ok;
	if ((ok = calc_mindex ()) != "") return ok;
	if ((ok = calc_splash ()) != "") return ok;
	if ((ok = calc_search ()) != "") return ok;
	if ((ok = calc_attach ()) != "") return ok;
	if ((ok = calc_mbox   ()) != "") return ok;
#endif
	return "";
}

string PTable::kill()
{
	set<KillState>::const_iterator i;
	for (i = state.begin(); i != state.end(); ++i)
	{
		if (i->kill)
		{
			if (verbose) std::cout << "Deleting: " << i->file << endl;
			if (unlink(i->file.c_str()) < 0)
			{
				cerr << "Cannot unlink: " << i->file << ": " << strerror(errno) << endl;
				// continue
			}
		}
	}
	
	return "";
}
