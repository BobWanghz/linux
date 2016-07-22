#include<pthread.h>
#include<stdio.h>
#include<errno.h>
#include<semaphore.h>
#include"frame.h"
#include"debug.h"

void *threadsend(void *vargp);
void *threadrecv(void *vargp);
void *thread(void *vargp);
