#include <stdio.h>
#include <string.h>

#define TOKEN_REG       "([A-Za-z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9]|[a-zA-Z0-9])"
#define HOST_REG	"((" TOKEN_REG "\\.)*" TOKEN_REG ")"
#define USER_REG	"([A-Za-z0-9][a-zA-Z0-9._=\\-]*[a-zA-Z0-9]|[a-zA-Z0-9])"
#define FILE_REG        "([A-Za-z%~0-9:.,_\\-]+)"
#define PATH_REG	"((/" FILE_REG ")+)"
#define POST_REG   	"([A-Za-z%~0-9.,_=/:;+\\&\\-]+)"
#define PROTO_REG   	"([a-zA-Z]+)"
#define INDENT_REG 	"([a-zA-Z]?[a-zA-Z]?[a-zA-Z]?[a-zA-Z]?[>:] )"

#define URL_REG    	"(((" PROTO_REG "://" HOST_REG "(" PATH_REG ")?)" \
                         "|(" HOST_REG "\\." TOKEN_REG PATH_REG ")" \
                         "|(www\\." HOST_REG "\\." TOKEN_REG "(" PATH_REG ")?)" \
                    	 ")" \
                    	 "(#"   FILE_REG ")?" \
                    	 "(\\?" POST_REG ")?" \
                        ")"

#define EMAIL_REG       "(" USER_REG "@" HOST_REG "\\." TOKEN_REG ")"

#define NOT_LF		"[\001-\011\013-\377]"
#define QUOTE_REG       "(\n" INDENT_REG NOT_LF "*\n(" INDENT_REG NOT_LF "*\n)*)"

#define ASCII_ART_REG   "((" NOT_LF "*  " NOT_LF "*  " NOT_LF "*)" \
			"|(" NOT_LF "*\t" NOT_LF "*))"
#define ASCII_PIC_REG   "(\n" ASCII_ART_REG "\n(" ASCII_ART_REG "\n)*)"

int main(int argc, const char** argv)
{
	if (!strcmp(argv[1], "url"))	printf("%s\n", URL_REG);
	if (!strcmp(argv[1], "mailto"))	printf("%s\n", EMAIL_REG);
	if (!strcmp(argv[1], "quote"))	printf("%s\n", QUOTE_REG);
	if (!strcmp(argv[1], "art"))	printf("%s\n", ASCII_PIC_REG);
	
	return 0;
}
