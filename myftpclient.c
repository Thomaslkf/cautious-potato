#include <arpa/inet.h>	// "in_addr_t"
#include <netinet/in.h>	// "struct sockaddr_in"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <unistd.h>

#include "myftp.h"
#include "dir.h"
#include "dir_SunOS.h"

void get_request(int fd, char *fileName){
	struct message_s *header = createHeader(GET_REQUEST,HEADER_SIZE + strlen(fileName) + 1);
	sendPacket(fd, header, fileName, strlen(fileName) + 1);
	free(header);

	// GET_REPLY
	char *header_buffer = malloc(sizeof(char)*HEADER_SIZE);
	recv(fd,header_buffer,HEADER_SIZE,0);
	header = decodeHeader(header_buffer);

	if(header->type == GET_REPLY_EXIST){
		// Receive Header 
		int count = recv(fd,header_buffer,HEADER_SIZE,0);
		if(count != HEADER_SIZE)
		{
			perror("Error during deciphering header...");
			exit(1);
		} 
		header = decodeHeader(header_buffer);

		// Receive payload
		int payload_Size = header->length - HEADER_SIZE;
		char *payload = malloc(sizeof(char)*payload_Size);
		count = recvn(fd,payload,payload_Size);
		if(count != payload_Size)
		{
			perror("Error during deciphering payload...");
			exit(1);
		}

		// Store file
		char *path = malloc(sizeof(char)*(strlen(fileName)+DATA_DIR_OFFSET));
		strcat(path, "./");
		strcat(path, fileName);


		FILE *file = fopen(path,"w");
		if(file == NULL) printf("Error Storing %s\n", fileName);
		else printf("GET %s in %s\n", fileName, path);
		fwrite(payload,sizeof(char),strlen(payload),file);
		fclose(file);
	} else {
		printf("Cannot find %s in server\n", fileName);
	}
}

void list_request(int fd){
	// LIST_REQUEST
	struct message_s *header = createHeader(LIST_REQUEST,HEADER_SIZE);
	sendPacket(fd, header, " ", 0);
	free(header);

	// LIST_REPLY
	char *header_buffer = malloc(sizeof(char)*HEADER_SIZE);
	recv(fd,header_buffer,HEADER_SIZE,0);
	header = decodeHeader(header_buffer);

	int payload_Size = header->length - HEADER_SIZE;
	char *payload = malloc(sizeof(char)*payload_Size);
	int count = recv(fd,payload,payload_Size,0);
	if(count != payload_Size)
	{
		perror("Error during deciphering payload...");
		exit(1);
	}

	printf("%s", payload);
}

void put_sendFileName(int fd, char *fileName){
	// PUT_REQUEST
	struct message_s *header = createHeader(PUT_REQUEST,HEADER_SIZE + strlen(fileName) + 1);
	sendPacket(fd, header, fileName, strlen(fileName) + 1);
	free(header);

	// PUT_REPLY
	char *header_buffer = malloc(sizeof(char)*HEADER_SIZE);
	recv(fd,header_buffer,HEADER_SIZE,0);
	header = decodeHeader(header_buffer);
}

void put_sendFile(int fd, char *fileName){
	// FILE_DATA
	char *path = calloc(sizeof(char)*(strlen(fileName) + LOCAL_DIR_OFFSET), 1);
	strcat(path,"./");
	strcat(path,fileName);

	FILE *file = fopen(path,"r");
	int fs = getFileSize(file);
	char *payload = readFileToByte(file);
	fclose(file);

	struct message_s *header = createHeader(FILE_DATA,HEADER_SIZE + fs);
	printf("Sending %s to server...\n", fileName);
	sendPacketWithFile(fd, header, payload, fs);
	printf("Done\n");

	free(payload);
	free(header);
	free(path);
}

void main_task(int cmd, in_addr_t ip, unsigned short port, char *src)
{
	int fd;
	int choice;
	struct sockaddr_in addr;
	unsigned int addrlen = sizeof(struct sockaddr_in);
	FILE *file;

	fd = socket(AF_INET, SOCK_STREAM, 0);	// Create a TCP socket

	if(fd == -1)
	{
		perror("socket()");
		exit(1);
	}

	// Below 4 lines: Set up the destination with IP address and port number.

	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip;
	addr.sin_port = htons(port);

	if( connect(fd, (struct sockaddr *) &addr, addrlen) == -1 )		// connect to the destintation
	{
		perror("connect()");
		exit(1);
	}

	// CMD switch
	switch(cmd) {
		case 1:	
			list_request(fd);
			break;
		case 2:	
			get_request(fd,src);
			break;
		case 3:	
			if(!checkFileExsist("./",src)){
				printf("File Not Found\n");
				close(fd);
				exit(1);
			}
			put_sendFileName(fd,src);
			put_sendFile(fd,src);
			break;
	}
	close(fd);	// Time to shut up
}

int main(int argc, char **argv)
{
	in_addr_t ip;
	unsigned short port;
	int cmd = 0;

	if(argc < 4)
	{
		fprintf(stderr, "Usage: %s [IP address] [port] [list|get|put] [file]\n", argv[0]);
		exit(1);
	}

	if( (ip = inet_addr(argv[1])) == -1 )
	{
		perror("inet_addr()");
		exit(1);
	}
	port = atoi(argv[2]);
	
	// Detect OS
	struct utsname unameData;
  	uname(&unameData);
  	is_SunOS = strcmp(unameData.sysname, "SunOS") == 0;

	// CMD switch
	if(!strcmp(argv[3],"list")) cmd = 1;
	if(!strcmp(argv[3],"get")) cmd = 2;
	if(!strcmp(argv[3],"put")) cmd = 3;
	if(!cmd) {
		printf("Invalid CMD\n");
		exit(1);
	}

	main_task(cmd, ip, port , argc == 5 ? argv[4] : "");
	return 0;
}
