#include <stdio.h>

extern int demo(const char** a, const char** b);

int main(int argc, const char** argv)
{
	const char* s;
	const char* e;
	
	if (argc != 2)
	{
		fprintf(stderr, "Syntax: %s <string to test>\n", argv[0]);
		return 1;
	}
	
	s = argv[1];
	e = argv[1]+strlen(argv[1]);
	
	demo(&s, &e);
	(*(char*)e) = 0;
	if (s)	printf("Match: %s\n", s);
	else	printf("Miss.\n");
	
	return 0;
}
