#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

int main(){

	pid_t pid;
	pid = fork();
	if (pid == 0){
		//Child process
		printf ("I am the child process\n");
		exit(1);
		printf("This is my last instruction\n");	
	}
	else{
		//Parent process
		printf ("I am the parent process\n");
	}
	return 0;
}





