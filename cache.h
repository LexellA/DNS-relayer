#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <time.h>
#include "DNS.h"

//cache的最大容量和database容量
#define MAX_CACHE_CAPACITY 100
#define MAX_DATABASE_CAPACITY 3000

//最大id转换队列长度，超时时间
#define MAX_QUEUE_SIZE 65535
#define TIMEOUT 3

typedef struct Cache{
    char domainname[MAX_DOMAINNAME_LENGTH]; //域名
    char buffer[MAX_BUFFER_SIZE];           //报文
    unsigned short type;                    //类型
    int buffersize;                         //报文大小
    int timeout;                            //超时时间
    int insert_time;                        //加入cache的时间
    struct Cache* next;
}CACHE;

typedef struct Data {
    char ip[MAX_IP_LENGTH];
    char domainname[MAX_DOMAINNAME_LENGTH];
}data;

typedef struct IdEntry{
    SOCKADDR_IN addr;   //套接字地址
    unsigned short Id;  //id
    Bool is_replyed;    //是否回复
    int timeout;        //超时时间
}IdEntry;

typedef struct IdQueue{
    IdEntry queue[MAX_QUEUE_SIZE];  //id转换队列
    int front;                      //队头
    int rear;                       //队尾
}IdQueue;

data database[MAX_DATABASE_CAPACITY];   //静态表
int database_count;                     //静态表数量
CACHE* cache;                           //缓存表
int cache_count;                        //缓存数量
IdQueue id_queue;                       //id转换队列

void get_database();//读取文件

CACHE* init_cache();//初始化缓存

void insert_cache(CACHE* target_data);//加入cache
Bool is_hit_database(char* domainname, char* ip);//在database中寻找
Bool is_hit_cache(char* domainname, char* buffer, unsigned short type, int* buffersize, int* timepassed);//在cache中寻找
void insert_buffer(char* domainname, char* buffer, unsigned short type, int ttl, int buffersize);//将报文加入cache

void init_id_queue();//初始化id转换队列
Bool push_id_queue(SOCKADDR_IN* addr, unsigned short originId, unsigned short* new_id);//将原id以及客户端地址存入队列
Bool pop_id_queue();//出队
Bool set_id_replied(unsigned short id);//设置已经回复
Bool get_id_entry(unsigned short id, IdEntry* id_entry);//得到id转换项