#include <stdio.h>

extern int demo(const char* a, const char* b);

int main(int argc, const char** argv)
{
	int	out;
	int	i;
	int	len;
	int	add;
	int	line;
	FILE*	f;
	char	buf[8096];
	
	if (argc < 2)
	{
		fprintf(stderr, "Syntax: %s <files to scan>\n", argv[0]);
		return 1;
	}
	
	out = 0;
	for (i = 1; i < argc; i++)
	{
		f = fopen(argv[i], "r");
		if (!f)
		{
			perror(argv[i]);
			return 1;
		}
		
		line = 0;
		while (fgets(&buf[0], sizeof(buf), f))
		{
			len = strlen(&buf[0]);
			if (buf[len-1] == '\n')
			{
				buf[--len] = 0;
				add = 1;
			}
			else
			{
				add = 0;
			}
			
			if (demo(&buf[0], &buf[len]))
			{
				out = 1;
				printf("%s:%i:%s\n", argv[i], line, &buf[0]);
			}
			
			line += add;
		}
	}
	
	return out;
}
