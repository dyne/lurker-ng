/*  $Id: intSerialize.h,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  intSerialize.h - A template-driven integer serializer
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

#ifndef __JFA_INT_SERIALIZE_H__
#define __JFA_INT_SERIALIZE_H__

#include <stdexcept>

/* A helper which serializes any integer to any byte-buffer iterator.
 * Usage: intSerialize<3>(it, num);
 * 
 * This will use 3 bytes on the stream to encode num.
 * On negative integers or those too large to pack out_of_range is thrown.
 * 
 * You must ensure prior to call that there are the correct number of bytes
 * avaliable. The iterator will not be repositioned by this call.
 * 
 * To check the range, please call 'bool intRangeOk<3>(num);'
 * If you want no checking, use intSerializeFast<3>(it, num);
 */

namespace JFA
{
namespace detail
{

//------------------------------------------------------------ Serialization

template <typename Iterator, typename Int, int N>
struct IntSerializor
{
	static void serialize(Iterator buf, Int i)
	{
		*buf = (i & 0xFF);
		i >>= 8;
		IntSerializor<Iterator, Int, N-1>::serialize(buf-1, i);
	}
};

template <typename Iterator, typename Int>
struct IntSerializor<Iterator, Int, 0>
{
	static void serialize(Iterator buf, Int i)
	{
		// noop
	}
};

template <int N, typename Iterator, typename Int>
inline void intSerializeFast(Iterator buf, Int i)
{
	return IntSerializor<Iterator, Int, N>::
		serialize(buf+N-1, i);
}


//------------------------------------------------------------ Range testing

template <int N>
struct IntMaxSize
{
	enum { max = IntMaxSize<N-1>::max*256 };
};

struct IntMaxSize<0>
{
	enum { max = 1 };
};

template <int N, typename Int>
bool intRangeOk(Int i)
{
	if (i < 0) return false;
	if (i >= IntMaxSize<N>::max) return false;
	return true;
}

//------------------------------------------------------------ Serialization

template <int N, typename Iterator, typename Int>
inline void intSerialize(Iterator buf, Int i)
{
	if (i < 0) throw std::out_of_range("cannot pack negative integers");
	if (i >= IntMaxSize<N>::max) throw
		std::out_of_range("cannot pack so large an integer");
	
	return intSerializeFast<N>(buf, i);
}

} // detail
} // JFA

#endif
