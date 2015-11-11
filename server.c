#include "cs537.h"
#include "request.h"
#include <pthread.h>


int *bufferconnd;
int count, tail, head, buffers_nums,threads_nums,port,connfd;
pthread_mutex_t mutex;
pthread_cond_t fill;
pthread_cond_t empty;

// CS537: Parse the new arguments too
void getargs(int *port,int *threadnums, int *buffernums, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <port> <threadnums> <buffernums> \n", argv[0]);
	exit(1);
    }
    *threadnums = atoi(argv[2]);
	*buffernums = atoi(argv[3]);
    *port = atoi(argv[1]);
}


void produce(int connfd){
	bufferconnd[tail] = connfd;
	tail = (tail + 1) % buffers_nums;
	count = count + 1;
}

int consume(){
	printf("consume\n");
	//int tmp = sharedBuffer.connfd[sharedBuffer.head];	
	int tmp = bufferconnd[head];
	printf("consume connid tmp is %d \n",tmp);
	head = (head + 1) % buffers_nums;
	count = count - 1;
	return tmp;
}

void *consumer(void *arg){
	printf("consumer\n");
	while(1){
	//	printf("consumer\n");
		pthread_mutex_lock(&mutex);
		while(count == 0)
			pthread_cond_wait(&fill ,&mutex);
		connfd = consume();
		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
		requestHandle(connfd);
		Close(connfd);
	}
}





int main(int argc, char *argv[])
{
    
    struct sockaddr_in clientaddr;
    int i;
    int listenfd, clientlen;

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&fill,NULL);
    pthread_cond_init(&empty,NULL);
    
    getargs(&port,&threads_nums, &buffers_nums, argc, argv);
 
    bufferconnd = (int *)malloc(sizeof(int)*buffers_nums);
    
    tail = 0;
    head = 0;
	count = 0;
    pthread_t *threads;
    threads = (pthread_t *)malloc(sizeof(pthread_t)*threads_nums);
    for(i = 0; i < threads_nums;i++){
    	pthread_create(threads+i,NULL,consumer,NULL);
    }

	listenfd = Open_listenfd(port);
	while(1){
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);
		pthread_mutex_lock(&mutex);
		while(count == buffers_nums){
			pthread_cond_wait(&empty, &mutex);
		}
		produce(connfd);
		pthread_cond_signal(&fill);
		pthread_mutex_unlock(&mutex);
	}
}
