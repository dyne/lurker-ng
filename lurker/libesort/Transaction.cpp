/*  $Id: Transaction.cpp,v 1.5 2003-05-07 15:43:13 terpstra Exp $
 *  
 *  Transaction.cpp - Write buffer for a commit
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
  
#include "io.h"
#include "Transaction.h"
#include "esort.h"

#include <iostream>

/* #define DEBUG */

namespace ESort
{

Transaction::Transaction(int fd_, const Parameters* p_)
 : fd(fd_), p(p_), buf(new unsigned char[p->blockSize()]), 
   eof(buf + p->blockSize() - 2*(p->keyWidth() + 1)), off(buf)
{
}

Transaction::~Transaction()
{
	delete [] buf;
}

int Transaction::dump()
{
//	std::cout << "BLOCK WRITE" << std::endl;
	
	ssize_t out = ::write(fd, buf, p->blockSize());
	if (out != (ssize_t)p->blockSize())
	{
#ifdef DEBUG
		perror(__FILE__ ":" #__LINE__ ":write");
#endif
		return -1;
	}
	
	return 0;
}

int Transaction::write(long len, int dup, const char* key)
{
	assert (dup < 256);
	
//	std::cout << "Write: " << len << " - " << dup << " - " << key << std::endl;
	
	long fragment = len - dup;
	
	if (off + fragment > eof)
	{	// would overflow this block
		for (unsigned int i = 0; i < p->keyWidth(); ++i)
			off[i] = 0;
		off[p->keyWidth()] = 1; // (0, 1) is eof marker
		
		if (dump() != 0) return -1;
		off = buf;
		
		// first key has 0 compression
		dup = 0;
		fragment = len;
	}
	
	for (int i = p->keyWidth()-1; i >= 0; --i)
	{
		((unsigned char*)off)[i] = len;
		len >>= 8;
	}
	
	off += p->keyWidth();
	*off = dup;
	++off;
	
	memcpy(off, key+dup, fragment);
	off += fragment;
	
//	std::cout << "Position: " << long(off - buf) << " from 0." << std::endl;
	
	return 0;
}

int Transaction::finish()
{
	if (off != buf)
	{
		for (unsigned int i = 0; i < p->keyWidth(); ++i)
			off[i] = 0;
		off[p->keyWidth()] = 1; // (0, 1) is eof marker
		
		if (dump() != 0) return -1;
	}
	
	if (p->synced())
		return fsync(fd);
	else	return 0;
}

}
