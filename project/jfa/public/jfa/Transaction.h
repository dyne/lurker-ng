/*  $Id: Transaction.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  Transaction.h - User visible Transaction wrapper
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

#ifndef __JFA_TRANSACTION_H__
#define __JFA_TRANSACTION_H__

#include <jfa/ABI/TransactionI.h>
#include <jfa/Environment.h>

#include <unistd.h>

namespace JFA
{

class Transaction
{
 protected:
 	detail::SmartPtr<detail::TransactionI>	interface;
 
 public:
 	Transaction(const Environment& env)
 	 : interface(env._get_internal()->open_transaction())
 	{ }
 	
 	void commit() throw(trans_exception) { interface->commit(); }
 	void abort () throw(trans_exception) { interface->abort();  }
 	
 	// Begin operating on a database
 	template <class _P>
 	typename _P::Session operator () (const _P& o)
 	{
 		return o.create_session(interface);
 	}
};

} // JFA

// Here are definitions for beginning and ending a transaction.
// Used like:
// JFA_BEGIN_TRANSACTION(t, env)
//   stuff;
// JFA_END_TRANSACTION(t)

// Override this with whatever is appropriate for your system
#define JFA_DELAY_MECHANISM	sleep(random() % 5)

#define JFA_BEGIN_TRANSACTION(t, env) 				\
	if (1)							\
	{							\
		int _jfa_internal_retries = 5;			\
		while (1)					\
		{						\
			try					\
			{					\
				JFA::Transaction t = env;	\

#define JFA_END_TRANSACTION(t) 					\
				t.commit();			\
				break;				\
			}					\
			catch (const JFA::jfa_exception& e)	\
			{					\
				JFA_DELAY_MECHANISM;		\
				if (!--_jfa_internal_retries)	\
					throw;			\
			}					\
		}						\
	}

#endif
