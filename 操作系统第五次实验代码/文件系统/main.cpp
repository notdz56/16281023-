#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <iostream.h>
using namespace std;
#define B		10			//�洢�鳤��
#define L		500			//�洢������
#define K		100			//��������С
#define BUSY	1
#define FREE	0
#define OK		1
#define ERROR	0
#define FILE_BLOCK_LENGTH		(B-3)				//�ļ�������̿�����鳤��
#define FILE_NAME_LENGTH		(B-1)				//��ļ�������
#define FILE_SIGN_AREA			((L-1-K)/B+1)		//���������ļ���ʶ����ʼ���λͼ֮��
#define FILE_NUM				FILE_BLOCK_LENGTH	//Ŀ¼������ļ���Ŀ
#define BUFFER_LENGTH			25					//���ļ���Ŀ�еĻ���������
#define INPUT_LENGTH			100					//д�ļ�ʱ������볤��
#define OUTPUT_LENGTH			100					//���ļ�ʱ����������



struct filesign {							//�ļ�������
	int file_length;						//�ļ�����
	int filesign_flag;						//ռ�ñ�ʶλ
	int file_block;							//�ļ�������̿������ʵ�ʳ���
	int file_block_ary[FILE_BLOCK_LENGTH];	//�ļ�������̿������
};

struct contents {							//Ŀ¼��
	char filename[FILE_NAME_LENGTH];		//�ļ���
	int	 filesignnum;						//�ļ����������
};

struct openfilelist {						//���ļ����Ŀ
	char buffer[BUFFER_LENGTH];				//��д������
	int pointer[2];							//��дָ���ļ����ݵ�λ��
	int filesignnum;						//�ļ�������
	int flag;								//ռ�÷�
};

char ldisk[L][B];						//���ַ�����ģ�����

openfilelist open_list[FILE_NUM];		//���ļ���

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


										//��������
int show_openlist();				//��ʾ���ļ���,���ش��ļ�����
void directory();					//��ʾĿ¼�ļ���ϸ��Ϣ
void show_help();					//���ļ�ϵͳ�İ���
void show_ldisk();					//��ʾ�������ݣ������ã�
									//���ĺ���
void read_block(int, char *);		//�ļ�ϵͳ��IO�豸�Ľӿں�������ȡ��
void write_block(int, char *);		//�ļ�ϵͳ��IO�豸�Ľӿں�����д���
void Init();						//��ʼ���ļ�ϵͳ
int create(char *);					//�����ļ�
int destroy(char *);				//ɾ���ļ�
int open(char *);					//���ļ�
int close(int);						//�ر��ļ�
int read(int, int, int);				//���ļ�
int write(int, int, int);				//д�ļ�
int write_buffer(int, int);			//�ѻ���������д���ļ�
int lseek(int, int);					//��λ�ļ�ָ��
void Init_block(char, int);			//��ʼ���ַ�����飨������

