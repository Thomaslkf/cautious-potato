CC = gcc
LIB = 

all: server client

server: myftpserver.c
		${CC} -o server myftpserver.c ${LIB}

client:	myftpclient.c
		${CC} -o client myftpclient.c ${LIB} 