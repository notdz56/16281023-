#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <iostream.h>
using namespace std;
#define B		10			//存储块长度
#define L		500			//存储块总数
#define K		100			//保留区大小
#define BUSY	1
#define FREE	0
#define OK		1
#define ERROR	0
#define FILE_BLOCK_LENGTH		(B-3)				//文件分配磁盘块号数组长度
#define FILE_NAME_LENGTH		(B-1)				//最长文件名长度
#define FILE_SIGN_AREA			((L-1-K)/B+1)		//保留区中文件标识符起始块号位图之后
#define FILE_NUM				FILE_BLOCK_LENGTH	//目录内最多文件数目
#define BUFFER_LENGTH			25					//打开文件表目中的缓冲区长度
#define INPUT_LENGTH			100					//写文件时最大输入长度
#define OUTPUT_LENGTH			100					//读文件时最大读出长度



struct filesign {							//文件描述符
	int file_length;						//文件长度
	int filesign_flag;						//占用标识位
	int file_block;							//文件分配磁盘块号数组实际长度
	int file_block_ary[FILE_BLOCK_LENGTH];	//文件分配磁盘块号数组
};

struct contents {							//目录项
	char filename[FILE_NAME_LENGTH];		//文件名
	int	 filesignnum;						//文件描述符序号
};

struct openfilelist {						//打开文件表表目
	char buffer[BUFFER_LENGTH];				//读写缓冲区
	int pointer[2];							//读写指针文件内容的位置
	int filesignnum;						//文件描述符
	int flag;								//占用符
};

char ldisk[L][B];						//用字符数组模拟磁盘

openfilelist open_list[FILE_NUM];		//打开文件表

#define DIR		0
#define	CREATE	1
#define	DELETE	2
#define	OPEN	3
#define	CLOSE	4
#define	READ	5	
#define	WRITE	6
#define HELP	7
#define LSEEK	8
#define EXIT	9
#define	OPLIST	10
#define LDISK	11
#define CH_LENGTH	20


										//辅助函数
int show_openlist();				//显示打开文件表,返回打开文件个数
void directory();					//显示目录文件详细信息
void show_help();					//该文件系统的帮助
void show_ldisk();					//显示磁盘内容（辅助用）
									//核心函数
void read_block(int, char *);		//文件系统与IO设备的接口函数，读取块
void write_block(int, char *);		//文件系统与IO设备的接口函数，写入块
void Init();						//初始化文件系统
int create(char *);					//创建文件
int destroy(char *);				//删除文件
int open(char *);					//打开文件
int close(int);						//关闭文件
int read(int, int, int);				//读文件
int write(int, int, int);				//写文件
int write_buffer(int, int);			//把缓冲区内容写入文件
int lseek(int, int);					//定位文件指针
void Init_block(char, int);			//初始化字符数组块（辅助）

void read_block(int i, char *p)
/**************************读磁盘块
该函数把逻辑块i的内容读入到指针p指向的内存位置
拷贝的字符个数为存储块的长度B。
***************************/
{
	char * temp = (char *)malloc(sizeof(char));
	temp = p;
	for (int a = 0; a < B;)
	{
		*temp = ldisk[i][a];
		a++;
		temp++;
	}
}

void write_block(int i, char *p)
/**************************写磁盘块
该函数把指针p指向的内容写入逻辑块
拷贝的字符个数为存储块的长度B。
***************************/
{
	char * temp = (char *)malloc(sizeof(char));
	temp = p;
	for (int a = 0; a < B;)
	{
		ldisk[i][a] = *temp;
		a++;
		temp++;
	}
}

void Init_block(char *temp, int length)
/**************************初始化一个字符数组块
处理的字符数组块长度为B
内容为0
***************************/
{
	int i;
	for (i = 0; i < length; i++)
	{
		temp[i] = '\0';
	}
}

