#include <unistd.h>
#include <string.h>

#define TOKEN_REG       "([A-Za-z0-9][a-zA-Z0-9\\-]*[a-zA-Z0-9]|[a-zA-Z0-9])"
#define HOST_REG	"((" TOKEN_REG "\\.)*" TOKEN_REG ")"
#define USER_REG	"([A-Za-z0-9][a-zA-Z0-9._=\\-]*[a-zA-Z0-9]|[a-zA-Z0-9])"
#define FILE_REG        "([A-Za-z%~0-9:.,_\\-]+)"
#define PATH_REG	"((/" FILE_REG ")+)"
#define POST_REG   	"([A-Za-z%~0-9.,_=/:;+\\&\\-]+)"
#define PROTO_REG   	"([a-zA-Z]+)"
#define INDENT_REG 	"([a-zA-Z]?[a-zA-Z]?[a-zA-Z]?[a-zA-Z]?>)"

#define URL_REG    	"(((" PROTO_REG "://" HOST_REG "(" PATH_REG ")?)" \
                         "|(www\\." HOST_REG "\\." TOKEN_REG ")" \
                         ")" \
                         "("    PATH_REG ")?" \
                    	 "(#"   FILE_REG ")?" \
                    	 "(\\?" POST_REG ")?" \
                        ")"

#define EMAIL_REG       "(" USER_REG "@" HOST_REG "\\." TOKEN_REG ")"

#define NOT_LF		"[\001-\011\013-\377]"
#define QUOTE_REG       "(\n" INDENT_REG NOT_LF "*\n(" INDENT_REG NOT_LF "*\n)*)"

#define ASCII_ART_REG   "(" NOT_LF "*" \
			"[\001-\011\013-\037\041-\\\055\057-\075\077-\377]  " NOT_LF "*" \
			"[\001-\011\013-\037\041-\\\055\057-\075\077-\377]  " NOT_LF "*\n)"
#define ASCII_ROWS_REG	"(" NOT_LF "+\n)*"

#define ASCII_PIC_REG   "\n" ASCII_ROWS_REG ASCII_ART_REG \
			     ASCII_ROWS_REG ASCII_ART_REG \
			     ASCII_ROWS_REG

int main(int argc, const char** argv)
{
	if (!strcmp(argv[1], "url"))	write(1, URL_REG,       sizeof(URL_REG)-1);
	if (!strcmp(argv[1], "mailto"))	write(1, EMAIL_REG,     sizeof(EMAIL_REG)-1);
	if (!strcmp(argv[1], "quote"))	write(1, QUOTE_REG,     sizeof(QUOTE_REG)-1);
	if (!strcmp(argv[1], "art"))	write(1, ASCII_PIC_REG, sizeof(ASCII_PIC_REG)-1);
	
	return 0;
}
