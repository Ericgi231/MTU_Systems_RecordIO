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

/* Develop a test program called testio which includes recordio.h and 
 * is linked against recordio.o. This program should expect a single 
 * argument which supplies a file name. The program should rio_open 
 * the supplied argument and report any errors to the user. Upon a 
 * successful open it should execute a series of rio_read statements, 
 * read the file one record at a time and write each record to the 
 * standard output as shown below.
 *
 * Data File                          Index file
 * ---------------------------------------------------
 *  Systems                            0, 8
 *  programming is cool.               8, 20
 *
 * "make testio" will compile this souce into testio
 * To execute your program: ./testio <some_record_file_name>
 */

int main(int argc, char *argv[]){
	//exit on too few arguments
	if(argc <= 1){
		printf("Please provide file name\n");
		exit(0);
	}

	//open given file, error if no file exists
	int fd = rio_open(argv[1], O_RDWR, 0666);
	if(fd == -1){
		printf("Open Fail \n");
		exit(0);
	}

	printf("Data File                          Index file\n");
	printf("---------------------------------------------------\n");
	char* res;
	int pointer = 0;
	int len;
	while(1){
		//read each record
		res = rio_read(fd, &len);
		if(len <= 0){
			break;
		}
		else {
			//strip \n from record
			res[strcspn(res, "\n")] = 0;
			printf("%-35s %d, %d\n", res, pointer, len);
			pointer += len;
		}
	}
	rio_close(fd);
	
	return 0;
}
