/*
 * tree1.c
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
#include <unistd.h>
int main(){
	pid_t p1;
	printf("p1 pid: %d, ppid: %d\n", getpid(),getppid());
	if(p1==0)
	{
		pid_t p3;
		p3 = fork();
		if(p3 == 0)
			printf("p3 pid: %d, ppid: %d\n", getpid(),getppid());
		else if(p3 > 0){
			pid_t p2;
			p2 = fork();
			if(p2==0){
				printf("p2 pid: %d, ppid: %d\n", getpid(),getppid());
				pid_t p4;
				p4 = fork();
				if(p4 == 0){
					printf("p4 pid: %d, ppid: %d\n", getpid(),getppid());					
				}
				else if(p4 >0){
					pid_t p5;
					p5 = fork();
					if(p5==0)
						printf("p5 pid: %d, ppid: %d\n", getpid(),getppid());	
				}
			}
		}
	}
	sleep(1);	
}
