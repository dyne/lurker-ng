/*  $Id: handler.h,v 1.6 2002-02-22 00:53:11 terpstra Exp $
 *  
 *  main.c - render missing pages
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

#include <stdio.h>

typedef enum lu_doctype_t { LU_HTML, LU_XML, LU_OTHER } lu_doctype;

extern FILE* lu_server_link;

extern FILE* lu_render_open(const char* parameter);
extern int   lu_render_close(FILE* f);
extern int   lu_forward_xml(const char* parameter);

extern int lu_message_handler(char* parameter, const char* uri, lu_doctype t);
extern int lu_thread_handler (char* parameter, const char* uri, lu_doctype t);
extern int lu_mindex_handler (char* parameter, const char* uri, lu_doctype t);
extern int lu_search_handler (char* parameter, const char* uri, lu_doctype t);
extern int lu_splash_handler (char* parameter, const char* uri, lu_doctype t);

extern const char basic_error[];    /* %s %s %s */
extern const char redirect_error[]; /* %s */
extern const char not_found[];      /* %s */
