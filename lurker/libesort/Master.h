/*  $Id: Master.h,v 1.3 2003-04-24 23:52:36 terpstra Exp $
 *  
 *  Master.h - Coordinate commit+read interface
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

#ifndef MASTER_H
#define MASTER_H

#include "esort.h"
#include "Memory.h"
#include "View.h"
#include "DbMan.h"

namespace ESort
{

class Transaction;

class Master : public Writer
{
 protected:
 	View	view;
 	Memory	memory;
 	DbMan	man;
 
 public:
 	Master();
 	~Master();
 	
 	int init(const string& db, const Parameters& p, int mode);
 	
 	int commit();
 	int insert(const string& k);
 	auto_ptr<Walker> seek(const string& k, bool forward);
};

}

#endif
