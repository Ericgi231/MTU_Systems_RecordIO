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

/* You must use this struct when constructing records. It is
 * expected that all binary data in the index file will follow
 * this format. Do not modify the struct.
 */
struct record_descriptor
{
	int position; //byte offset relative to the beginning of the data file
	int length;   //length of record in bytes
};

/* main - indexer
 * Develop a single standalone program called indexer which creates an 
 * index file for a text file whose title is supplied as an argument to 
 * the program. This program should read the text file from beginning 
 * till end, find the beginning and ending of each line (ending with 
 * the newline character), create a descriptor and write the descriptor 
 * to the created index file. The program should not modify the text 
 * file which is supplied as an argument.
 * 
 * "make indexer" will compile this source into indexer
 * To run your program: ./indexer <some_text_file>
 */
int main(int argc, char *argv[]){
	/* The index file for record i/o is named by following the 
	 * convention .rinx.<data file name> (i.e., if the data file
	 * is named myfile.txt, its index will be .rinx.myfile.txt). 
	 * This convention conveniently hides the index files while
	 * permitting easy access to them based on the data file name. 
	 */

	//exit on too few arguments
	if(argc <= 1){
		printf("Please provide file name\n");
		exit(0);
	}

	//open given file, error if no file exists
	int fd = open(argv[1], O_RDONLY, 0666);
	if(fd == -1){
		printf("Data file open fail\n");
		exit(0);
	}

	//read file and close it
	int end = lseek(fd, 0, SEEK_END);
	lseek(fd, 0, SEEK_SET);
	char *data = malloc(end+1);
	int len;
	len = read(fd, data, end);
	close(fd);

	//open new record index file
	char *fn = malloc(strlen(".rinx.") + strlen(argv[1]));
	strcpy(fn, ".rinx.");
	strcat(fn, argv[1]);
	fd = open(fn, O_CREAT | O_TRUNC | O_RDWR, 0666);
	free(fn);
	if(fd == -1){
		printf("Record index file creation fail\n");
		exit(0);
	}

	//write to record index file
	struct record_descriptor rd;
	char *tok = strtok(data, "\n");
	int pointer = 0;
	while (tok != NULL) {
		len = strlen(tok)+1;
		rd.position = pointer;
		rd.length = len;
		//write record
		if (write(fd, &rd, sizeof(rd)) == -1) {
			return -1;
		}
		pointer += len;
		tok = strtok(NULL, "\n");
	}
	free(data);

	return 0;
}
