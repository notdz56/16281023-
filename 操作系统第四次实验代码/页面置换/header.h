#pragma once
struct LNode
{
	int data;
	int flag;//����λ
	int modify;//�޸�λ
	LNode* next;
};
struct Link
{
	int num;//��ǰ�����ϵĽ����
	LNode* next;
};

typedef struct node
{
	int num;
	node* next;
} Node, *pNode;

typedef struct queue
{
	int n;
	pNode front;
	pNode rear;

} Queue, *pQueue;

void initMemo();
void generate();//���ɷ�������
bool  isInMemo(int n); //
void optimal(int n); //
void testOptimal();
void LRU(int n);
void testLRU();
void updated_Clock(int n);
void test_Clock();

void initQueue(pQueue q);
void push(pQueue q, int num);
void pop(pQueue q);
void destroy(pQueue q);
bool findInQueue(pQueue q, int num);
void fifoTest();
void fifo(pQueue q, int num);

void test_PBA();
bool isInNodes(int n); 
void addToLink(int data, int type);
void emptyIdle();
void emptyModi();
void PBA(int n);