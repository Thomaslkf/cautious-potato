CC = gcc
LIB = myftp.c -lpthread

all: 
	@if [ "`uname -s`" = "Linux" ] ; then \
		make -f Makefile.linux ; \
	else \
		make -f Makefile.solaris ; \
	fi

clean: 
	@if [ "`uname -s`" = "Linux" ] ; then \
		make clean -f Makefile.linux ; \
	else \
		make clean -f Makefile.solaris ; \
	fi
