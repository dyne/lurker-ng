/*  $Id: Parameters.cpp,v 1.2 2003-04-21 18:25:32 terpstra Exp $
 *  
 *  Parameters.cpp - Control how the database is stored
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

#include "esort.h"
#include <cmath>

namespace ESort
{

Parameters::Parameters(
	unsigned int  version, 
	unsigned long blockSize, 
	unsigned long keySize, 
	bool          unique)
 : version_(version), blockSize_(blockSize), keySize_(keySize), unique_(unique)
{
	assert (keySize_ > 0);
	assert (keySize_ * 4 <= blockSize_);
	
	keyWidth_ = static_cast<int>(
		ceil(
			log(static_cast<double>(keySize_)) /
			log(256)
		));
}

bool Parameters::isWider(const Parameters& p)
{
	return version_   >= p.version_ &&
	       blockSize_ >= p.blockSize_ &&
	       keySize_  >= p.keySize_;
}

}
