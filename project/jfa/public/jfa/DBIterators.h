/*  $Id: DBIterators.h,v 1.1.1.1 2003-01-04 21:13:58 terpstra Exp $
 *  
 *  SectorIterator.h - User visible sector reader implementation
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

#ifndef __JFA_SECTOR_ITERATOR_H__
#define __JFA_SECTOR_ITERATOR_H__

template <class DerivedT>
class SectorIterator
{
 protected:
 	T&       derived() { return *static_cast<DerivedT      *>(this); }
 	T const& derived() { return *static_cast<DerivedT const*>(this); }
};

#endif
