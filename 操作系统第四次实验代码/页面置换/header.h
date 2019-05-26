#pragma once
struct LNode
{
	int data;
	int flag;//访问位
	int modify;//修改位
	LNode* next;
};
struct Link
{
	int num;//当前链表上的结点数
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
void generate();//生成访问序列
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