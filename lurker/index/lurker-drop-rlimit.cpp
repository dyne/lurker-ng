#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	struct rlimit rl;
	
	getrlimit(RLIMIT_FSIZE, &rl);
	rl.rlim_cur = rl.rlim_max;
	setrlimit(RLIMIT_FSIZE, &rl);
	
	execv(argv[1], argv+1);	
	perror("execv");
	return 1;
}
