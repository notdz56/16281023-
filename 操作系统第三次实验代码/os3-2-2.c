#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int ticketCount = 1000;
int temp;
int pthread_yield(void);
sem_t product;
sem_t consume;

void *sale() {
	for (int i = 0; i < 1000; i++){
		sem_wait(&consume);
		temp = ticketCount;
		pthread_yield();
		temp = temp - 1;
		pthread_yield();
		ticketCount = temp;
		printf("售票: %d 余票: %d \n",i, ticketCount);
		sem_post(&product);
	}
	return NULL;
}
void *preturn(void *arg) {
 
	for (int i = 0; i < 1000; i++){
		
		sem_wait(&product);
		
		temp = ticketCount;
		pthread_yield();
		temp = temp + 1;
		pthread_yield();
		ticketCount = temp;
		printf("退票: %d 余票: %d \n",i, ticketCount);
		sem_post(&consume);
	}
	return NULL;
 
}
int main()
{
	sem_init(&product, 0, 0);
	sem_init(&consume, 0, 1000);
	pthread_t p1, p2;
	pthread_create(&p1, NULL, sale, NULL);
	pthread_create(&p2, NULL, preturn, NULL);
 
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	sem_destroy(&consume);
	sem_destroy(&product);
	return 0;
}
