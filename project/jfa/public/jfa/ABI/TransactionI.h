/*  $Id: TransactionI.h,v 1.1.1.1 2003-01-04 21:13:59 terpstra Exp $
 *  
 *  TransactionI.h - Interface into the library for Transaction support
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

#ifndef __JFA_ABI_TRANSACTION_I_H__
#define __JFA_ABI_TRANSACTION_I_H__

#include <jfa/ABI/Exceptions.h>
#include <jfa/ABI/SmartPtr.h>

namespace JFA
{

/* What is the scoop on these exceptions?
 *
 * A failed_exception means that the caller MUST abort the transaction
 *   immediately. The database is in a corrupt state with respect to the
 *   given transaction. The caller may retry the operation after abort.
 * 
 * A read_locked or write_locked_exception means the caller SHOULD abort
 *   transaction and then retry. If the caller does not do this, it is
 *   possible that a deadlock can result.
 *
 * An io_retry_exception means that the caller can retry the operation which
 *   failed without restarting the transaction.
 */

class trans_exception : public jfa_exception
{
 public:
 	trans_exception(const char* f, int l, const char* w)
 	 : jfa_exception(f, l, w) { }
};

class failed_exception : public trans_exception
{
 public:
 	failed_exception(const char* f, int l, const char* w)
 	 : trans_exception(f, l, w) { }
 	
 	const char* detail() const throw();
};

class read_locked_exception : public trans_exception
{
 public:
 	read_locked_exception(const char* f, int l, const char* w)
 	 : trans_exception(f, l, w) { }
 	 
 	const char* detail() const throw();
};

class write_locked_exception : public trans_exception
{
 public:
 	write_locked_exception(const char* f, int l, const char* w)
 	 : trans_exception(f, l, w) { }
 	 
 	const char* detail() const throw();
};

class io_retry_exception : public trans_exception
{
 public:
 	io_retry_exception(const char* f, int l, const char* w)
 	 : trans_exception(f, l, w) { }
 	 
 	const char* detail() const throw();
};

namespace detail
{

// Any transaction will keep alive the TransactionManager (err. I mean
// Environment) that was created from.
class TransactionI : public MemoryManaged
{
 public:
 	virtual void commit() throw(trans_exception) = 0;
 	virtual void abort () throw() = 0; // this must never fail
};

} // detail
} // JFA

#endif