int write_buffer(int index, int list)
{

	int i;
	int j;
	int freed;
	char temp[B];

	int buffer_length = BUFFER_LENGTH;
	for (i = 0; i < BUFFER_LENGTH; i++)
	{
		if (open_list[list].buffer[i] == '\0')
		{
			buffer_length = i;
			break;
		}
	}

	int x = open_list[list].pointer[0];
	int y = open_list[list].pointer[1];
	int z = B - y;									//当前块空闲容量

	if (buffer_length < z)					//块容量可写入缓冲区不需要再分配
	{
		read_block(x, temp);
		strncat(temp + y, open_list[list].buffer, buffer_length);			//缓冲区接入
		write_block(x, temp);

		read_block(index + FILE_SIGN_AREA, temp);	//更改文件长
		temp[1] += buffer_length;
		write_block(index + FILE_SIGN_AREA, temp);

		open_list[list].pointer[0] = x;
		open_list[list].pointer[1] = y + buffer_length;					//更新文件读写指针
	}
	else									//大于需分配新块
	{
		read_block(index + FILE_SIGN_AREA, temp);
		if (temp[2] + (buffer_length - z) / B + 1 > FILE_BLOCK_LENGTH)
		{
			printf("文件分配数组不够分配\n");
			return ERROR;
		}

		read_block(x, temp);
		strncat(temp + y, open_list[list].buffer, z);			//缓冲区接入的长度，填满当块
		write_block(x, temp);

		for (i = 0; i < (buffer_length - z) / B; i++)
		{
			for (j = K + FILE_NUM; j < L; j++)
			{
				read_block((j - K) / B, temp);
				if (temp[(j - K) % B] == FREE)
				{
					freed = j;
					break;
				}
			}
			if (j == L)
			{
				printf("磁盘已满，分配失败\n");
				return ERROR;
			}

			Init_block(temp, B);
			strncpy(temp, (open_list[list].buffer + z + (i*B)), B);
			write_block(freed, temp);				//写入内容

			read_block((freed - K) / B, temp);			//更改位图状态
			temp[(freed - K) % B] = BUSY;
			write_block((freed - K) / B, temp);			//写入磁盘，位图(文件内容所对应的)

			read_block(index + FILE_SIGN_AREA, temp);
			temp[2] ++;
			temp[2 + temp[2]] = freed;
			write_block(index + FILE_SIGN_AREA, temp);
		}

		for (j = K + FILE_NUM; j < L; j++)
		{
			read_block((j - K) / B, temp);
			if (temp[(j - K) % B] == FREE)
			{
				freed = j;
				break;
			}
		}
		if (j == L)
		{
			printf("磁盘已满，分配失败\n");
			return ERROR;
		}
		Init_block(temp, B);
		strncpy(temp, (open_list[list].buffer + z + (i*B)), (buffer_length - z) % B);
		write_block(freed, temp);				//写入内容

		read_block((freed - K) / B, temp);			//更改位图状态
		temp[(freed - K) % B] = BUSY;
		write_block((freed - K) / B, temp);

		read_block(index + FILE_SIGN_AREA, temp);
		temp[2] ++;
		temp[2 + temp[2]] = freed;
		write_block(index + FILE_SIGN_AREA, temp);


		read_block(index + FILE_SIGN_AREA, temp);
		temp[1] += buffer_length;
		write_block(index + FILE_SIGN_AREA, temp);

		open_list[list].pointer[0] = freed;
		open_list[list].pointer[1] = (buffer_length - z) % B;
	}
	//	printf("X = %d, Y = %d\n",open_list[list].pointer[0],open_list[list].pointer[1]);
}
int lseek(int index, int pos)

