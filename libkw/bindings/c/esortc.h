/*  $Id: esortc.h,v 1.3 2003-08-16 18:36:20 terpstra Exp $
 *  
 *  esortc.h - Public interface to libesort for C
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

#ifndef ESORTC_H
#define ESORTC_H

/* For documentation see esort.h
 * The only documentation included here is what the binding does.
 */

/* Use these constants to determine the direction in seek methods */	
#define ESORT_FORWARD	1
#define ESORT_BACKWARD	2

/* Opaque datatypes */
typedef void* Walker;
typedef void* Reader;
typedef void* Writer;

#ifdef __cplusplus
/* If you are including this in C++ you are on serious crack */
extern "C" {
#endif

/* Read the current value of the key on the walker */
const char*	esort_walker_key(Walker);
unsigned long	esort_walker_keylen(Walker);

/* Advance the walker, -1 on error, errno = 0 -> EOF */
int		esort_walker_advance(Walker);

/* Destroy a walker when you are done with it */
void		esort_walker_destroy(Walker);

/* Methods for opening/closing a reader/writer, 0 = error + errno */
Reader	esort_reader_opendb(const char* database);
Reader	esort_reader_opendb_extended(const char* database, int synced, int unique, unsigned int version, unsigned long blockSize, unsigned long keySize);
void	esort_reader_destroy(Reader);

Writer	esort_writer_opendb(const char* database);
Writer	esort_writer_opendb_extended(const char* database, int mode, int synced, int unique, unsigned int version, unsigned long blockSize, unsigned long keySize);
void	esort_writer_destroy(Writer);

/* Methods for reading a Reader or Writer */
Walker	esort_reader_seek (Reader r, const char* key,    unsigned long keylen, int direction);
Walker	esort_reader_seekp(Reader r, const char* prefix, unsigned long prefixlen, const char* key, unsigned long keylen, int direction);
Walker	esort_writer_seek (Writer r, const char* key,    unsigned long keylen, int direction);
Walker	esort_writer_seekp(Writer r, const char* prefix, unsigned long prefixlen, const char* key, unsigned long keylen, int direction);

/* Methods for writing to a Writer */
int	esort_writer_insert  (Writer w, const char* key, unsigned long keylen);
int	esort_writer_commit  (Writer w);
int	esort_writer_rollback(Writer w);

#ifdef __cplusplus
}
#endif

#endif
