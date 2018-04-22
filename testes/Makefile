#
# Makefile para os testes
# 

CC=gcc
CFLAGS=-Wall -I../include -L../lib -lcthread
LIB=../lib/libcthread.a

all: teste1 teste2

teste1: teste1.c $(LIB)
	$(CC) -o teste1 teste1.c $(CFLAGS)

teste2: teste2.c $(LIB)
	$(CC) -o teste2 teste2.c $(CFLAGS)

clean:
	rm -rf teste1 teste2 *.o *~




