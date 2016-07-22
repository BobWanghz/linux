#include<pthread.h>
#include"frame.h"

struct buffer rx_buf;
void *threadsend(void *vargp);
void *threadrecv(void *vargp);
void *thread(void *vargp);
void *thread(void *vargp)
{
	pthread_t tid_send,tid_recv;
	int connfd = *(int *)vargp;
	pthread_create(&tid_send,NULL,threadsend,vargp);
	pthread_create(&tid_recv,NULL,threadrecv,vargp);
	return NULL;
}

void *threadsend(void *vargp)
{ 
	int connfd = *(int *)vargp;
	int idata;
	u8 temp[MAX_DATA_LEN];
 	while(1){
		printf("Input the message to send:");
		fget(temp,MAX_DATA_LEN,stdin);
		set_frame(temp,strlen(temp));
		send_frame(connfd,0);
		DEBUG("Send ok!\n");
	}
	return NULL;
}

int get_data()
{
	if(rx_buf.first == rx_buf.last){
		DEBUG("Receive buffer is empty!\n");
		return -1;
	}
	struct frame_head f;
	get_frame(&f,rx_buf[last]);
	return 0;
}
void *threadrecv(void *vargp)
{
	int buflen;
	int idata = 0;
	
	int connfd = *(int *)vargp;
	while(1){
		if((rx_buf.last + 1)%BUFFER_COUNT == rx_buf.first){
			DEBUG("Receive buffer is full!\n");
			continue;
		}
		idata = recv(connfd,rx_buf[last],MAX_DATA_LEN,0);
		if(idata == -1){
			DEBUG("recv error,errno=%d \n",errno);
		}else if(idata > 0){
			rx_buf.last = (rx_buf.last + 1)%BUFFER_COUNT;
		}
		get_data();
	}
}
