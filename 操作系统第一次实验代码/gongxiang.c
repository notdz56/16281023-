/*
 * gongxiang.c
 * 
 * Copyright 2019 zhanghua <zhanghua@zhanghua-virtual-machine>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
volatile int counter = 0;
int loops;
void *worker(void *arg){
	int i;
	for (i = 0; i < loops; i++) 
	{
	counter++;
	}
	return NULL;
}
int main(int argc, char *argv[])
{
	if (argc != 2) 
	{
	fprintf(stderr, "usage: threads <value>\n");
	exit(1);
	}
	loops = atoi(argv[1]);
	pthread_t p1, p2;
	printf("Initial value : %d\n", counter);
	pthread_create(&p1, NULL, worker, NULL);
	pthread_create(&p2, NULL, worker, NULL);
	pthread_join(p1, NULL);
	pthread_join(p2, NULL);
	printf("Final value : %d\n", counter);
	return 0;
}
