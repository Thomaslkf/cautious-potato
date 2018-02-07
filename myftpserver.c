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
#include <sys/wait.h>
#include "myftp.h"

void child_function(int accept_fd) {
	int pid = getpid(), count,i;

	char *header_buffer = malloc(sizeof(char)*HEADER_SIZE);
	struct message_s *header_decoded;

	int payload_Size;
	char *payload_decoded;
	
	// Receive Header 
	count = recv(accept_fd,header_buffer,HEADER_SIZE,0);
	if(count != HEADER_SIZE)
	{
		perror("Error during deciphering header...");
		exit(1);
	} 
	header_decoded = decodeHeader(header_buffer);
	//check protocol?

	// Receive Header 
	payload_Size = header_decoded->length - HEADER_SIZE;
	payload_decoded = malloc(sizeof(char)*payload_Size);
	count = recv(accept_fd,payload_decoded,payload_Size,0);
	if(count != payload_Size)
	{
		perror("Error during deciphering payload...");
		exit(1);
	}

	close(accept_fd);	// Time to shut up.
	exit(0);
}


void main_loop(unsigned short port)
{
	int fd, accept_fd, client_count, pid = getpid();
	struct sockaddr_in addr, tmp_addr;
	unsigned int addrlen = sizeof(struct sockaddr_in);

	fd = socket(AF_INET, SOCK_STREAM, 0);		// Create a TCP Socket

	if(fd == -1)
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

	if( listen(fd, 1024) == -1 )
	{
		perror("listen()");
		exit(1);
	}

	printf("[To stop the server: press Ctrl + C]\n");

	client_count = 0;
	while(1) {
		// Accept one client
		if( (accept_fd = accept(fd, (struct sockaddr *) &tmp_addr, &addrlen)) == -1)
		{
			perror("accept()");
			exit(1);
		}

		client_count++;
		printf("[Mother Process:%d] Connection count = %d\n", pid, client_count);

		if(!fork())
			child_function(accept_fd);

		close(accept_fd);	// don't worry, child is still opening the socket.

		// I don't handle zombie. Can you help me?

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

	main_loop(port);

	return 0;
}
