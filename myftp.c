#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
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

void sendPacketWithFile(int fd, struct message_s *header, char *payload, int payload_size){
	char *header_encoded = encodeHeader(header);
	send(fd,header_encoded,HEADER_SIZE,0);
	sendn(fd,payload,payload_size);
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

int sendn(int sd, void *buf, int buf_len) {
	int n_left = buf_len;
	int n;
	while (n_left >0) {
		if((n = send(sd, buf + (buf_len - n_left), n_left, 0)) < 0) {
			if(errno == EINTR)
				n = 0;
			else
				return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

int recvn(int sd, void *buf, int buf_len) {
	int n_left = buf_len;
	int n;
	while (n_left >0) {
		if((n = recv(sd, buf + (buf_len - n_left), n_left, 0)) < 0) {
			if(errno == EINTR)
				n = 0;
			else
				return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
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

// 	if ((dir = opendir("./data/"))
