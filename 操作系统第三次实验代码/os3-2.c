#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

int ticketCount = 1000;
int temp;
int pthread_yield(void);

void *sale(void *arg) {
	for (int i = 0; i < 1000; i++){
		temp = ticketCount;
		pthread_yield();
		temp = temp - 1;
		pthread_yield();
		ticketCount = temp;
		printf("售票: %d 余票: %d \n",i, ticketCount);
	}
	return NULL;
}
void *preturn(void *arg) {
 
	for (int i = 0; i < 1000; i++){
		temp = ticketCount;
		pthread_yield();
		temp = temp + 1;
		pthread_yield();
		ticketCount = temp;
		printf("退票: %d 余票: %d \n",i, ticketCount);
	}
	return NULL;
 
}
int main()
{
 
	pthread_t p1, p2;
	pthread_create(&p1, NULL, sale, NULL);
	pthread_create(&p2, NULL, preturn, NULL);
 
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
 
	return 0;
}
