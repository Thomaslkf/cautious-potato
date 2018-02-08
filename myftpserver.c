#include <arpa/inet.h>	// "in_addr_t"
#include <netinet/in.h>	// "struct sockaddr_in"
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

#include "myftp.h"
#include "dir.h"
#include "dir_SunOS.h"

struct thread_data thread_ava[MAX_LISTEN];

void get_reply(int fd, char *fileName){
	// GET_REQUEST
	printf("Client asked for %s...", fileName);
	if(checkFileExsist("./data/", fileName)) {
		printf("Found\n");
		struct message_s *header = createHeader(GET_REPLY_EXIST,HEADER_SIZE + strlen(fileName));
		sendPacket(fd, header," ", 0);
		free(header);

		// FILE_DATA
		printf("Sending File to Client...\n");
		char *path = calloc(sizeof(char)*(strlen(fileName) + DATA_DIR_OFFSET),1);
		path[0] = '\0';
		strcat(path,"./data/");
		strcat(path,fileName);
		// printf("PATH: %s", path);

		FILE *file = fopen(path,"r");
		int fs = getFileSize(file);

		char *payload = readFileToByte(file);
		fclose(file);

		header = createHeader(FILE_DATA,HEADER_SIZE + fs);
		sendPacketWithFile(fd, header, payload, fs);
		free(header);
		free(path);

	} else {
		printf("Not Found\n");
		struct message_s *header = createHeader(GET_REPLY_NON_EXIST,HEADER_SIZE + strlen(fileName));
		sendPacket(fd, header," ", 0);
		free(header);
	}
}

void list_reply(int fd, char *list){
	// LIST_REQUEST
	struct message_s *header = createHeader(LIST_REPLY,HEADER_SIZE + strlen(list) + 1);
	printf("Replying Directory to Client...\n");
	sendPacket(fd, header, list, strlen(list)+1);
	free(header);
}

void put_storeFile(int fd, char *fileName){
	struct message_s *header;
	char *header_buffer = malloc(sizeof(char)*HEADER_SIZE);

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
	// printf("File Size: %d\n", payload_Size);
	char *payload = malloc(sizeof(char)*payload_Size);
	// printf("located\n");
	count = recvn(fd,payload,payload_Size);
	// printf("Recieved: %d\n", count);
	if(count != payload_Size)
	{
		perror("Error during deciphering payload...");
		exit(1);
	}

	// Store file
	// printf("FILE NAME B4 %s\n", fileName);
	char *path = calloc(sizeof(char)*(strlen(fileName) + DATA_DIR_OFFSET) + 1,1);
	strcat(path, "./data/");
	strcat(path, fileName);

	FILE *file = fopen(path,"w");
	if(file == NULL) printf("Error Storing %s\n", fileName);
	else printf("PUT %s in %s\n", fileName, path);
	fwrite(payload,sizeof(char),strlen(payload),file);

	free(header_buffer);
	free(path);
	free(payload);
	fclose(file);
	// printf("%s\n", payload);
}

void put_reply(int fd){
	struct message_s *header = createHeader(PUT_REPLY,HEADER_SIZE);
	sendPacket(fd, header, " ", 0);
}

void *child_function(void *args) {
	int thread_num = *(int *)args;
	int count;
	int accept_fd = thread_ava[thread_num].fd;

	char *header_buffer = malloc(sizeof(char)*HEADER_SIZE);
	struct message_s *header;

	int payload_Size;
	char *payload;
	
	// Receive Header 
	count = recv(accept_fd,header_buffer,HEADER_SIZE,0);
	if(count == 0){
		printf("Client Disconnected\n");
		thread_ava[thread_num].in_use = 0;
  		pthread_exit(NULL);
	}
	if(count != HEADER_SIZE)
	{
		perror("Error during deciphering header...");
		thread_ava[thread_num].in_use = 0;
  		pthread_exit(NULL);
	} 
	header = decodeHeader(header_buffer);
	//check protocol?

	// Receive payload
	if(hasPayload(header->type)){
		payload_Size = header->length - HEADER_SIZE;
		// printf("payload_Size %d\n",payload_Size);
		payload = malloc(sizeof(char)*payload_Size);
		// payload[sizeof(char)*payload_Size] = '\0';
		count = recv(accept_fd,payload,payload_Size,0);
		if(count != payload_Size)
		{
			perror("Error during deciphering payload...");
			thread_ava[thread_num].in_use = 0;
  			pthread_exit(NULL);
		}
		// printf("PAYLOAD %s\n", payload);
	}

	char *list;
	switch(header->type) {
		case PUT_REQUEST :	
			put_reply(accept_fd);
			put_storeFile(accept_fd, payload);
			break;
		case GET_REQUEST :	
			get_reply(accept_fd, payload);
			// get_storeFile(accept_fd, payload);
			break;
		case LIST_REQUEST :	
			list = listFile();
			list_reply(accept_fd,list);
			break;
	}

	close(accept_fd);	// Time to shut up.
	free(header);
	// free(payload);
	thread_ava[thread_num].in_use = 0;
  	pthread_exit(NULL);
}


void main_loop(unsigned short port)
{
	int fd, accept_fd, client_count, i;
	struct sockaddr_in addr, tmp_addr;
	unsigned int addrlen = sizeof(struct sockaddr_in);

	fd = socket(AF_INET, SOCK_STREAM, 0);		// Create a TCP Socket
	long val = 1;
	val = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(long));
	if(fd == -1 || val == -1)
	{
		perror("socket()");
		exit(1);
	}

	// 4 lines below: setting up the port for the listening socket
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(port);

	// After the setup has been done, invoke bind()
	if(bind(fd, (struct sockaddr *) &addr, sizeof(addr)) == -1)
	{
		perror("bind()");
		exit(1);
	}
	// Switch to listen mode by invoking listen()

	if( listen(fd, MAX_LISTEN) == -1 )
	{
		perror("listen()");
		exit(1);
	}

	printf("[To stop the server: press Ctrl + C]\n");

	client_count = 0;
  	pthread_t thread[MAX_LISTEN];
	pthread_mutex_t mutex;
  	pthread_mutex_init(&mutex, NULL);

  	for(i=0;i<MAX_LISTEN;i++){
  		thread_ava[i].thread_id = i;
  		thread_ava[i].fd = 0;
  		thread_ava[i].in_use = 0;
  	}

	while(1) {
		// Accept one client
		if( (accept_fd = accept(fd, (struct sockaddr *) &tmp_addr, &addrlen)) == -1)
		{
			perror("accept()");
			exit(1);
		}
		i = (i + 1) % MAX_LISTEN;
		client_count++;
		printf("[Mother Process] Connection count = %d\n", client_count);

    	pthread_mutex_lock(&mutex);
		while(thread_ava[i].in_use)	i = (i + 1) % 1024;
		thread_ava[i].in_use = 1;		
		thread_ava[i].fd = accept_fd;	
		pthread_create(&thread[i], NULL, child_function, &(thread_ava[i].thread_id));
		// if(!fork())
		// 	child_function(accept_fd);
    	pthread_mutex_unlock(&mutex);

		// close(accept_fd);	// don't worry, child is still opening the socket.
		// NOT FOR pthread mate

	}	// End of infinite, accepting loop.
}

int main(int argc, char **argv)
{
	unsigned short port;

	if(argc != 2)
	{
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);

	// Detect OS
	struct utsname unameData;
  	uname(&unameData);
  	is_SunOS = strcmp(unameData.sysname, "SunOS") == 0;
  	// printf("OS %d\n", is_SunOS);

	main_loop(port);

	return 0;
}
