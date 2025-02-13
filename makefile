#makefile for hello.c

CC=gcc
CFLAGS = -g -Wall -lcjson

all: server client 

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm server client 
