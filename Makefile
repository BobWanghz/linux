#!/bin/bash
all:server client

server:server.c protocol.c
	gcc -w -o server server.c protocol.c
client:client.c protocol.c
	gcc -w -o client client.c protocol.c

.PHONE:clean
clean:
	rm -f server client *.o *.i *.s
