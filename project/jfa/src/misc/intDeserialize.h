/*  $Id: intDeserialize.h,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  intDeserialize.h - A template-driven integer serializer
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

#ifndef __JFA_INT_DESERIALIZE_H__
#define __JFA_INT_DESERIALIZE_H__

/* A helper which deserializes any integer from any byte-buffer iterator.
 * Usage: intDeserialize<3>(it, num);
 * 
 * This will use 3 bytes on the stream to decode num.
 * No exceptions are thrown.
 * 
 * You must ensure prior to call that there are the correct number of bytes
 * avaliable. The iterator will not be repositioned by this call.
 */

namespace JFA
{
namespace detail
{

template <typename Iterator, typename Int, int N>
struct IntDeserializor
{
	static void deserialize(Iterator buf, Int& i)
	{
		i <<= 8;
		i |= (*buf & 0xFF);
		IntDeserializor<Iterator, Int, N-1>::deserialize(buf+1, i);
	}
};

template <typename Iterator, typename Int>
struct IntDeserializor<Iterator, Int, 0>
{
	static void deserialize(Iterator buf, Int& i)
	{
		// noop
	}
};
 	
template <int N, typename Iterator, typename Int>
inline void intDeserialize(Iterator buf, Int& i)
{
	i = 0;
	return IntDeserializor<Iterator, Int, N>::
 			deserialize(buf, i);
}

} // detail
} // JFA

#endif
