/*  $Id: esortc.cpp,v 1.3 2003-08-16 18:36:20 terpstra Exp $
 *  
 *  esortc.cpp - C binding for the ESort API
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

#include <esort.h>
#include <stdlib.h>
#include <assert.h>

#include "esortc.h"

extern "C" {

const char* esort_walker_key(Walker w)
{
	ESort::Walker* rw = reinterpret_cast<ESort::Walker*>(w);
	return rw->key.c_str();
}

unsigned long esort_walker_keylen(Walker w)
{
	ESort::Walker* rw = reinterpret_cast<ESort::Walker*>(w);
	return rw->key.length();
}

int esort_walker_advance(Walker w)
{
	ESort::Walker* rw = reinterpret_cast<ESort::Walker*>(w);
	return rw->advance();
}

void esort_walker_destroy(Walker w)
{
	ESort::Walker* rw = reinterpret_cast<ESort::Walker*>(w);
	delete rw;
}

Reader esort_reader_opendb(const char* database)
{
	std::auto_ptr<ESort::Reader> rr(ESort::Reader::opendb(database));
	return rr.release(); // can't do memory management in C
}

Reader esort_reader_opendb_extended(const char* database, int synced, int unique, unsigned int version, unsigned long blockSize, unsigned long keySize)
{
	std::auto_ptr<ESort::Reader> rr(ESort::Reader::opendb(database,
		ESort::Parameters(synced, unique, version, blockSize, keySize)));
	return rr.release(); // can't do memory management in C
}

void esort_reader_destroy(Reader r)
{
	// will work for Writer's too due to virtual ~Reader, but shhh...
	ESort::Reader* rr = reinterpret_cast<ESort::Reader*>(r);
	delete rr;
}

Writer esort_writer_opendb(const char* database)
{
	std::auto_ptr<ESort::Writer> rw(ESort::Writer::opendb(database));
	return rw.release(); // can't do memory management in C
}

Writer esort_writer_opendb_extended(const char* database, int mode, int synced, int unique, unsigned int version, unsigned long blockSize, unsigned long keySize)
{
	std::auto_ptr<ESort::Writer> rw(ESort::Writer::opendb(database,
		ESort::Parameters(synced, unique, version, blockSize, keySize), mode));
	return rw.release(); // can't do memory management in C
}

void esort_writer_destroy(Writer w)
{
	ESort::Writer* rw = reinterpret_cast<ESort::Writer*>(w);
	delete rw;
}

Walker esort_reader_seek(Reader r, const char* key, unsigned long keylen, int direction)
{
	ESort::Reader* rr = reinterpret_cast<ESort::Reader*>(r);
	ESort::Direction d = (ESort::Direction)direction;
	std::auto_ptr<ESort::Walker> o(rr->seek(std::string(key, keylen), d));
	return o.release();
}

Walker esort_reader_seekp(Reader r, const char* prefix, unsigned long prefixlen, const char* key, unsigned long keylen, int direction)
{
	ESort::Reader* rr = reinterpret_cast<ESort::Reader*>(r);
	ESort::Direction d = (ESort::Direction)direction;
	std::auto_ptr<ESort::Walker> o(rr->seekp(std::string(prefix, prefixlen), std::string(key, keylen), d));
	return o.release();
}

Walker esort_writer_seek(Writer w, const char* key, unsigned long keylen, int direction)
{
	ESort::Writer* rw = reinterpret_cast<ESort::Writer*>(w);
	ESort::Direction d = (ESort::Direction)direction;
	std::auto_ptr<ESort::Walker> o(rw->seek(std::string(key, keylen), d));
	return o.release();
}

Walker esort_writer_seekp(Writer w, const char* prefix, unsigned long prefixlen, const char* key, unsigned long keylen, int direction)
{
	ESort::Writer* rw = reinterpret_cast<ESort::Writer*>(w);
	ESort::Direction d = (ESort::Direction)direction;
	std::auto_ptr<ESort::Walker> o(rw->seekp(std::string(prefix, prefixlen), std::string(key, keylen), d));
	return o.release();
}

int esort_writer_insert(Writer w, const char* key, unsigned long keylen)
{
	ESort::Writer* rw = reinterpret_cast<ESort::Writer*>(w);
	return rw->insert(std::string(key, keylen));
}

int esort_writer_commit(Writer w)
{
	ESort::Writer* rw = reinterpret_cast<ESort::Writer*>(w);
	return rw->commit();
}

int esort_writer_rollback(Writer w)
{
	ESort::Writer* rw = reinterpret_cast<ESort::Writer*>(w);
	return rw->rollback();
}

} // extern C
