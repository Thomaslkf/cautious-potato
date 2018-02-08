CC = gcc
LIB = myftp.c -lpthread

all: server client

server: ${LIB} myftpserver.c
		${CC} -o server myftpserver.c ${LIB}

client:	${LIB} myftpclient.c
		${CC} -o client myftpclient.c ${LIB} 