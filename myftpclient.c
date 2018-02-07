#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>	// "struct sockaddr_in"
#include <arpa/inet.h>	// "in_addr_t"
#include "myftp.h"


void main_task(in_addr_t ip, unsigned short port)
{
	int buf[2];
	int fd,i;
	int choice;
	struct sockaddr_in addr;
	unsigned int addrlen = sizeof(struct sockaddr_in);

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

	FILE *file = fopen("./data/dummy","r");
	int fs = getFileSize(file);
	char *payload = readFileToByte(file);

	struct message_s *header = createHeader(FILE_DATA,HEADER_SIZE + strlen(payload));
	char *header_encoded = encodeHeader(header);

	printf("FS: %d\n", fs);
	send(fd,header_encoded,HEADER_SIZE,0);
	send(fd,payload,fs,0);

	close(fd);	// Time to shut up
}

int main(int argc, char **argv)
{
	in_addr_t ip;
	unsigned short port;

	if(argc != 3)
	{
		fprintf(stderr, "Usage: %s [IP address] [port]\n", argv[0]);
		exit(1);
	}


	if( (ip = inet_addr(argv[1])) == -1 )
	{
		perror("inet_addr()");
		exit(1);
	}

	port = atoi(argv[2]);

	main_task(ip, port);
	return 0;
}
