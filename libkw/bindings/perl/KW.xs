#include "EXTERN.h"
#include "perl.h"
#include "XSUB.h"

#include "esort.h"

MODULE = KW		PACKAGE = KW

void
hello()
	CODE:
		printf("Hello, world!\n");

int is_even(int input)
	CODE:
		RETVAL = (input % 2 == 0);
	OUTPUT:
		RETVAL
