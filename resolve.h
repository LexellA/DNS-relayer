#pragma once 

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <time.h>
#include "DNS.h"
#include "cache.h"

#pragma comment(lib, "ws2_32.lib")

const int Port;//端口

SOCKADDR_IN addrSrv; //服务端地址

SOCKET sockSrv;//服务端套接字


Bool init_socket();//初始化socket

int resolve_question(char* recvData, char* sendData, int recvSize, SOCKADDR_IN* addrSrc);//解析问题包

int resolve_answer(char* recvData, char* sendData, int recvSize, SOCKADDR_IN* addrSrc);

void release_socket();

int get_domainName(const unsigned char* nName, unsigned char* pName);