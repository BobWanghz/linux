#include<sys/time.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include"frame.h"
#include"mypthread.h"
#define PORT 12345

#define BACKLOG 10
#define MAXRECVLEN 1024

int main(int argc,char *argv[])
{
	char buf[MAXRECVLEN];
	int listenfd,connectfd;
	struct sockaddr_in server; //server address information
	struct sockaddr_in client; //client address information
	socklen_t addrlen;

	/* Create TCP socket*/
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("socket() error");
		exit(1);
	}
	
	/*Set socket option*/
	int opt = SO_REUSEADDR;
	setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	//inet_pton(AF_INET,INADDR_ANY,&server.sin_addr); //just use inet_pton() 

	/*bind to address*/
	if(bind(listenfd,(struct sockaddr *)&server,sizeof(server)) == -1){
		perror("bind() error");
		exit(1);
	}

	if(listen(listenfd,BACKLOG) == -1){
		perror("listen() error");
		exit(1);
	}
	addrlen = sizeof(client);
	//pthread_t tid1,tid2;
	/*
	while(1){
		if((connectfd = accept(listenfd,(struct sockaddr *)&client,&addrlen)) == -1){
			perror("accept() errror");
			exit(1);
		}
		struct timeval tv;
		gettimeofday(&tv,NULL);
		printf("You got a connection from client's ip %s,port %d\n",\
				inet_ntoa(client.sin_addr),htons(client.sin_port));
		int iret = -1;
		while(1){
			iret = recv(connectfd,buf,MAXRECVLEN,0);
			if(iret > 0){
				printf("%s\n",buf);
			}else{
				close(connectfd);
				break;
			}
			send(connectfd,buf,iret,0); //send message to clientd
		}
	}
	close(listenfd); //close listenfd
	*/
	while(1){
		connectfd = accept(listen,(struct sockaddr*)&client,&addrlen);
		pthread_t tid;
		pthread_create(&tid,NULL,thread,&connectfd);
		//pthread_create(&tid1,NULL,threadsend,&connectfd);
		//pthread_create(&tid2,NULL,threadrecv,&connectfd);
	}
	return EXIT_SUCCESS;
}
