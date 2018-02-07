#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "myftp.h"

// Transfer function
void createHeader(struct message_s *header, unsigned char type, unsigned int length){
	strcpy(header->protocol, "myftp");
	header->type = type;
	header->length = length;
}

// File Operation
int getFileSize(FILE *file){
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	rewind(file);

	return size;
}

unsigned char **fragmentateFile(FILE *file, unsigned int size){
	int fs = getFileSize(file);
	int dimension = fs / size + (fs >= size && fs % size != 0);
	int i,j;

	unsigned char **list = malloc(sizeof(unsigned char*)*dimension);
	for(i=0;i<dimension;i++){
		list[i] = (unsigned char*)malloc(sizeof(unsigned char)*size);
		for (j = 0; j < size; j++)
		{
			fread(&list[i][j], 1, 1, file);
			printf("%c\n", list[i][j]);
		}
	}

	return list;
}

int main(int argc, char **argv) {
	struct message_s header;
	createHeader(&header,FILE_DATA,16);
	
	printf("%s\n", header.protocol);
	printf("%x\n", header.type);
    
	FILE *fd = fopen("./data/dummy", "r");
    unsigned char **list = fragmentateFile(fd,4);
    fclose(fd);
	// printf("%d\n", sizeof(list));
	exit(0);
}
