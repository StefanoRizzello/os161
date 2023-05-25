#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>


int main(){
	
	pid_t pid;
	for (int i = 0; i < 5; i++){
		pid = fork();
		printf ("%d\n", getpid());
		if (pid == 0){

			exit(1);
		}
	}
	return 0;
}