{
	int i;
	int list = -1;
	char temp[B];
	int pos_i = pos / B;
	int pos_j = pos % B;

	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}

	if (list == -1)
	{
		printf("没找到当前索引号文件,操作失败..\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}

	read_block(open_list[list].filesignnum + FILE_SIGN_AREA, temp);
	if (pos_i > temp[2] - 1)
	{
		printf("异常越界,定位失败\n");
		return ERROR;
	}



	open_list[list].pointer[0] = temp[3 + pos_i];
	open_list[list].pointer[1] = pos_j;


	return OK;
	//*****************************

}


void Init()

{

	int i;
	char temp[B];


	for (i = 0; i < L; i++)
	{
		Init_block(temp, B);
		write_block(i, temp);
	}

	for (i = K; i < L; i++)
	{
		read_block((i - K) / B, temp);
		temp[(i - K) % B] = FREE;
		write_block((i - K) % B, temp);
	}

	//************
	filesign temp_cnt_sign;
	temp_cnt_sign.filesign_flag = 1;
	temp_cnt_sign.file_length = 0;
	temp_cnt_sign.file_block = FILE_BLOCK_LENGTH;

	Init_block(temp, B);
	temp[0] = temp_cnt_sign.filesign_flag;
	temp[1] = temp_cnt_sign.file_length;
	temp[2] = temp_cnt_sign.file_block;

	for (i = 0; i < FILE_BLOCK_LENGTH; i++)
	{
		temp[i + 3] = K + i;
	}
	write_block(FILE_SIGN_AREA, temp);
	//************?
	read_block(0, temp);
	for (i = 0; i < FILE_NUM; i++)
	{
		temp[i] = FREE;
	}
	write_block(0, temp);
}


int create(char filename[])

{
	int i;
	int frees;
	int	freed;
	int freed2;
	char temps[B];
	char tempc[B];
	char temp[B];
	//**************
	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / B, temp);
		if (temp[(i - K) % B] == BUSY)
		{
			read_block(i, temp);
			if (strncmp(temp + 1, filename, FILE_NAME_LENGTH) == 0)
			{
				printf("该目录已经存在文件名为%s的文件\n", filename);
				return ERROR;
			}
		}
	}
	//*************
	for (i = FILE_SIGN_AREA; i < K; i++)
	{
		read_block(i, temp);
		if (temp[0] == FREE)
		{
			frees = i;
			break;
		}
	}
	if (i == K)
	{
		printf("没有空闲的文件描述符\n");
		return ERROR;
	}
	//*************
	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / B, temp);
		if (temp[(i - K) % B] == FREE)
		{
			freed = i;
			break;
		}
	}
	if (i == K + FILE_NUM)
	{
		printf("文件个数已达上限\n");
		return ERROR;
	}
	//*****************
	for (i = K + FILE_NUM; i < L; i++)
	{
		read_block((i - K) / B, temp);
		if (temp[(i - K) % B] == FREE)
		{
			freed2 = i;
			break;
		}
	}
	if (i == L)
	{
		printf("磁盘已满，分配失败\n");
		return ERROR;
	}


	filesign temp_filesign;
	contents temp_contents;
	//*******
	temp_filesign.filesign_flag = 1;
	temp_filesign.file_length = 0;
	temp_filesign.file_block = 1;


	Init_block(temps, B);
	temps[0] = temp_filesign.filesign_flag;
	temps[1] = temp_filesign.file_length;
	temps[2] = temp_filesign.file_block;
	temps[3] = freed2;
	for (i = 4; i < FILE_BLOCK_LENGTH; i++)
	{
		temps[i] = '\0';
	}
	write_block(frees, temps);

	//**************
	temp_contents.filesignnum = frees - FILE_SIGN_AREA;
	strncpy(temp_contents.filename, filename, FILE_NAME_LENGTH);

	Init_block(tempc, B);
	tempc[0] = temp_contents.filesignnum;
	tempc[1] = '\0';
	strcat(tempc, temp_contents.filename);
	write_block(freed, tempc);

	//****************
	read_block((freed - K) / B, temp);
	temp[(freed - K) % B] = BUSY;
	write_block((freed - K) / B, temp);
	read_block((freed2 - K) / B, temp);
	temp[(freed2 - K) % B] = BUSY;
	write_block((freed2 - K) / B, temp);
	read_block(FILE_SIGN_AREA, temp);
	temp[1]++;
	write_block(FILE_SIGN_AREA, temp);

	return OK;


}

