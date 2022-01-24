#include "recordio.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <unistd.h>

struct record_descriptor
{
	int position; //byte offset relative to the beginning of the data file
	int length;   //length of record in bytes
};

void makeFile1(){
	
	int fd = rio_open("f1.txt", O_CREAT | O_TRUNC | O_RDWR, 0600);
	rio_write(fd, "cat\n", strlen("cat\n"));
	rio_close(fd);
}

void file1UpdateInvalid(){
	
	int fd = rio_open("f1.txt", O_RDWR, 0600);
	rio_lseek(fd, 0, SEEK_SET);
	rio_write(fd, "cat but some more\n", strlen("cat but some more\n"));
	rio_close(fd);
}

void file1AddMore(){
	
	int fd = rio_open("f1.txt", O_RDWR, 0600);
	rio_lseek(fd, 0, SEEK_END);
	rio_write(fd, "cat2\n", strlen("cat2\n"));
	rio_close(fd);
}

void makeFile2(){
	
	int fd = rio_open("f2.txt", O_CREAT | O_TRUNC | O_RDWR, 0600);
	rio_write(fd, "the cat is\n", strlen("the cat is\n"));
	rio_write(fd, "holding his hat\n", strlen("holding his hat\n"));
	rio_write(fd, "just\n", strlen("just\n"));
	rio_write(fd, "like\n", strlen("like\n"));
	rio_write(fd, "a bat\n", strlen("a bat\n"));
	rio_close(fd);
}

void file2UpdateValid(){
	
	int fd = rio_open("f2.txt", O_RDWR, 0600);
	rio_lseek(fd, 2, SEEK_SET);
	rio_write(fd, "and\n", strlen("and\n"));
	rio_write(fd, "also\n", strlen("also\n"));
	rio_close(fd);
}

void file2ReplaceValid(){
	
	int fd = rio_open("f2.txt", O_RDWR, 0600);
	rio_write(fd, "a dog is\n", strlen("a dog is\n"));
	rio_write(fd, "holding a hat\n", strlen("holding a hat\n"));
	rio_write(fd, "and\n", strlen("and\n"));
	rio_write(fd, "may\n", strlen("may\n"));
	rio_close(fd);
}

void makeFileNoIndex(){
	
	int fd = open("f3.txt", O_CREAT | O_TRUNC | O_RDWR, 0600);
	write(fd, "some lines\n", strlen("some lines\n"));
	write(fd, "are in\n", strlen("are in\n"));
	write(fd, "this file\n", strlen("this file\n"));
	close(fd);
}

void makeFileNoMatch(){
	
	int fd = rio_open("f4.txt", O_CREAT | O_TRUNC | O_RDWR, 0600);
	rio_write(fd, "the cat is\n", strlen("the cat is\n"));
	rio_write(fd, "holding his hat\n", strlen("holding his hat\n"));
	rio_close(fd);
	fd = open(".rinx.f4.txt", O_TRUNC, 0600);
	close(fd);
}

int main(int argc, char *argv[]){
	//makeFile1();
	//makeFile2();
	// makeFileNoIndex();
	// makeFileNoMatch();
	//file1UpdateInvalid();
	//file2UpdateValid();
	//file1AddMore();
	//file2ReplaceValid();

	return 0;
}
