/*  $Id: DbMan.h,v 1.2 2003-04-21 18:25:32 terpstra Exp $
 *  
 *  DbMan.h - Manage the commit'd segments and parameters
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

#ifndef DBMAN_H
#define DBMAN_H

#include <set>
#include <string>

namespace ESort
{

using std::string;

class Parameters;
class View;
class DbMan
{
 protected:
 	string	dbname;
 	FILE*	dbfile;
 	int	cmode;
 	int	dblock;
 	std::set<string> ids;
 	
 	static int qualify(string& db);
 	int scanFile(Parameters& p);
 	
 protected:
 	/** Lock the snapshot against changes.
 	 */
 	int  lock_snapshot_ro();
 	void unlock_snapshot_ro();
 	int  lock_snapshot_rw();
 	void unlock_snapshot_rw();
 	
 public:
 	DbMan();
 	~DbMan();
 	
 	/** Open the database
 	 */
 	int open(const string& db, Parameters& p); // ro access
 	int open(const string& db, Parameters& p, int mode); // rw
 	
 	/** Lock the database for this single writer.
 	 */
 	int  lock_database_rw();
 	void unlock_database_rw();
 	
 	/** Obtain a View snapshot
 	 *  Must be lock_snapshot_ro'd
 	 */
 	int snapshot(View& view);
 	
 	/** Store the current View + Parameters as the new db state
 	 *  Must be lock_snapshot_rw'd & lock_database_rw'd
 	 */
 	int commit(const View& view);
 	
 	/** Open a new sub-database.
 	 *  Must be lock_database_rw'd
 	 */
 	int openNew(string& id);
 	
 	/** Destroy a sub-database.
 	 *  Must be lock_data_base_rw'd & after commit
 	 */
 	int killSub(const string& id);
};

}

#endif
