/*  $Id: host.c,v 1.14 2003-06-12 22:56:01 terpstra Exp $
 *  
 *  host.c - collection of useful web regexps for reg2c
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

#include <unistd.h>
#include <string.h>

#define TOKEN_REG       "([A-Za-z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9]|[a-zA-Z0-9])"
#define INT_REG		"([0-9]+)"
#define HOST_REG	"((" TOKEN_REG "\\.)*" TOKEN_REG ")"
#define USER_REG	"([A-Za-z0-9][a-zA-Z0-9._=\\-]*[a-zA-Z0-9]|[a-zA-Z0-9])"
#define FILE_REG        "([A-Za-z@!%~0-9:.,_\\-+]*|#)"
#define TAIL_REG        "([A-Za-z@!%~0-9:.,_\\-+]*[A-Za-z0-9]|#)"
#define PATH_REG	"((/" FILE_REG ")*\\/" TAIL_REG ")"
#define POST_REG   	"([A-Za-z%~0-9.,_=/:;+\\&\\-]*[A-Za-z0-9]|#)"
#define PROTO_REG   	"(([a-zA-Z]-?)*[a-zA-Z])"
#define INDENT_REG 	"([a-zA-Z]?[a-zA-Z]?[a-zA-Z]?[a-zA-Z]?>)"

#define URL_REG    	"(((" PROTO_REG "://" HOST_REG ")" \
                         "|(www\\." HOST_REG "\\." TOKEN_REG ")" \
                         ")" \
                         "(:"	INT_REG  ")?" \
                         "("    PATH_REG ")?" \
                    	 "(\\#" TAIL_REG ")?" \
                    	 "(\\?" POST_REG ")?" \
                        ")"

#define EMAIL_REG       "(" USER_REG "@" HOST_REG "\\." TOKEN_REG ")"

#define NOT_LF		"[\001-\011\013-\377]"
#define QUOTE_REG       "(\n" INDENT_REG NOT_LF "*(\n" INDENT_REG NOT_LF "*)*)"

#define SIGBLOCK_REG	"(-- (\n" NOT_LF "+)+)"
#define ART_REG		"([\t ]" NOT_LF "+)"

#define ASCII_PIC_REG   "((\n(" SIGBLOCK_REG "|" ART_REG "))+\n)"

int main(int argc, const char** argv)
{
	if (!strcmp(argv[1], "host"))	write(1, HOST_REG,      sizeof(HOST_REG)-1);
	if (!strcmp(argv[1], "url"))	write(1, URL_REG,       sizeof(URL_REG)-1);
	if (!strcmp(argv[1], "mailto"))	write(1, EMAIL_REG,     sizeof(EMAIL_REG)-1);
	if (!strcmp(argv[1], "quote"))	write(1, QUOTE_REG,     sizeof(QUOTE_REG)-1);
	if (!strcmp(argv[1], "art"))	write(1, ASCII_PIC_REG, sizeof(ASCII_PIC_REG)-1);
	
	return 0;
}
