/*  $Id: esort.h,v 1.4 2003-04-24 11:56:18 terpstra Exp $
 *  
 *  esort.h - Public interface to libesort
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

#ifndef ESORT_H
#define ESORT_H

#include <string>

/* What is ESort? -- An Online External Sort
 * 
 * ESort provides a very primitive database API: An insert-only set of keys.
 * 
 * There are very different trade-offs when compared with a standard database.
 *
 * N = number of keys in the set, M = number of operations
 * The seeks needed to for each repeated operation in one transaction:
 *
 *                    ESort           BTree            Hash
 * Insertion          O(1)            O(M*log(N))      O(M)
 * Read in sequence   O(1)            O(M)             impossible
 * Seek to key        O(M*log^2(N))   O(M*log(N))      O(M)
 * Delete             impossible      O(M*log(N))      O(M)
 * 
 * From this table, one can conclude that ESort allows blindly fast insertion
 * and sequential read, but pays in the cost to seek. This is what makes it
 * highly appropriate for keyword indexes.
 * 
 * An additional restriction is that ESort only supports a single-writer.
 * An additional advantage is that ESort readers get snap-shots.
 */

namespace ESort
{

using std::string;

/** These parameters specify minimum values required for a database.
 *  If an existing database has too small a value, an error is returned.
 *  The unique flag toggles whether the database is a set or multiset.
 */
class Parameters
{
 protected:
	unsigned int	version_;
	unsigned long	blockSize_;
	unsigned long	keySize_;
	bool		unique_;
	unsigned int	keyWidth_;

 public:
 	// keySize & blockSize are the number of bytes maximum
	Parameters(
		unsigned int  version   = 1, 
		unsigned long blockSize = 8192, 
		unsigned long keySize   = 255, 
		bool          unique    = true);
	
	bool isWider(const Parameters& o);
	
	unsigned int  version  () const { return version_;   }
	unsigned long blockSize() const { return blockSize_; }
	unsigned long keySize  () const { return keySize_;   }
	bool          unique   () const { return unique_;    }
	unsigned int  keyWidth () const { return keyWidth_;  }
};

/** The Walker class walks the database in sorted order.
 *  As from the chart above, each invokation of advance() pays 0 seeks.
 */
class Walker
{
 public:
 	/** The current key which the walker points to.
 	 */
 	string		key;
 	
 	/** Advance to the next key in the database.
 	 *  Returned is the number of bytes in common with the last key.
 	 *  -1 is returned on error, errno = 0 -> reached EOF.
 	 */
	virtual int advance() = 0;
	
 	virtual ~Walker();
};

/** A Reader object allows you to obtain Walkers.
 *  
 *  A Reader is atomic. When you obtain a Reader, the contents of the set
 *  remain fixed for as long as you keep a hold of the handle. Furthermore,
 *  a Reader will take snapshots of the database only between Writer commits.
 */
class Reader
{
 public:
 	virtual ~Reader();
 	
 	/** Obtain a walker pointing to the first key >= k.
 	 *  This costs O(log^2(N)) seeks, so try not to call it too often.
 	 *  Failure returns 0 and sets errno, errno = 0 -> eof.
 	 */
 	virtual Walker* seek(const string& k) = 0;
 	
 	/** Open an existing database for reading.
 	 *  0 is returned and errno set on error.
 	 */
 	static Reader* open(const string& db);
};

/** The Writer object allows you to insert keys.
 *
 *  The Writer is atomic. When a key is inserted, it immediately becomes
 *  available in all Walkers & Readers obtained from the Writer. However,
 *  all open Readers never see the inserted keys, and new Readers do not
 *  see them until the Writer calls commit.
 */
class Writer : public Reader
{
 public:
 	/** Insert a key into the database.
 	 *  If an error occures, -1 is returned and errno set, else 0.
 	 *  Be sure the key is smaller than keySize on pain of EINVAL.
 	 */
 	virtual int insert(const string& k) = 0;
 	
 	/** Commit the changes to the database.
 	 *  If an error occures, -1 is returned and errno set, else 0.
 	 */
 	virtual int commit() = 0;
 	
 	/** Open a database for writing.
 	 *  It is created if it did not exist.
 	 *  
 	 *  The mode is identical to open(2).
 	 *  
 	 *  0 is returned and errno set on error.
 	 */
 	static Writer* open(
 		const string& db, const Parameters& p = Parameters(), 
 		int mode = 0666);
};

}

#endif
