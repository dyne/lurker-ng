/*  $Id: FileSystemVolumeManager.cpp,v 1.1.1.1 2003-01-04 21:13:57 terpstra Exp $
 *  
 *  FileSystemVolumeManager.cpp - Maintain the id <-> filename mapping
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

#include <map>
#include <fstream>

#include "FileSystem.h"
#include "../Exception.h"

#define IDS_NAME	"/volume.ids"

namespace JFA
{
namespace detail
{

class FileSystemVolumeManager : public FileSystemVolumeManagerI
{
 protected:
 	std::string	m_path;
 	int		m_mode;
 	sectorlen_t	m_len;
 	
 protected:
 	typedef std::map<std::string, int>	VolumeIds;
 	typedef std::map<int, VolumeI*>		VolumeObjs;
 	
 	VolumeIds	m_volume_ids;
 	VolumeObjs	m_volume_objs;
 	
 	void	load_ids () throw (env_exception);
 	void	write_ids() throw (env_exception);
 	
 public:
 	FileSystemVolumeManager(
 			const std::string&	path, 
 			int			mode,
 			sectorlen_t		len)
 		throw(env_exception);
 	~FileSystemVolumeManager();
 	
 	VolumeI*	get_volume(const std::string&	name) throw (env_exception);
 	VolumeI*	get_volume(volume_t		id)   throw (env_exception);
 	
 	std::string	get_path() const;
 	int		get_mode() const;
};

FileSystemVolumeManager::FileSystemVolumeManager(
		const std::string&	path, 
		int			mode,
		sectorlen_t		len)
	throw(env_exception)
 : m_path(path), m_mode(mode), m_len(len)
{
	load_ids();
}

FileSystemVolumeManager::~FileSystemVolumeManager()
{
	VolumeObjs::iterator i;
	for (i = m_volume_objs.begin(); i != m_volume_objs.end(); ++i)
	{
		delete i->second;
	}
}

void FileSystemVolumeManager::load_ids() throw (env_exception)
{
	std::string ids = m_path + IDS_NAME;
	std::ifstream f(ids.c_str());
	
	// if cannot open, assume DNE
	if (!f.is_open()) return;
	
	std::string volume;
	int id;
	while (f >> volume >> id)
	{
		m_volume_ids[volume] = id;
		m_volume_objs[id] = FileSystemVolumeFactory(
			this, volume, id, m_len, false);
	}
}

void FileSystemVolumeManager::write_ids() throw (env_exception)
{
	std::string ids = m_path + IDS_NAME;
	std::ofstream f(ids.c_str());
	
	if (!f.is_open())
		THROW(corrupt_exception, "cannot open ids file for writing");
	
	VolumeIds::iterator i;
	for (i = m_volume_ids.begin(); i != m_volume_ids.end(); ++i)
	{
		f << i->first << " " << i->second << "\n";
	}
	f.close();
	
	if (f.fail())
		THROW(corrupt_exception, "cannot write to ids file");
}

VolumeI* FileSystemVolumeManager::get_volume(const std::string& name)
	throw (env_exception)
{
	VolumeIds::iterator i = m_volume_ids.find(name);
	if (i != m_volume_ids.end()) return get_volume(i->second);
	
	// new volume name...
	volume_t id;
	
	do { id = random() % 0xFFFF; }
	while (m_volume_objs.find(id) != m_volume_objs.end());
	
	VolumeI* impl = FileSystemVolumeFactory(this, name, id, m_len, true);
	m_volume_ids[name] = id;
	m_volume_objs[id] = impl;
	
	write_ids();
	
	return impl;
}

VolumeI* FileSystemVolumeManager::get_volume(volume_t id)
	throw (env_exception)
{
	VolumeObjs::iterator i = m_volume_objs.find(id);
	if (i != m_volume_objs.end())
		return i->second;
	return 0;
}

std::string FileSystemVolumeManager::get_path() const
{
	return m_path;
}

int FileSystemVolumeManager::get_mode() const
{
	return m_mode;
}

FileSystemVolumeManagerI* FileSystemVolumeManagerFactory(
		const std::string&		path,
		int				mode,
		sectorlen_t			len)
	throw (env_exception)
{
	return new FileSystemVolumeManager(path, mode, len);
}
						
} // detail
} // JFA
