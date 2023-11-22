#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>
#include <time.h>
#include <string.h>
#include "cache.h"
#include "resolve.h"
#include "DNS.h"

int	main(int argc, char* argv[]) {
	debug_information = get_debug_information(argc, argv);
	if (debug_information.debug_level != 0 && 
		strcmp(debug_information.debug_server_ip, "0") == 0 && 
		strcmp(debug_information.configuration_file, "0") == 0) {
		return 0;
	}
	else {
		printf("init debug information succeed！\n");
	}

	//初始化缓冲区
	cache = init_cache();
	printf("init cache succeed！\n");

	//初始化id转换表
	init_id_queue();
	printf("init ID queue succeed！\n");

	//初始化本地数据库
	get_database();
	printf("succeed static database succeed！\n");

	//初始化连接
	if (init_socket()) 
	{
		printf("init socket succeed！\n");
	}
	else 
	{
		printf("init socket failed\n");
		release_socket();
		return 0;
	}

	show_title();

	SOCKADDR_IN addrCli;	/*客户端地址*/
	int addrCli_size = sizeof(addrCli);			 /*客户端地址的大小*/
	
	while (True) 
	{
		printf("\n");
		char recvData[MAX_BUFFER_SIZE];	//接受的数据
		char sendData[MAX_BUFFER_SIZE];	//发送的数据

		int sendSize = 0;				//接受的数据大小
		int recvSize = 0;				//发送的数据大小
		printf("\n*******************************************************************************************\n");
		recvSize = recvfrom(sockSrv, (char*)recvData, MAX_BUFFER_SIZE, 0, (SOCKADDR*)&addrCli, &addrCli_size);
		if (recvSize <= 0) 
		{
			printf("recvfrom() failed ！\n");
			recvSize = 0;
			continue;
		}

		if ((recvData[2] & 0x80) >> 7 == 0) 
		{//QR = 0代表的是询问报文
			printf("receive a query message and size = %d ！\n", recvSize);
			
			debug_buffer(recvData, recvSize);
			sendSize = resolve_question(recvData, sendData, recvSize, &addrCli);
			
			if (sendSize < 0)
			{
				printf("resolve_question() failed\n");
				continue;
			}
		}
		else {
			printf("receive an answer message and size = %d ！\n", recvSize);

			debug_buffer(recvData, recvSize);
			sendSize = resolve_answer(recvData, sendData, recvSize, &addrCli);
			if (sendSize < 0) 
			{
				printf("resolve_answer() failed！\n");
				continue;
			}
		}

		addrCli_size = sizeof(addrCli);
		sendto(sockSrv, (char*)sendData, sendSize, 0, (SOCKADDR*)&addrCli, addrCli_size);
		debug_buffer(sendData, sendSize);
		printf_d("sendto() succeed！\n");
	}

	release_socket();
	return 0;
}