int destroy(char * filename)
{
	int i;
	int dtys;
	int dtyd;
	int use_block;
	int index;
	char temp[B];
	char tempd[B];


	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / B, temp);
		if (temp[(i - K) % B] == BUSY)
		{
			read_block(i, temp);
			if (strncmp(temp + 1, filename, FILE_NAME_LENGTH) == 0)
			{
				dtyd = i;
				dtys = temp[0] + FILE_SIGN_AREA;
				index = temp[0];
				break;
			}
		}
	}
	if (i == K + FILE_NUM)
	{
		printf("没有找到该文件\n");
		return ERROR;
	}

	//**
	int list = -1;
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if (open_list[list].flag == BUSY && list != -1)
	{
		printf("该文件已经被打开,需要关闭才能删除\n");
		return ERROR;
	}


	//***********


	//**********
	read_block(dtys, temp);
	use_block = temp[2];
	for (i = 0; i < use_block; i++)
	{
		read_block((temp[i + 3] - K) / B, tempd);
		tempd[(temp[i + 3] - K) % B] = FREE;
		write_block((temp[i + 3] - K) / B, tempd);
	}
	//***********
	Init_block(temp, B);
	write_block(dtys, temp);




	//************
	Init_block(temp, B);
	write_block(dtyd, temp);

	//***************
	read_block((dtyd - K) / B, temp);
	temp[(dtyd - K) % B] = FREE;
	write_block((dtyd - K) / B, temp);
	//**********
	read_block(FILE_SIGN_AREA, temp);
	temp[1]--;
	write_block(FILE_SIGN_AREA, temp);


	return OK;

}
int open(char * filename)

{
	int i;
	int opd;
	int ops;
	int list;
	char temp[B];
	int index;
	//************
	for (i = K; i < K + FILE_NUM; i++)
	{
		read_block((i - K) / B, temp);
		if (temp[(i - K) % B] == BUSY)
		{
			read_block(i, temp);
			if (strncmp(temp + 1, filename, FILE_NAME_LENGTH) == 0)
			{
				opd = i;
				ops = temp[0];
				//	printf("opd: %d,ops: %d\n",opd,ops);
				break;
			}
		}
	}
	if (i == K + FILE_NUM)
	{
		printf("没有找到该文件\n");
		return ERROR;
	}

	//***********
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == ops && open_list[i].flag == BUSY)
		{
			printf("该文件已经被打开\n");
			return ERROR;
		}
	}

	//**********
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].flag != BUSY)
		{
			list = i;
			break;
		}
	}

	//**************

	open_list[list].filesignnum = ops;

	open_list[list].flag = BUSY;

	index = open_list[list].filesignnum;
	lseek(index, 0);

	Init_block(open_list[list].buffer, BUFFER_LENGTH);
	read_block(open_list[list].pointer[0], temp);
	strncpy(open_list[list].buffer, temp, BUFFER_LENGTH);

	return OK;

}

