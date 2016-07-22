#!/bin/bash
all:server client

server:server.c protocol.c mypthread.c
	gcc -w -o server server.c protocol.c mypthread.c -lpthread
client:client.c protocol.c mypthread.c
	gcc -w -o client client.c protocol.c mypthread.c -lpthread

.PHONE:clean
clean:
	rm -f server client *.o *.i *.s
