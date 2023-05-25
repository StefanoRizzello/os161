#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

int main(){

	//WRITING INTO A FILE
	int fd;
	int sz;
	char message [60];
	int len;
	fd = open ("/bin/myfilew.txt", O_WRONLY | O_CREAT);
	printf("fd = %d\n", fd);
	strcpy(message, "This is a test write by the write test");
	len = strlen(message);
	sz = write(fd, message, len);
	close(fd);
	printf("size written = %d\n", sz);
		
	return 0;

}

