void read_block(int i, char *p)
/**************************�����̿�
�ú������߼���i�����ݶ��뵽ָ��pָ����ڴ�λ��
�������ַ�����Ϊ�洢��ĳ���B��
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
/**************************д���̿�
�ú�����ָ��pָ�������д���߼���
�������ַ�����Ϊ�洢��ĳ���B��
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
/**************************��ʼ��һ���ַ������
������ַ�����鳤��ΪB
����Ϊ0
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
	int z = B - y;									//��ǰ���������

	if (buffer_length < z)					//��������д�뻺��������Ҫ�ٷ���
	{
		read_block(x, temp);
		strncat(temp + y, open_list[list].buffer, buffer_length);			//����������
		write_block(x, temp);

		read_block(index + FILE_SIGN_AREA, temp);	//�����ļ���
		temp[1] += buffer_length;
		write_block(index + FILE_SIGN_AREA, temp);

		open_list[list].pointer[0] = x;
		open_list[list].pointer[1] = y + buffer_length;					//�����ļ���дָ��
	}
	else									//����������¿�
	{
		read_block(index + FILE_SIGN_AREA, temp);
		if (temp[2] + (buffer_length - z) / B + 1 > FILE_BLOCK_LENGTH)
		{
			printf("�ļ��������鲻������\n");
			return ERROR;
		}

		read_block(x, temp);
		strncat(temp + y, open_list[list].buffer, z);			//����������ĳ��ȣ���������
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
				printf("��������������ʧ��\n");
				return ERROR;
			}

			Init_block(temp, B);
			strncpy(temp, (open_list[list].buffer + z + (i*B)), B);
			write_block(freed, temp);				//д������

			read_block((freed - K) / B, temp);			//����λͼ״̬
			temp[(freed - K) % B] = BUSY;
			write_block((freed - K) / B, temp);			//д����̣�λͼ(�ļ���������Ӧ��)

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
			printf("��������������ʧ��\n");
			return ERROR;
		}
		Init_block(temp, B);
		strncpy(temp, (open_list[list].buffer + z + (i*B)), (buffer_length - z) % B);
		write_block(freed, temp);				//д������

		read_block((freed - K) / B, temp);			//����λͼ״̬
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
		printf("û�ҵ���ǰ�������ļ�,����ʧ��..\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)
	{
		printf("��������������󣬲���ʧ��...\n");
		return ERROR;
	}

	read_block(open_list[list].filesignnum + FILE_SIGN_AREA, temp);
	if (pos_i > temp[2] - 1)
	{
		printf("�쳣Խ��,��λʧ��\n");
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
				printf("��Ŀ¼�Ѿ������ļ���Ϊ%s���ļ�\n", filename);
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
		printf("û�п��е��ļ�������\n");
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
		printf("�ļ������Ѵ�����\n");
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
		printf("��������������ʧ��\n");
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
		printf("û���ҵ����ļ�\n");
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
		printf("���ļ��Ѿ�����,��Ҫ�رղ���ɾ��\n");
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
		printf("û���ҵ����ļ�\n");
		return ERROR;
	}

	//***********
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == ops && open_list[i].flag == BUSY)
		{
			printf("���ļ��Ѿ�����\n");
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
/***************************�ر��ļ�
***************************/
{
	int i;
	int list = -1;
	char temp[B];
	//***************����index�ұ�Ŀ
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//û�ҵ�
	{
		printf("û�ҵ���ǰ�������ļ�,����ʧ��...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//�����index��Ӧ�ļ�û����
	{
		printf("��������������󣬲���ʧ��...\n");
		return ERROR;
	}

	//****************��������������д�����
	/* //������
	for(i = 0 ; i < BUFFER_LENGTH-1; i++ )
	{
	open_list[list].buffer[i] = 5;
	}
	*/

	write_buffer(index, list);			//����ǰlist���ļ����Ӧ�Ļ�������index�����ŵ��ļ�

										//****************������ͷű�Ŀ
	Init_block(open_list[list].buffer, BUFFER_LENGTH);				//���������
	open_list[list].filesignnum = FREE;								//����ļ�������
	open_list[list].flag = FREE;									//���ռ�ñ�־λ
	open_list[list].pointer[0] = NULL;								//���ָ��
	open_list[list].pointer[1] = NULL;
	return OK;
}
int read(int index, int mem_area, int count)
/*
��ָ���ļ�˳�����count ����
��mem_area ָ�����ڴ�λ�á����������ļ��Ķ�дָ��ָʾ��λ��
��ʼ��
*/
{
	int i;
	int list = -1;
	char temp[B];
	//***************����index�Ҵ��ļ����Ŀ
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//û�ҵ�
	{
		printf("û�ҵ���ǰ�������ļ�,����ʧ��...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//�����index��Ӧ�ļ�û����
	{
		printf("��������������󣬲���ʧ��...\n");
		return ERROR;
	}

	//***************����index���ļ�������
	char temp_output[OUTPUT_LENGTH];
	Init_block(temp_output, OUTPUT_LENGTH);
	char output[OUTPUT_LENGTH];
	Init_block(output, OUTPUT_LENGTH);

	read_block(FILE_SIGN_AREA + index, temp);
	int file_length = temp[1];					//�ļ�����
	int file_block = temp[2];					//�ļ�ʵ��ʹ�ÿ�
	int file_area;

	//**********************�����ļ�������temp_output
	for (i = 0; i < file_block - 1; i++)
	{
		read_block(FILE_SIGN_AREA + index, temp);
		read_block(temp[3 + i], temp);
		strncpy(temp_output + i * B, temp, B);
	}
	read_block(FILE_SIGN_AREA + index, temp);
	read_block(temp[3 + i], temp);
	strncpy(temp_output + i * B, temp, B);

	//******************��ǰ�ļ���д����
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
	file_area = i * B + y;							//ת���ļ������λ��									

	for (i = 0; i < count; i++)
	{
		output[i + mem_area] = temp_output[i + file_area];
	}

	printf("%s\n", output + mem_area);
	return OK;
}
int write(int index, int mem_area, int count)
/*
��mem_area ָ�����ڴ�λ�ÿ�
ʼ��ount ���ֽ�˳��д��ָ���ļ�д�������ļ��Ķ�дָ��ָʾ
��λ�ÿ�ʼ��
*/
{
	int i;
	int list = -1;
	int input_length;
	char temp[B];

	//*************����index�ҵ��ļ��򿪱�
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].filesignnum == index)
		{
			list = i;
			break;
		}
	}
	if (list == -1)					//û�ҵ�
	{
		printf("û�ҵ���ǰ�������ļ�,����ʧ��...\n");
		return ERROR;
	}
	if (open_list[list].flag != BUSY)		//�����index��Ӧ�ļ�û����
	{
		printf("��������������󣬲���ʧ��...\n");
		return ERROR;
	}

	char input[INPUT_LENGTH];
	Init_block(input, INPUT_LENGTH);
	i = 0;
	fflush(stdin);
	while (scanf("%c", &input[i]))
	{
		if (input[i] == '\n')											//�س�ʱ��ֹ����
		{
			input[i] = '\0';
			break;
		}
		i++;
	}
	input_length = i;
	//*******************�������봮�ĳ�����Ҫд��ĳ���Ϊinput_length - mem_area
	//������������д���������ݲ���Ҫ�ٷ���
	if (count <= BUFFER_LENGTH)
	{
		strncat(open_list[list].buffer, input + mem_area, count);			//���뻺����
	}
	//���ڻ��������ȷִ�д�룬��Ҫ�ֿ�д��
	else
	{
		int rest;						//��ǰ��������������
		for (i = 0; i < BUFFER_LENGTH; i++)
		{
			if (open_list[list].buffer[i] == FREE)
			{
				rest = BUFFER_LENGTH - i;
				break;
			}
		}
		//��һ���֣���������һ������rest ��������д���ļ�����ջ�����	
		strncat(open_list[list].buffer + BUFFER_LENGTH - rest, input + mem_area, rest);
		write_buffer(index, list);
		Init_block(open_list[list].buffer, BUFFER_LENGTH);
		//�ڶ����֣�ѭ��(input_length - mem_area)/BUFFER_LENGTH �黺����д���ļ�
		for (i = 0; i < (count / BUFFER_LENGTH) - 1; i++)
		{
			strncpy(open_list[list].buffer, (input + mem_area) + rest + i * BUFFER_LENGTH, BUFFER_LENGTH);
			write_buffer(index, list);
			Init_block(open_list[list].buffer, BUFFER_LENGTH);
		}
		//�������֣�(count%BUFFER_LENGTH)ʣ��δ��һ���д�뻺����
		Init_block(open_list[list].buffer, BUFFER_LENGTH);
		strncpy(open_list[list].buffer, (input + mem_area) + rest + i * BUFFER_LENGTH, count%BUFFER_LENGTH);
		int buffer_start;
	}
	return OK;
}

void directory()
//�б���ʾ�����ļ����䳤�ȡ�
{
	int i;
	int filenum;
	int filelength;
	char filename[FILE_NAME_LENGTH];
	char temp[B];
	char tempd[B];
	char temps[B];
	read_block(FILE_SIGN_AREA, temp);
	filenum = temp[1];						//ʵ�ʴ��ڵ��ļ�����
	printf("\n");
	if (filenum == 0)
	{
		printf("\t\t\t\t��Ŀ¼��û���ļ�\n");
	}

	for (i = 0; i < FILE_NUM; i++)
	{
		read_block(temp[3 + i], tempd);					//��ȡĿ¼��
		if (tempd[0] != 0)
		{
			read_block(tempd[0] + FILE_SIGN_AREA, temps);		//��ȡ�ļ�������
			if (temps[0] == BUSY && tempd[0] != 0)
			{
				filelength = temps[1];
				strcpy(filename, tempd + 1);
				printf("%-10s\t\t%-2d�ֽ�\n", filename, filelength);
			}
		}
	}

	if (filenum != 0)
	{
		printf("\t\t\t\t��%d���ļ�\n", filenum);
	}
}

int show_openlist()
{

	int i, j;
	int openfile = 0;
	char temp[B];
	int index;
	printf("\n������\t\t��С\t\t�ļ���\n");
	for (i = 0; i < FILE_NUM; i++)
	{
		if (open_list[i].flag == BUSY)
		{
			index = open_list[i].filesignnum;
			printf("  %-2d", index);
			openfile++;
			read_block(FILE_SIGN_AREA + index, temp);
			printf("\t\t %-2d", temp[1]);
			//����Yndex�ļ�����������ҵ���Ŀ¼��
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
	printf("**    ����                        ˵��                                  **\n");
	printf("** dir                  ��ʾĿ¼����                                    **\n");
	printf("** create+filename      �½���filenameΪ�ļ������ļ�                    **\n");
	printf("** delete+filename      ɾ����filenameΪ�ļ������ļ�                    **\n");
	printf("** open+filename        ����filenameΪ�ļ������ļ�                    **\n");
	printf("** close                �ر�indexΪ�����ŵ��ļ�                         **\n");
	printf("** read                 �������ʾ������Ҫ�������������Լ���ȡ���Ƚ��ж�**\n");
	printf("**                      ȡ�ļ�����                                      **\n");
	printf("** write                �������ʾ������Ҫ�������������Լ�д�볤�Ƚ���д**\n");
	printf("**                      ���ļ�����                                      **\n");
	printf("** lseek                �������ʾ������Ҫ������һ���������ļ����ȵ�����**\n");
	printf("**                      ���ڶ�λ��дָ��                                **\n");
	printf("** help                 ����                                            **\n");
	printf("** exit                 �˳��ļ�ϵͳ                                    **\n");

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

	printf("\t\t\t��ӭʹ�ø��ļ�ϵͳ  ʹ�ð�������\n");
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
		//��ʼ��
		cmd = -1;
		Init_block(filename, FILE_NAME_LENGTH);
		if (strncmp("dir", ch, 3) == 0)			//���Ŀ¼dir(non)
		{
			cmd = DIR;
		}
		if (strncmp("create", ch, 6) == 0)			//�����ļ�����create(filename)
		{
			cmd = CREATE;
			strcat(filename, ch + 7);
		}
		if (strncmp("delete", ch, 6) == 0)			//ɾ���ļ�����delete(filename)
		{
			cmd = DELETE;
			strcat(filename, ch + 7);
		}
		if (strncmp("open", ch, 4) == 0)			//���ļ�����open(filename)
		{
			cmd = OPEN;
			strcat(filename, ch + 5);
		}
		if (strncmp("close", ch, 5) == 0)			//�ر��ļ�����close(index)
		{
			cmd = CLOSE;
		}
		if (strncmp("read", ch, 4) == 0)			//���ļ�����read(index)
		{
			cmd = READ;
		}
		if (strncmp("write", ch, 5) == 0)			//д�ļ�����write(index)
		{
			cmd = WRITE;
		}
		if (strncmp("lseek", ch, 5) == 0)			//ָ������lseek(index,pos)
		{
			cmd = LSEEK;
		}
		if (strncmp("oplist", ch, 6) == 0)			// �鿴���ļ���
		{
			cmd = OPLIST;
		}
		if (strncmp("exit", ch, 4) == 0)			// �˳�����exit
		{
			cmd = EXIT;
			break;
		}
		if (strncmp("ldisk", ch, 5) == 0)			//�鿴Ӳ�����ݣ������ã�
		{
			cmd = LDISK;
		}
		if (strncmp("help", ch, 4) == 0)			//��������help(non)
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
				printf("�����ļ��ɹ�|\n");
			printf("----------------------------------------------\n");
			break;
		case DELETE:
			if (destroy(filename) == OK)
				printf("ɾ���ļ��ɹ�|\n");
			printf("----------------------------------------------\n");
			break;
		case OPEN:
			if (open(filename) == OK)
				printf("���ļ��ɹ�|\n");
			printf("----------------------------------------------\n");
			break;
		case CLOSE:
			if (show_openlist() == 0)
			{
				printf("��ǰû���ļ�����\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("������Ҫ�ر��ļ��������ţ�\n");
			scanf("%d", &index);
			if (close(index) == OK)
				printf("�رղ����ɹ�\n");
			printf("----------------------------------------------\n");
			break;
		case READ:
			if (show_openlist() == 0)
			{
				printf("��ǰû���ļ�����a\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("�������ȡ�ļ��������ţ�\n");
			scanf("%d", &index);
			printf("��������Ҫ��ȡ�ļ����ȣ�\n");
			scanf("%d", &count);
			if (read(index, 0, count) == OK)
				printf("���ļ������ɹ�\n");
			printf("----------------------------------------------\n");
			break;
		case WRITE:
			if (show_openlist() == 0)
			{
				printf("��ǰû���ļ�����\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("������Ҫд���ļ��������ţ�\n");
			scanf("%d", &index);
			printf("��������Ҫд���ļ����ȣ�\n");
			scanf("%d", &count);
			if (write(index, 0, count) == OK)
				printf("д������ɹ�\n");
			printf("----------------------------------------------\n");
			break;
		case LSEEK:
			if (show_openlist() == 0)
			{
				printf("��ǰû���ļ�����\n");
				printf("----------------------------------------------\n");
				break;
			}
			printf("������Ҫд���ļ���������:\n");
			scanf("%d", &index);
			printf("��������Ҫ���õ��ļ����λ��\n");
			scanf("%d", &pos);
			lseek(index, pos);
			printf("----------------------------------------------\n");
			break;
		case OPLIST:
			if (show_openlist() == 0)
			{
				printf("\t\t\n��ǰû���ļ�����\n");
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
			printf("ָ�����\n");
			printf("----------------------------------------------\n");
			break;
		}
		fflush(stdin);
		Init_block(ch, CH_LENGTH);
	}

}