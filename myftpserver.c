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

void child_function(int accept_fd) {
	int pid = getpid(), count, buf;

	// Read from network, buf is 4 bytes.
	// Since the opposite side sends 4 bytes of data,
	// "count" should report 4 bytes.
	count = read(accept_fd, &buf, sizeof(buf));

	if(count == -1)
	{
		perror("reading...");
		exit(1);
	}

	if(count == 0)		// nothing can be received.
		printf("[PID:%d] Nothing\n", pid);
	else {
		// ntoh() is important. Will explain it in lectures and tutorials
		buf = ntohl(buf);

		// Print the 4 bytes of received data in Hex
		printf("[PID:%d] result = %x\n", pid, buf);
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
