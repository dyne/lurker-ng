/*  $Id: File.cpp,v 1.3 2003-04-24 23:52:36 terpstra Exp $
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
#include <cstdio>

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
 : f(f_), block(block_), forward(forward_),
   buf(new unsigned char[f->p->blockSize()]), off(0)
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
	// !!!
	return 0;
}

int FileSource::advance()
{
//	printf("off: %d, buf: %d, wid: %d\n", off, buf, f->p->keyWidth());
	assert ((off - buf) + f->p->keyWidth() <= f->p->blockSize());
	
	length = 0;
	while (1)
	{
		for (unsigned int i = 0; i < f->p->keyWidth(); ++i)
		{
			length <<= 8;
			length += *off;
			++off;
		}
		
		if (length != 0) break;
		
		if (loadBuf() != 0) return -1;
	}
	
	dup = *off;
	++off;
	
	tail = off;
	off += length - dup;
	
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
	
	if (out->loadBuf() == 0)
		return auto_ptr<Source>(out);
	// else return 0
}

}
