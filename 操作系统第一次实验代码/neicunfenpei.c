/*
 * neicunfenpei.c
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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
int main(int argc, char *argv[])
{
	int *p = malloc(sizeof(int)); // a1
	assert(p != NULL);
	printf("(%d) address pointed to by p: %p\n",
	getpid(), p); // a2
	*p = 0; // a3
	while (1) 
	{
		sleep(1);
		*p = *p + 1;
		printf("(%d) p: %d\n", getpid(), *p); // a4
	}
	return 0;
}
