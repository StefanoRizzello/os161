#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

int main() {

	pid_t mypid;
	mypid = getpid();
	printf ("Il PID ha valore %d\n", mypid);
	return 0;
}
