#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include "myftp.h"

/**
/* Header Function
**/
struct message_s *createHeader(unsigned char type, unsigned int length){
	struct message_s *header = malloc(sizeof(struct message_s));

	strcpy(header->protocol, "myftp");
	header->type = type;
	header->length = length;

	return header;
}

char *encodeHeader(struct message_s *header){
	char *temp = malloc(sizeof(char)*HEADER_SIZE);
	memcpy(temp, header, HEADER_SIZE);

	return temp;
}

struct message_s *decodeHeader(char* message){
	struct message_s *temp = (struct message_s*)message;

	return temp;
}

int hasPayload(unsigned char type){
	return (type == 0xA2 || type == 0xB1 || type == 0xC1 || type == 0xFF);
}

/**
/* Transfer function
**/
char *generatePacket(struct message_s *header, char* payload, int packetSize){
	char *packet = malloc(sizeof(char)*packetSize);
	char *header_buf = encodeHeader(header);

	memset(packet, ' ', HEADER_SIZE);
	strcat(packet,payload);
	strcpy(packet,header_buf);

	return packet;
}

void sendPacket(int fd, struct message_s *header, char *payload, int payload_size){
	char *header_encoded = encodeHeader(header);

	send(fd,header_encoded,HEADER_SIZE,0);
	if(payload_size > 0) send(fd,payload,payload_size,0);
}

// not used
void *explodePacket(char* packet, struct message_s **header_src, char **payload){
	// Explode Header
	char *header_buffer = malloc(sizeof(char)*HEADER_SIZE);
	strcpy(header_buffer,packet);
	*header_src = decodeHeader(header_buffer);

	// Explode payload
	int payloadSize = (*header_src)->length - 10;
	*payload = malloc(sizeof(char)*payloadSize);
	packet +=10;
	strcpy(*payload,packet);
}

/**
// File Operation
**/
int getFileSize(FILE *file){
	fseek(file, 0L, SEEK_END);
	int size = ftell(file);
	rewind(file);

	return size;
}

// Should send whole file at once, maybe use readFileToByte()
int *fragmentateFile(FILE *file, unsigned int size){
	int fs = getFileSize(file);
	int dimension = fs / size + (fs >= size && fs % size != 0);
	int i,j;

	int *list = calloc(sizeof(int)*dimension,sizeof(int));
	for(i=0;i<dimension;i++){
		fread(&list[i], 1, size, file);
		list[i]=ntohl(list[i]);
	}

	return list;
}

char *readFileToByte(FILE *file){
	int fs = getFileSize(file);
	char *buffer = malloc(sizeof(char)*fs);
	fread(buffer, 1, fs, file);

	return buffer;
}

// int checkFileExsist(char *target_dir, char *rqFile) {
// 	DIR *dir;
// 	struct dirent *reader;
// 	int isFind = 0;

// 	if ((dir = opendir(target_dir)) == NULL){
// 		printf("error: directory can not opened.\n");
// 		closedir(dir);
// 		exit(1);
// 	} else {
// 		// printf("The file you want: %s\n", rqFile);
// 		// printf("The following are the result: \n");
// 		while (( reader = readdir(dir)) != NULL){
// 			if(strcmp(reader->d_name,rqFile) == 0){
// 				isFind = 1;
// 				break;
// 			}
// 		}
// 	}
// 	closedir(dir);
// 	return isFind;
// }

// char *listFile(){
// 	DIR *dir;
// 	struct dirent *reader;
// 	int fileNumber = 0;
// 	char *fileName;

// 	if ((dir = opendir("./data/")) == NULL){
// 		printf("error: directory can not opened.\n");
// 	} else {
// 		fileName = calloc(sizeof(char)*1024,1);
// 		while (( reader = readdir(dir)) != NULL){
// 			if((strcmp(reader->d_name,".") == 0) || ((strcmp(reader->d_name,"..") == 0)) ) continue;
// 			strcat(fileName,reader->d_name);
// 			strcat(fileName," \n");
// 			fileNumber++;
// 		}
// 		closedir(dir);

// 		if(fileNumber == 0){
// 			strcpy(fileName,"No file found in the directory.\n");
// 			return fileName;
// 		} else {
// 			char *result = malloc(sizeof(char)*strlen(fileName)+1);
// 			memcpy(result,fileName,strlen(fileName));

// 			free(fileName);
// 			return result;
// 		}
		
// 	}
// }

/**
// Network
**/
void bindAndListen(int fd, struct sockaddr_in *addr){
	if(bind(fd, (struct sockaddr *) addr, sizeof(struct sockaddr_in)) == -1)
	{
		perror("bind()");
		exit(1);
	}

	// Switch to listen mode by invoking listen()

	if( listen(fd, 1024) == -1 )
	{
		perror("listen()");
		exit(1);
	}
}

// void test() {

//   DIR *dp;
//   struct dirent64 *ep;

//   dp = opendir ("./");
//   if (dp != NULL)
//     {
//       while (ep = readdir (dp))
//         printf("%s\n", ep->d_name);
//       (void) closedir (dp);
//     }
//   else
//     perror ("Couldn't open the directory");

// }

// int main(int argc, char **argv) {
// 	test();

// 	exit(0);
// }
