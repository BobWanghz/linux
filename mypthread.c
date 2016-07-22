#include "mypthread.h"
struct buffer rx_buf;

pthread_mutex_t rx_buf_mutex = PTHREAD_MUTEX_INITIALIZER;
sem_t write_buf_count;
sem_t read_buf_count;
void *thread(void *vargp)
{
	pthread_t tid_send,tid_recv;
	sem_init(&write_buf_count,0,BUFFER_COUNT);
	sem_init(&read_buf_count,0,0);
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
		fgets(temp,MAX_DATA_LEN,stdin);
		set_frame(temp,strlen(temp));
		send_frame(connfd,0);
		DEBUG("Send ok!\n");
	}
	return NULL;
}

int get_data()
{
	/*
	if(rx_buf.first == rx_buf.last){
		DEBUG("Receive buffer is empty!\n");
		return -1;
	}*/
	struct frame f;
	get_frame(&f,rx_buf.data[rx_buf.last]);
	return 0;
}
void *threadrecv(void *vargp)
{
	int buflen;
	int idata = 0;
	
	int connfd = *(int *)vargp;
	while(1){
		/*
		if((rx_buf.last + 1)%BUFFER_COUNT == rx_buf.first){
			DEBUG("Receive buffer is full!\n");
			continue;
		}*/
		sem_wait(&buf_count);
		pthread_mutex_lock(&rx_buf_mutex);
		idata = recv(connfd,rx_buf.data[rx_buf.last],MAX_DATA_LEN,0);
		if(idata > 0){
			rx_buf.last = (rx_buf.last + 1)%BUFFER_COUNT;
			printf("Recv ok\n");
			pthread_mutex_unlock(&rx_buf_mutex);
			sem_post(&read_buf_count);
		}else{
			//printf("Recv error\n");
			pthread_mutex_unlock(&rx_buf_mutex);
			sem_post(&write_buf_count);
		}
		get_data();
	}
}
