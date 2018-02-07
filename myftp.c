#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	struct message_s *temp = malloc(sizeof(struct message_s));
	temp = (struct message_s*)message;

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

void test() {
	FILE *file = fopen("./data/dummy","r");
	char *payload = readFileToByte(file);

	struct message_s *header = createHeader(FILE_DATA,HEADER_SIZE + strlen(payload));
	char *packet = generatePacket(header, payload, header->length);

	struct message_s *header_decoded;
	char *payload_decoded;
	explodePacket(packet,&header_decoded,&payload_decoded);

	printf("%x\n", header_decoded->type);
	printf("%s\n", payload_decoded);
}

// int main(int argc, char **argv) {
// 	test();

// 	exit(0);
// }
