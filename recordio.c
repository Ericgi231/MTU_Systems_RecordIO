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

/* rio_open
 * Open data file and index file. If create is requested, open both 
 * the data file and the index file with O_CREAT. If O_RDONLY or
 * O_RDWR is requested, make sure that the index file is present and 
 * return an error otherwise. On success the data file's descriptor
 * should be returned and the file descriptor for the index file
 * must be maintained within the abstraction.
 */
int rio_open(const char *pathname, int flags, mode_t mode){
	char *temp = malloc(1);
	//file descriptor index of original file
	int fdo = open(pathname, flags, mode);
	if (fdo == -1) { 
		free(temp);
		return fdo; 
	}
	int leno = read(fdo, temp, 1);
	lseek(fdo, 0, SEEK_SET);
	//concatinated name of original file with .rinx. to make name of record file
	char *fn = malloc(strlen(".rinx.") + strlen(pathname));
	strcpy(fn, ".rinx.");
	strcat(fn, pathname);
	//file descriptor index of .rinx. file
	int fdr = open(fn, flags, mode);
	free(fn);
	if (fdr == -1) { 
		free(temp);
		return fdr; 
	}
	int lenr = read(fdr, temp, 1);
	free(temp);
	lseek(fdr, 0, SEEK_SET);
	//check that both files are in same state
	//if one contains data, both must
	if ((leno > 0 && lenr == 0) || (leno == 0 && lenr > 0)) {
		return -1;
	}
	
	return (fdo << 16) | fdr;
}

/* rio_read
 * Allocate a buffer large enough to hold the requested record, read 
 * the record into the buffer and return the pointer to the allocated 
 * area. The I/O result should be returned through the return_value 
 * argument. On success this will be the number of bytes read into
 * the allocated buffer. If any system call returns an error, this
 * should be communicated to the caller through return_value.
 */
void *rio_read(int fd, int *return_value){
	//get original file descriptor index
	int fdo = fd >> 16;
	//get record file descriptor index
	int fdr = fd & 0x0000FFFF;

	//read record, return on error or end of file
	struct record_descriptor rd;
	char buff[sizeof(rd)];
	int res = read(fdr, buff, sizeof(rd));
	if (res == -1 || res == 0) {
		*return_value = res;
		return NULL;
	}

	//fill record descriptor
	rd.position = *((int*)buff);
	rd.length = *((int*)buff+1);

	//set read pointer for original file
	if (lseek(fdo, rd.position, SEEK_SET) == -1) {
		*return_value = -1;
		return NULL;
	}

	//read file
	char *outBuff = malloc(rd.length+1);
	res = read(fdo, outBuff, rd.length);
	if (res == -1) {
		*return_value = res;
		return NULL;
	}

	//read next record, update original position to next location
	int res2 = read(fdr, buff, sizeof(rd));
	if (res2 == -1) {
		return NULL;
	} else if (res2 > 0) {
		rd.position = *((int*)buff);
		lseek(fdo, rd.position, SEEK_SET);
		lseek(fdr, -res2, SEEK_CUR);
	}

	outBuff[rd.length] = '\000';
	*return_value = res;
	return outBuff;
}

/* rio_write
 * Write a new record. If appending to the file, create a record_descriptor 
 * and fill in the values. Write the record_descriptor to the index file and 
 * the supplied data to the data file for the requested length. If updating 
 * an existing record, read the record_descriptor, check to see if the new 
 * record fits in the allocated area and rewrite. Return an error otherwise.
 */
int rio_write(int fd, const void*buf, int count){
	//get original file descriptor index
	int fdo = fd >> 16;
	//get record file descriptor index
	int fdr = fd & 0x0000FFFF;

	//output var
	int outVal = count;

	//read record
	struct record_descriptor rd;
	char buff[sizeof(rd)];
	int res = read(fdr, buff, sizeof(rd));
	//error on reading record
	if (res == -1) {
		return res;
	} 
	//record empty, write to record and file, (append)
	else if (res == 0) {
		rd.position = lseek(fdo, 0, SEEK_CUR);
		rd.length = count;
		//write record file
		if (write(fdr, &rd, sizeof(rd)) == -1) {
			return -1;
		}
		//write original file
		if (write(fdo, buf, count) == -1) {
			return -1;
		}
	}
	//record exists and has data, update if it fits (replace)
	else {
		rd.position = *((int*) buff);
		rd.length = *((int*) buff + 1);

		//check if new record can fit, error if not
		if (count > rd.length) {
			return -1;
		}

		//write new data to original file
		// int test = lseek(fdo, 0, SEEK_CUR);
		//lseek(fdo, rd.position, SEEK_SET);
		outVal = write(fdo, buf, count);
		if (outVal == -1) {
			return -1;
		}

		// if (lseek(fdo, rd.position+rd.length, SEEK_SET) == -1) {
		// 	return -1;
		// }
		
		//update length and update record file
		rd.length = strlen(buf);
		if (lseek(fdr, -res, SEEK_CUR) == -1) {
			return -1;
		}
		if (write(fdr, &rd, sizeof(rd)) == -1) {
			return -1;
		}

		//read next record, update original position to next location
		int res2 = read(fdr, buff, sizeof(rd));
		if (res == -1) {
			return -1;
		} else if (res > 0) {
			rd.position = *((int*)buff);
			lseek(fdo, rd.position, SEEK_SET);
			lseek(fdr, -res2, SEEK_CUR);
		}
		// test = lseek(fdo, 0, SEEK_CUR);
		// test = lseek(fdr, 0, SEEK_CUR);
		// printf(" ");
	}
	
	return outVal;
}

/* rio_lseek
 * Seek both files (data and index files) to the beginning of the requested 
 * record so that the next I/O is performed at the requested position. The
 * offset argument is in terms of records not bytes (relative to whence).
 * whence assumes the same values as lseek whence argument.
 *
 */
int rio_lseek(int fd, int offset, int whence){
	//get original file descriptor index
	int fdo = fd >> 16;
	//get record file descriptor index
	int fdr = fd & 0x0000FFFF;

	//vars
	struct record_descriptor rd;

	//move to new location in record index file
	int start = lseek(fdr, 0, whence);
	int val = offset*sizeof(rd);
	if(lseek(fdr, val+start, whence) == -1){
		return -1;
	}

	//read from record file
	char buff[sizeof(rd)];
	int res = read(fdr, buff, sizeof(rd));
	if (res == -1) {
		return -1;
	} 
	
	//if not end of file
	if (res != 0) {
		//reset record file
		if(lseek(fdr, -res, SEEK_CUR) == -1){
			return -1;
		}

		//get record info and move position of original file
		rd.position = *((int*) buff);
		rd.length = *((int*) buff + 1);

		lseek(fdo, rd.position, SEEK_SET);
	}
	//otherwise, is end of file
	else {
		lseek(fdo, 0, SEEK_END);
	}
	
	int temp = lseek(fdr, 0, SEEK_CUR);
	int outVal = (temp / 8);

	return outVal;
}

/* rio_close
 * Close both files. Even though a single integer is passed as an argument, 
 * your abstraction must close the other file as well. It is suggested 
 * in rio_open that you return the descriptor obtained by opening the data file 
 * to the user and keep the index file descriptor number in the 
 * abstraction and associate them. You may also shift and pack them together 
 * into a single integer.
 */
int rio_close(int fd){
	//close original file, top 16 bits
	int temp = close(fd >> 16);
	if (temp == -1) {
		return temp;
	}
	//close record file, bottom 16 bits
	temp = close(fd & 0x0000FFFF);
	if (temp == -1) {
		return temp;
	}
	return 0;
}
