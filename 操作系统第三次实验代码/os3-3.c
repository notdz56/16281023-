#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <unistd.h>

char buf[10]={0};
sem_t empty,full;

void *producer()
{
	int i=0;
	while(1){
		sem_wait(&empty);
		scanf("%c",&buf[i]);
		getchar();
		i=(i+1)%10;
		sem_post(&full);
	}
	return NULL;
}

void *consumer() 
{
	int i=0;
	while(1){
		sem_wait(&full);
		printf("%c\n",buf[i]);
		sleep(3);
		i=(i+1)%10;
		sem_post(&full);
	}
	return NULL;
}

int main(int argc, char *argv[])
{

	pthread_t p1, p2;
	sem_init(&empty, 0, 10);
	sem_init(&full, 0, 0);
	pthread_create(&p1, NULL, producer, NULL);
	pthread_create(&p2, NULL, consumer, NULL);

	pthread_join(p1, NULL);
	pthread_join(p2, NULL);

	sem_destroy(&empty);
	sem_destroy(&full);
	return 0;
}
