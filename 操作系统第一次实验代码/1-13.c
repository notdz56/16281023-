/*
 * 1-13.c
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
int main(int argc, char **argv)
{
    char* msg = "Hello World";
	int len = 11;
	int result = 0;
 
    __asm__ __volatile__("movl %2, %%edx;\n\r" /*传入参数：要显示的字符串长度*/
             "movl %1, %%ecx;\n\r" /*传入参赛：文件描述符（stdout）*/
			 "movl $1, %%ebx;\n\r" /*传入参数：要显示的字符串*/
			 "movl $4, %%eax;\n\r" /*系统调用号：4 sys_write*/
			 "int  $0x80" /*触发系统调用中断*/
             :"=m"(result) /*输出部分：本例并未使用*/
             :"m"(msg),"r"(len)  /*输入部分：绑定字符串和字符串长度变量*/
             :"%eax"); 
    	
	return 0;
}