int close(int index)
/***************************关闭文件
***************************/
{
	int i;
	int list = -1;
	char temp[B];
	//***************根据index找表目
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//没找到
	{
		printf("没找到当前索引号文件,操作失败...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//输入的index对应文件没被打开
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}

	//****************将缓冲区的内容写入磁盘
	/* //测试用
	for(i = 0 ; i < BUFFER_LENGTH-1; i++ )
	{
	open_list[list].buffer[i] = 5;
	}
	*/

	write_buffer(index, list);			//将当前list打开文件表对应的缓冲区入index索引号的文件

										//****************清楚操释放表目
	Init_block(open_list[list].buffer, BUFFER_LENGTH);				//清除缓冲区
	open_list[list].filesignnum = FREE;								//清除文件描述符
	open_list[list].flag = FREE;									//清除占用标志位
	open_list[list].pointer[0] = NULL;								//清楚指针
	open_list[list].pointer[1] = NULL;
	return OK;
}
int read(int index, int mem_area, int count)
/*
从指定文件顺序读入count 个字
节mem_area 指定的内存位置。读操作从文件的读写指针指示的位置
开始。
*/
{
	int i;
	int list = -1;
	char temp[B];
	//***************根据index找打开文件表表目
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//没找到
	{
		printf("没找到当前索引号文件,操作失败...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//输入的index对应文件没被打开
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}

	//***************根据index找文件描述符
	char temp_output[OUTPUT_LENGTH];
	Init_block(temp_output, OUTPUT_LENGTH);
	char output[OUTPUT_LENGTH];
	Init_block(output, OUTPUT_LENGTH);

	read_block(FILE_SIGN_AREA + index, temp);
	int file_length = temp[1];					//文件长度
	int file_block = temp[2];					//文件实际使用块
	int file_area;

	//**********************拷贝文件内容至temp_output
	for (i = 0; i < file_block - 1; i++)
	{
		read_block(FILE_SIGN_AREA + index, temp);
		read_block(temp[3 + i], temp);
		strncpy(temp_output + i * B, temp, B);
	}
	read_block(FILE_SIGN_AREA + index, temp);
	read_block(temp[3 + i], temp);
	strncpy(temp_output + i * B, temp, B);

	//******************当前文件读写坐标
	int x = open_list[list].pointer[0];
	int y = open_list[list].pointer[1];

	for (i = 0; i < file_block; i++)
	{
		read_block(FILE_SIGN_AREA + index, temp);
		if (temp[3 + i] == x)
		{
			break;
		}
	}
	file_area = i * B + y;							//转换文件内相对位置									

	for (i = 0; i < count; i++)
	{
		output[i + mem_area] = temp_output[i + file_area];
	}

	printf("%s\n", output + mem_area);
	return OK;
}
int write(int index, int mem_area, int count)
/*
把mem_area 指定的内存位置开
始的ount 个字节顺序写入指定文件写操作从文件的读写指针指示
的位置开始。
*/
{
	int i;
	int list = -1;
	int input_length;
	char temp[B];

	//*************根据index找到文件打开表
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//没找到
	{
		printf("没找到当前索引号文件,操作失败...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//输入的index对应文件没被打开
	{
		printf("输入的索引号有误，操作失败...\n");
		return ERROR;
	}

	char input[INPUT_LENGTH];
	Init_block(input, INPUT_LENGTH);
	i = 0;
	fflush(stdin);
	while (scanf("%c", &input[i]))
	{
		if (input[i] == '\n')											//回车时终止读入
		{
			input[i] = '\0';
			break;
		}
		i++;
	}
	input_length = i;
	//*******************考虑输入串的长度需要写入的长度为input_length - mem_area
	//缓冲区容量可写入输入内容不需要再分配
	if (count <= BUFFER_LENGTH)
	{
		strncat(open_list[list].buffer, input + mem_area, count);			//存入缓冲区
	}
	//大于缓冲区长度分次写入，需要分块写入
	else
	{
		int rest;						//当前缓冲区空闲容量
		for (i = 0; i < BUFFER_LENGTH; i++)
		{
			if (open_list[list].buffer[i] == FREE)
			{
				rest = BUFFER_LENGTH - i;
				break;
			}
		}
		//第一部分，缓冲区有一定容量rest 将缓冲区写入文件，清空缓冲区	
		strncat(open_list[list].buffer + BUFFER_LENGTH - rest, input + mem_area, rest);
		write_buffer(index, list);
		Init_block(open_list[list].buffer, BUFFER_LENGTH);
		//第二部分，循环(input_length - mem_area)/BUFFER_LENGTH 块缓冲区写入文件
		for (i = 0; i < (count / BUFFER_LENGTH) - 1; i++)
		{
			strncpy(open_list[list].buffer, (input + mem_area) + rest + i * BUFFER_LENGTH, BUFFER_LENGTH);
			write_buffer(index, list);
			Init_block(open_list[list].buffer, BUFFER_LENGTH);
		}
		//第三部分，(count%BUFFER_LENGTH)剩余未满一块的写入缓冲区
		Init_block(open_list[list].buffer, BUFFER_LENGTH);
		strncpy(open_list[list].buffer, (input + mem_area) + rest + i * BUFFER_LENGTH, count%BUFFER_LENGTH);
		int buffer_start;
	}
	return OK;
}

void directory()
//列表显示所有文件及其长度。
{
	int i;
	int filenum;
	int filelength;
	char filename[FILE_NAME_LENGTH];
	char temp[B];
	char tempd[B];
	char temps[B];
	read_block(FILE_SIGN_AREA, temp);
	filenum = temp[1];						//实际存在的文件个数
	printf("\n");
	if (filenum == 0)
	{
		printf("\t\t\t\t该目录下没有文件\n");
	}

	for (i = 0; i < FILE_NUM; i++)
	{
		read_block(temp[3 + i], tempd);					//读取目录项
		if (tempd[0] != 0)
		{
			read_block(tempd[0] + FILE_SIGN_AREA, temps);		//读取文件描述符
			if (temps[0] == BUSY && tempd[0] != 0)
			{
				filelength = temps[1];
				strcpy(filename, tempd + 1);
				printf("%-10s\t\t%-2d字节\n", filename, filelength);
			}
		}
	}

	if (filenum != 0)
	{
		printf("\t\t\t\t共%d个文件\n", filenum);
	}
}

int show_openlist()
{

	int i, j;
	int openfile = 0;
	char temp[B];
	int index;
	printf("\n索引号\t\t大小\t\t文件名\n");
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].flag == BUSY)
		{
			index = open_list[i].filesignnum;
			printf("  %-2d", index);
			openfile++;
			read_block(FILE_SIGN_AREA + index, temp);
			printf("\t\t %-2d", temp[1]);
			//根据Yndex文件描述符序号找到其目录项
			for (j = K; j < K + FILE_NUM; j++)
			{
				read_block(j, temp);
				if (temp[0] == index)
				{
					printf("\t\t%-10s\n", temp + 1);
				}
			}
		}
	}
	return openfile;

}
void show_help()
{
	printf("**----------------------------------------------------------------------**\n");
	printf("**    命令                        说明                                  **\n");
	printf("** dir                  显示目录内容                                    **\n");
	printf("** create+filename      新建以filename为文件名的文件                    **\n");
	printf("** delete+filename      删除以filename为文件名的文件                    **\n");
	printf("** open+filename        打开以filename为文件名的文件                    **\n");
	printf("** close                关闭index为索引号的文件                         **\n");
	printf("** read                 请根据提示，接着要求输入索引号以及读取长度进行读**\n");
	printf("**                      取文件操作                                      **\n");
	printf("** write                请根据提示，接着要求输入索引号以及写入长度进行写**\n");
	printf("**                      入文件操作                                      **\n");
	printf("** lseek                请根据提示，接着要求输入一个不大于文件长度的数字**\n");
	printf("**                      用于定位读写指针                                **\n");
	printf("** help                 帮助                                            **\n");
	printf("** exit                 退出文件系统                                    **\n");

}



void show_ldisk()
{
	int a, b;
	for (a = 0; a < K + 30; a++)
	{
		printf("%-3d :", a);
		for (b = 0; b < B; b++)
		{
			printf("%-3d ", ldisk[a][b]);
		}
		printf("\n");
	}
}

void main()
{

	printf("\t\t\t欢迎使用该文件系统  使用帮助如下\n");
	show_help();
	Init();
	create((char*)"file1");
	create((char*)"file2");
	create((char*)"file3");
	open((char*)"file1");
	open((char*)"file2");

	char ch[CH_LENGTH];
	Init_block(ch, CH_LENGTH);
	while (gets_s(ch))
	{

		int cmd;
		char filename[FILE_NAME_LENGTH];
		//初始化
		cmd = -1;
		Init_block(filename, FILE_NAME_LENGTH);
		if (strncmp("dir", ch, 3) == 0)			//浏览目录dir(non)
		{
			cmd = DIR;
		}
		if (strncmp("create", ch, 6) == 0)			//创建文件命令create(filename)
		{
			cmd = CREATE;
			strcat(filename, ch + 7);
		}
		if (strncmp("delete", ch, 6) == 0)			//删除文件命令delete(filename)
		{
			cmd = DELETE;
			strcat(filename, ch + 7);
		}
		if (strncmp("open", ch, 4) == 0)			//打开文件命令open(filename)
		{
			cmd = OPEN;
			strcat(filename, ch + 5);
		}
		if (strncmp("close", ch, 5) == 0)			//关闭文件命令close(index)
		{
			cmd = CLOSE;
		}
		if (strncmp("read", ch, 4) == 0)			//读文件命令read(index)
		{
			cmd = READ;
		}
		if (strncmp("write", ch, 5) == 0)			//写文件命令write(index)
		{
			cmd = WRITE;
		}
		if (strncmp("lseek", ch, 5) == 0)			//指针命令lseek(index,pos)
		{
			cmd = LSEEK;
		}
		if (strncmp("oplist", ch, 6) == 0)			// 查看打开文件表
		{
			cmd = OPLIST;
		}
		if (strncmp("exit", ch, 4) == 0)			// 退出命令exit
		{
			cmd = EXIT;
			break;
		}
		if (strncmp("ldisk", ch, 5) == 0)			//查看硬盘内容（辅助用）
		{
			cmd = LDISK;
		}
		if (strncmp("help", ch, 4) == 0)			//帮助命令help(non)
		{
			cmd = HELP;
		}
		int index, count, pos;
		switch (cmd)
		{
		case DIR:
			directory();
			printf("----------------------------------------------\n");
			break;
		case CREATE:
			if (create(filename) == OK)
				printf("创建文件成功|\n");
			printf("----------------------------------------------\n");
			break;
		case DELETE:
			if (destroy(filename) == OK)
				printf("删除文件成功|\n");
			printf("----------------------------------------------\n");
			break;
		case OPEN:
			if (open(filename) == OK)
				printf("打开文件成功|\n");
			printf("----------------------------------------------\n");
			break;
		case CLOSE:
			if (show_openlist() == 0)
			{
				printf("当前没有文件被打开\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("请输入要关闭文件的索引号：\n");
			scanf("%d", &index);
			if (close(index) == OK)
				printf("关闭操作成功\n");
			printf("----------------------------------------------\n");
			break;
		case READ:
			if (show_openlist() == 0)
			{
				printf("当前没有文件被打开a\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("请输入读取文件的索引号：\n");
			scanf("%d", &index);
			printf("请输入想要读取文件长度：\n");
			scanf("%d", &count);
			if (read(index, 0, count) == OK)
				printf("读文件操作成功\n");
			printf("----------------------------------------------\n");
			break;
		case WRITE:
			if (show_openlist() == 0)
			{
				printf("当前没有文件被打开\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("请输入要写入文件的索引号：\n");
			scanf("%d", &index);
			printf("请输入想要写入文件长度：\n");
			scanf("%d", &count);
			if (write(index, 0, count) == OK)
				printf("写入操作成功\n");
			printf("----------------------------------------------\n");
			break;
		case LSEEK:
			if (show_openlist() == 0)
			{
				printf("当前没有文件被打开\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("请输入要写入文件的索引号:\n");
			scanf("%d", &index);
			printf("请输入想要设置的文件相对位置\n");
			scanf("%d", &pos);
			lseek(index, pos);
			printf("----------------------------------------------\n");
			break;
		case OPLIST:
			if (show_openlist() == 0)
			{
				printf("\t\t\n当前没有文件被打开\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("----------------------------------------------\n");
			break;
		case HELP:
			show_help();
			break;
		case LDISK:
			show_ldisk();
			break;
		default:
			printf("指令错误\n");
			printf("----------------------------------------------\n");
			break;
		}
		fflush(stdin);
		Init_block(ch, CH_LENGTH);
	}

}