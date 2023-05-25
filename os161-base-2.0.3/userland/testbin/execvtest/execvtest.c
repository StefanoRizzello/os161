#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(){

	
	const char *programName = "/bin/ls";
	char *args[] = {programName, "-a", "/testbin", NULL};
	execv(programName, args);
	return 0;

}
