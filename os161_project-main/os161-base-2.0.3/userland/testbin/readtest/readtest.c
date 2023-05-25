#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

int main(){


	//READING FROM A FILE
	int fd;
	int sz;
	char message [60];
	fd = open ("/bin/myfilew.txt", O_RDONLY | O_CREAT);
	printf("fd = %d\n", fd);
	sz = read(fd, message, 60);
	printf("Message read = %s\n", message);
	close(fd);
	printf("size read = %d\n", sz);
	
	return 0;

}








