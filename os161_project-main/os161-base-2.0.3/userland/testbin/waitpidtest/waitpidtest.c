#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

int main(){
	
	pid_t pid;
	int retval;
	int status;
	pid = fork();
	if (pid == 0){
		//Child process
		printf ("I am the child process with PID = %d\n", getpid());
		printf("Waiting 1\n");
		printf("Waiting 2\n");
		printf("Waiting 3\n");
		printf("Waiting 4\n");
		
		exit(1);
	}
	else{
		//Parent process
		retval = waitpid(pid, &status, 0);
		printf("Now I can continue!\n");
		printf("Waiting 5\n");
		printf("Waiting 6\n");
		printf("Status = %d\n", status);
		printf("Retval = %d\n", retval);	
	}
	
	return 0;
}




