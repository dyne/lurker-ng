/*  $Id: File.cpp,v 1.8 2003-05-06 14:32:32 terpstra Exp $
 *  
 *  File.cpp - Disk segment for commit'd inserts
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

#include "File.h"
#include "Source.h"
#include "esort.h"

/* #define DEBUG */

#include <cassert>
#include <iostream>

namespace ESort
{

class FileSource : public Source
{
 protected:
 	File* f;
 	long block;
 	unsigned char* buf;
 	unsigned char* off;
 	bool forward;
 	
 	unsigned char* inverseBuffer();
 	
 public:
 	FileSource(File* f_, long block_, bool forward_);
 	~FileSource();
 	
 	int loadBuf();
 	int advance();
};

FileSource::FileSource(File* f_, long block_, bool forward_)
 : f(f_), block(block_),
   buf(new unsigned char[f->p->blockSize()]), off(0), forward(forward_)
{
}

FileSource::~FileSource()
{
	delete [] buf;
}

int FileSource::loadBuf()
{
	assert (block <= f->blocks && block >= -1);
	
	if (forward)
	{
		if (block == f->blocks)
		{	// hit eof
			errno = 0;
			return -1;
		}
	}
	else
	{
		if (block == -1)
		{	// hit eof
			errno = 0;
			return -1;
		}
	}
	
	if (f->where != block)
	{
		off_t go = block;
		go *= f->p->blockSize();
		
		if (lseek(f->fd, go, SEEK_SET) != go)
		{
#ifdef DEBUG
			perror(__FILE__ ":" #__LINE__ ": lseek");
#endif
			// keep errno
			return -1;
		}
	}
	
	if (read(f->fd, buf, f->p->blockSize()) != (ssize_t)f->p->blockSize())
	{
#ifdef DEBUG
		perror(__FILE__ ":" #__LINE__ ": read");
#endif
		return -1;
	}
	
	if (forward)
	{
		f->where = ++block;
		off = buf;
	}
	else
	{
		f->where = block+1;
		--block;
		
		off = inverseBuffer(); // flip the order of the records
	}
	
	return 0;
}

unsigned char* FileSource::inverseBuffer()
{
	unsigned char* target  = new unsigned char[f->p->blockSize()];
	unsigned char* scratch = new unsigned char[f->p->keySize()];
	
	unsigned char* r = buf;
	unsigned char* w = target + f->p->blockSize();
	
	long thisLength, nextLength;	// total key length
	long thisDup,    nextDup;	// amount in common
	long thisDelta,  nextDelta;	// amount unique
	
	//----------------
	
	// teminate the scratch (0, 1) == eof
	--w;
	*w = 1; 
	w -= f->p->keyWidth();
	memset(w, 0, f->p->keyWidth());
	
	thisLength = 0;
	for (unsigned int i = 0; i < f->p->keyWidth(); ++i)
	{
		thisLength <<= 8;
		thisLength += *r;
		++r;
	}
	thisDup = *r;
	++r;
	thisDelta = thisLength - thisDup;
	if (thisDelta < 0) return w; // corrupt! must have one record at least
	
	while (thisDelta != -1)
	{
		// r points at the unique tail
		// w point at the last completed record
		
		// Read the next record's values
		nextLength = 0;
		unsigned char* nextr = r + thisDelta;
		for (unsigned int i = 0; i < f->p->keyWidth(); ++i)
		{
			nextLength <<= 8;
			nextLength += *nextr;
			++nextr;
		}
		
		nextDup = *nextr;
		++nextr;
		
		nextDelta = nextLength - nextDup;
		if (nextDelta < -1) return w; // corrupt! (-1 == eof)
		if (nextDelta == -1) nextDup = 0; // first record has 0 dup'd
		
		// Skip to next cell
		long wDelta = thisLength - nextDup;
		unsigned char* tailw = w - wDelta;
		unsigned char* nextw = tailw - 1 - f->p->keyWidth();
		
		// Write out the lengh
		for (int i = f->p->keyWidth()-1; i >= 0; --i)
		{
			nextw[i] = thisLength;
			thisLength >>= 8;
		}
		
		// We use nextDup for our dup
		*(tailw-1) = nextDup;
		
		// Now, write out the tail
		memcpy(scratch+thisDup, r, thisDelta);
		memcpy(tailw, scratch+nextDup, wDelta);
		
//		std::cout << " SCRATCH: ";
//		std::cout.write((const char*)scratch, thisDup + thisDelta);
//		std::cout << std::endl;
		
		// move along
		thisLength = nextLength;
		thisDup    = nextDup;
		thisDelta  = nextDelta;
		r = nextr;
		w = nextw;
	}
	
	delete [] scratch;
	delete [] buf;
	buf = target;
	
	return w;
}

int FileSource::advance()
{
//	printf("off: %d, buf: %d, wid: %d\n", off, buf, f->p->keyWidth());
	assert ((off - buf) + f->p->keyWidth() + 1 <= f->p->blockSize());
	
	length = 0;
	while (1)
	{
		for (unsigned int i = 0; i < f->p->keyWidth(); ++i)
		{
			length <<= 8;
			length += *off;
			++off;
		}
		
		dup = *off;
		++off;
		
		if (length != 0 || dup != 1) break; // (0, 1) == eof
		
		if (loadBuf() != 0) return -1;
	}
	
	tail = off;
	off += length - dup;
	
//	std::cout << " GET: " << length << ", " << dup << ": ";
//	std::cout.write((const char*)tail, length-dup);
//	std::cout << std::endl;
	
	return 0;
}

File::File(const string& id_, int fd_, const Parameters* p_)
 : fd(fd_), p(p_), where(0), id(id_)
{
	off_t len = lseek(fd, 0, SEEK_END);
	assert (len % p->blockSize() == 0);
	
	where = blocks = len / p->blockSize();
	category = categorize(blocks);
}

File::File(const File& f)
 : fd(dup(f.fd)), p(f.p), where(f.where), id(f.id), blocks(f.blocks), category(f.category)
{
}

File::~File()
{
	close(fd);
}

auto_ptr<Source> File::openBlock(long block, bool forward)
{
	assert (block <= blocks);
	
	auto_ptr<FileSource> out(new FileSource(this, block, forward));
	
	if (out->loadBuf() != 0)
		return auto_ptr<Source>(0);
	
	return auto_ptr<Source>(out);
}

}
