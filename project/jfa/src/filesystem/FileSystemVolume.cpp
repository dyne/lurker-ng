/*  $Id: FileSystemVolume.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  FileSystemVolume.cpp - Block-based file allocator
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

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

/* #define DEBUG 1 */

#include <unistd.h>
#include <fcntl.h>

#include "../Exception.h"
#include "FileSystem.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

// #define WATCH_IO

#ifdef WATCH_IO
#include <stdio.h>
#endif

namespace JFA
{
namespace detail
{

class FileSystemVolume : public VolumeI
{
 protected:
 	volume_t	m_id;
 	recordptr_t	m_pos;
 	sectorlen_t	m_sector_len;
 	filedes_t	m_des;
 	
 public:
 	void pull_disk(recordptr_t, MemBlock<data_t>       d) throw (trans_exception);
 	void push_disk(recordptr_t, MemBlock<const data_t> d) throw (trans_exception);
 	
 	recordptr_t get_end() throw (trans_exception);
 	
 	void truncate(recordptr_t last) throw(trans_exception);
 	
 	void sync() throw(trans_exception);
 	
 	volume_t get_id() const throw();
 	sectorlen_t get_sector_size() const throw();
 	 		
 public:
 	FileSystemVolume(
 		FileSystemVolumeManagerI*	env,
 		const std::string&		name, 
 		volume_t			id,
 		sectorlen_t			len,
 		bool				create)
 		throw (env_exception);
 	
 	~FileSystemVolume();
};


FileSystemVolume::FileSystemVolume(
	FileSystemVolumeManagerI*	env,
	const std::string&		name,
	volume_t			id,
	sectorlen_t			len,
	bool				create)
	throw (env_exception)
 : m_id(id), m_pos(0), m_sector_len(len), m_des(-1)
{
	std::string path = env->get_path() + "/db." + name;
	
	m_des = ::open(path.c_str(),
		O_BINARY | O_RDWR | (create?O_CREAT:0),
		env->get_mode());
	
	if (m_des == -1)
		THROW(corrupt_exception, "cannot open database");
}

FileSystemVolume::~FileSystemVolume()
{
	if (m_des != -1) ::close(m_des);
}

void FileSystemVolume::pull_disk(recordptr_t sector, MemBlock<data_t> m)
	throw (trans_exception)
{
	assert (m.len == m_sector_len);
#ifdef WATCH_IO
	printf("READ %d\n", (int)sector);
#endif
	if (m_pos != sector)
	{
		if (::lseek(m_des, sector * m.len, SEEK_SET) != off_t(sector * m.len))
			THROW(io_retry_exception, "cannot lseek volume");
		m_pos = sector;
	}
	
	if (::read(m_des, m.buf, m.len) != ssize_t(m.len))
	{
		m_pos = -1;
		THROW(io_retry_exception, "cannot read volume");
	}
	
	++m_pos;
}

void FileSystemVolume::push_disk(recordptr_t sector, MemBlock<const data_t> m)
	throw (trans_exception)
{
	assert (m.len == m_sector_len);
#ifdef WATCH_IO
	printf("WRITE %d\n", (int)sector);
#endif
	if (m_pos != sector)
	{
		if (::lseek(m_des, sector * m.len, SEEK_SET) != off_t(sector * m.len))
			THROW(io_retry_exception, "cannot lseek volume");
		m_pos = sector;
	}
	
	if (::write(m_des, m.buf, m.len) != ssize_t(m.len))
	{
		m_pos = -1;
		THROW(io_retry_exception, "cannot write volume");
	}
	
	++m_pos;
}

recordptr_t FileSystemVolume::get_end()
	throw (trans_exception)
{
	m_pos = -1;
	
	off_t pos = ::lseek(m_des, 0, SEEK_END);
	assert (pos % m_sector_len == 0);
	return pos / m_sector_len;
}

void FileSystemVolume::truncate(recordptr_t last)
	throw (trans_exception)
{
	m_pos = -1;
	
	if (::ftruncate(m_des, off_t(last) * m_sector_len) != 0)
		THROW(io_retry_exception, "cannot ftruncate volume");
}

void FileSystemVolume::sync() throw (trans_exception)
{
	if (::fsync(m_des) != 0)
		THROW(io_retry_exception, "cannot fsync volume");
}

volume_t FileSystemVolume::get_id() const throw ()
{
	return m_id;
}

sectorlen_t FileSystemVolume::get_sector_size() const throw ()
{
	return m_sector_len;
}

VolumeI* FileSystemVolumeFactory(
		FileSystemVolumeManagerI*	manager,
		const std::string&		name,
		volume_t			id,
		sectorlen_t                     len,
		bool                            create)
	throw (env_exception)
{
	return new FileSystemVolume(manager, name, id, len, create);
}
				                                
} // detail
} // JFA
