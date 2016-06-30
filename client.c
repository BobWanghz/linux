#include<stdlib.h>
#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h> //for struct hostent


#include"frame.h"

#define PORT 12345

#define MAXDATASIZE 100

int main(int argc,char *argv[])
{
	int sockfd,num;
	char buf[MAXDATASIZE];
	struct hostent *he; //struct will get information of remote host
	struct sockaddr_in server;
	if(argc != 2){
		printf("Usage:%s <IP Address>\n",argv[0]);
		exit(1);
	}
	if((he = gethostbyname(argv[1])) == NULL){
		perror("gethostbyname() error");
		exit(1);
	}
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) == -1){
		perror("socket() error");
		exit(1);
	}
	bzero(&server,sizeof(server));
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr = *(struct in_addr *)he->h_addr;
	if(connect(sockfd,(struct sockaddr *)&server,sizeof(server)) == -1){
		perror("connect() error");
		exit(1);
	}
	char str[] = "My name is WHZ\n";
	if((num = send_frame(sockfd,str,strlen(str),0)) == -1){
		perror("send() error");
		exit(1);
	}
	if((num = recv(sockfd,buf,MAXDATASIZE,0)) == -1){
		perror("recv() error");
		exit(1);
	}
	//buf[num-1] ='\0';
	struct gateway_frame frame;
	int buf_len = strlen(buf);
	get_frame(&frame,buf,buf_len);
	print_frame(&frame);
	//printf("server message:%s\n",buf);
	//puts(buf);
	close(sockfd);
	return 0;
}
