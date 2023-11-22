#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <WinSock2.h>


#define MAX_BUFFER_SIZE 512 //UDP接受的最大负载
#define MAX_DOMAINNAME_LENGTH 100 //最大域名长度
#define MAX_IP_LENGTH 128
#define MAX_FILENAME_LENGTH 100

#define INT_MAX 2147483647

//Resource Record的资源类型
#define TYPE_A 1
#define TYPE_AAAA 28
#define TYPE_PTR 12
#define TYPE_CNAME 5
#define TYPE_HINFO 13
#define TYPE_MX 15
#define TYPE_NS 2

typedef unsigned short uint_16;
typedef int uint_32;

typedef struct Dns_header_field {
    uint_16 id : 16;    /* query identification number */
    uint_16 rd : 1;     /* recursion desired */
    uint_16 tc : 1;     /* truncated message */
    uint_16 aa : 1;     /* authoritive answer */
    uint_16 opcode : 4; /* purpose of message */
    uint_16 qr : 1;     /* response flag */
    uint_16 rcode : 4;  /* response code */
    uint_16 cd : 1;     /* checking disabled by resolver */
    uint_16 ad : 1;     /* authentic data from named */
    uint_16 z : 1;      /* unused bits, must be ZERO */
    uint_16 ra : 1;     /* recursion available */
    uint_16  qdcount;       /* number of question entries */
    uint_16  ancount;       /* number of answer entries */
    uint_16  nscount;       /* number of authority entries */
    uint_16  arcount;       /* number of resource entries */
}DNS_header_field;

typedef struct Dns_information {
    int debug_level;//调试等级 -d为1 -dd为2
    char debug_client_ip[MAX_IP_LENGTH];//客户端ip 默认值"127.0.0.1"
    char debug_server_ip[MAX_IP_LENGTH];//服务器ip 默认值"10.3.9.44"
    char configuration_file[MAX_FILENAME_LENGTH];//配置文件
}DNS_information;

typedef enum Bool
{
    False, True
}Bool;

void show_title();

DNS_information debug_information;

DNS_information get_debug_information(int argc, char * argv[]);

Bool is_ip_valid(char* ip);

void printf_d(char* s, ...);

void debug_buffer(const unsigned char* buf, int bufSize);