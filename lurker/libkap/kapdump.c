/*  $Id: kapdump.c,v 1.1 2002-07-04 11:43:40 terpstra Exp $
 *  
 *  kapmake.c - Implementation of an import tool
 *  
 *  Copyright (C) 2002 - Wesley W. Terpstra
 *  
 *  License: LGPL
 *  
 *  Authors: 'Wesley W. Terpstra' <wesley@terpstra.ca>
 *  
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 2 of the License.
 *    
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *    
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#define _XOPEN_SOURCE 500
#define _BSD_SOURCE
#define _ISOC99_SOURCE

#include "../config.h"
#include "kap.h"

#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>	

#ifdef HAVE_GETTEXT
#include <libintl.h>
#else
#include "../intl/libgnuintl.h"
#endif

#define _(String) gettext (String)
#define gettext_noop(String) (String)
#define N_(String) gettext_noop (String)

void help()
{
	fputs("\n", stderr);
	fputs(_("kapdump [options] <file>\n"), stderr);
	fputs("\n", stderr);
	
	fputs(_("\t-v        \tDisplay version information and exit\n"), stderr);
	fputs(_("\t-h        \tDisplay this message and exit\n"), stderr);
	fputs(_("\t-b        \tUse only the btree layer\n"), stderr);
	fputs(_("\t-s <bytes>\tSector size for the btree                   (8096)\n"), stderr);
	fputs(_("\t-k <bytes>\tMaximum size of input key                    (100)\n"), stderr);
	fputs(_("\t-t <bytes>\tNumber of bytes used in tree pointers          (3)\n"), stderr);
	fputs(_("\t-l <bytes>\tMaximum number of bytes used in leaf records (252)\n"), stderr);
	fputs(_("\t-r <bytes>\tSize of append records                         (4)\n"), stderr);
	fputs("\n", stderr);
	fputs(_("\t<file>\tThe database to dump\n"), stderr);
	fputs("\n", stderr);
}

void bail(int err, const char* op)
{
	fprintf(stderr, _("Failed to read the database: %s: %s\n"), op,
		kap_strerror(err));
	exit(1);
}

int main(int argc, char * const * argv)
{
	int out;
	int c;
	Kap k;
	
	const char*	file;
	
	ssize_t	sector_size	= 8096;
	ssize_t	max_key_size	= 100;
	short	tree_size	= 3;
	ssize_t	leaf_size	= 252;
	ssize_t	record_size	= 4;
	
	int mode = KAP_BTREE | KAP_APPEND;
	
	while ((c = getopt(argc, argv, "hvbs:k:t:l:r:")) != -1)
	{
		switch (c)
		{
		case 'v':
			fputs("\n", stderr);
			fprintf(stderr, 
				_("Kap database format revision: %d.%d\n"), 
				KAP_FORMAT_MAJOR, KAP_FORMAT_MINOR);
			fprintf(stderr, 
				_("Kap tools version:            %d.%d.%d\n"), 
				KAP_FORMAT_MAJOR, KAP_FORMAT_MINOR, KAP_TOOLS);
			fputs("\n", stderr);
			return 0;
		
		case 'h':
			help();
			return 0;
			
		case 'b':
			mode &= ~KAP_APPEND;
			break;
		
		case 's':
			sector_size = atol(optarg);
			break;
		
		case 'k':
			max_key_size = atol(optarg);
			break;
		
		case 't':
			tree_size = atol(optarg);
			break;
		
		case 'l':
			leaf_size = atol(optarg);
			break;
		
		case 'r':
			record_size = atol(optarg);
			break;
		
		default:
			help();
			return 1;
		}
	}
	
	if (optind + 1 != argc)
	{
		help();
		return 1;
	}
	
	file    = argv[optind];
	
	/* Minimize all parameters so we can do the user ones in order */
	out = kap_create(&k, KAP_BTREE);
	if (out != 0) bail(out, "kap_create");
	
	out = kap_btree_set_maxkeysize(k, 2);
	if (out != 0) bail(out, "kap_btree_set_maxkeysize");
	
	out = kap_btree_set_treesize(k, 1);
	if (out != 0) bail(out, "kap_btree_set_treesize");
	
	out = kap_btree_set_leafsize(k, 1);
	if (out != 0) bail(out, "kap_btree_set_leafsize");
	
	out = kap_btree_set_sectorsize(k, sector_size);
	if (out != 0) bail(out, "kap_btree_set_sectorsize");
	
	out = kap_btree_set_maxkeysize(k, max_key_size);
	if (out != 0) bail(out, "kap_btree_set_maxkeysize");
	
	out = kap_btree_set_leafsize(k, leaf_size);
	if (out != 0) bail(out, "kap_btree_set_leafsize");
	
	out = kap_btree_set_treesize(k, tree_size);
	if (out != 0) bail(out, "kap_btree_set_treesize");
	
/*
	out = kap_append_set_recordsize(k, record_size);
	if (out != 0) bail(out, "kap_append_set_recordsize");
*/
	
	
	return 0;
}
