/*  $Id: StaticAssert.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  StaticAssert.h - Quick 'n dirty way to check things at compile-time
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

#ifndef __JFA_ABI_STATIC_ASSERT_H__
#define __JFA_ABI_STATIC_ASSERT_H__

namespace JFA
{
namespace detail
{

// Stolen shamelessly from boost (under fair use)
template <bool b>
struct STATIC_ASSERTION_FAILED;
struct STATIC_ASSERTION_FAILED<true> { };

template <int X>
struct jfa_static_assertion_test { };

#define JFA_JOIN(X,Y) JFA_JOIN2(X,Y)
#define JFA_JOIN2(X,Y) X##Y
#define JFA_STATIC_ASSERT(expr) \
	typedef JFA::detail::jfa_static_assertion_test< \
		sizeof(JFA::detail::STATIC_ASSERTION_FAILED<bool(expr)>)> \
		JFA_JOIN(jfa_static_assertion_, __LINE__)

} // detail
} // JFA

#endif
