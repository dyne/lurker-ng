/*  $Id: Transaction.h,v 1.3 2003-04-25 14:09:16 terpstra Exp $
 *  
 *  Transaction.h - Write buffer for a commit
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

#ifndef TRANSACTION_H
#define TRANSACTION_H

namespace ESort
{

class Parameters;
class Transaction
{
 protected:
 	int		fd;
 	const Parameters* p;
 	unsigned char*	buf;
 	unsigned char*	eof;
 	unsigned char*	off;
 	
 	/** Dump the buffer.
 	 */
 	int dump();
 
 public:
 	/** Write to this file descriptor.
 	 *  We do NOT assume responsibility for closing the file!!
 	 */
 	Transaction(int fd_, const Parameters* p_);
 	~Transaction();
 	
 	/** Output this to the file, 0 = success
 	 */
 	int write(long len, int dup, const char* key);
 	
 	/** Finalize the file, 0 = success
 	 */
 	int finish();
};

}

#endif
