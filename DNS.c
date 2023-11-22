#pragma once
#include "DNS.h"
#include "cache.h"
#include"resolve.h"
#include <stdarg.h>

DNS_information debug_information;


void show_title()
{
	printf("\t\t***********************************************************\n");
	printf("\t\t* ------------------------------------------------------- *\n");
	printf("\t\t* @Course Name: Course Design of Computer Network         *\n");
	printf("\t\t* @Team members: He Zhizhen Liu Lizi     *\n");
	printf("\t\t* @Teacher: Jiang Yanjun         @Class number: 302       *\n");
	printf("\t\t* ------------------------------------------------------- *\n");
	printf("\t\t*                   DNS Relay Server                      *\n");
	printf("\t\t***********************************************************\n");
	printf("\t\t***********************************************************\n");
	printf("\t\t*                        profile:                         *\n");
	printf("\t\t*    debug level：%d                                       *\n",debug_information.debug_level);
	printf("\t\t*    ip file：%10s                                *\n",debug_information.configuration_file);
	printf("\t\t*    extern dns address：%16s                 *\n",debug_information.debug_server_ip);
	printf("\t\t***********************************************************\n");
}

DNS_information get_debug_information(int argc ,char* argv[])
{
	DNS_information temp = { 0 ,"127.0.0.1" ,"10.3.9.44" ,"dnsrelay.txt" };
	int temp_argc = 1;
	if (argc == 1) 
	{
		printf("请输入调试信息 -d / -dd\n");
		return temp;
	}
	if (strcmp(argv[1], "-d") == 0) 
	{
		temp.debug_level = 1;
		temp_argc++;
	}
	else if (strcmp(argv[1], "-dd") == 0) 
	{
		temp.debug_level = 2;
		temp_argc++;
	}
	else 
	{
		printf("请重新输入调试信息 -d / -dd\n");
		return temp;
	}
	if (temp_argc >= argc) 
	{
		return temp;
	}
	else 
	{
		int ip[4];
		if (sscanf(argv + temp_argc, "%d.%d.%d.%d", ip[0] ,ip[1], ip[2], ip[3]) == 4)//需要传指针进去
		{
			if (is_ip_valid(argv[temp_argc]) == 1)//判断是否为合法ip
			{
				strcpy(temp.debug_server_ip, argv[temp_argc]);
			}
			else 
			{
				strcpy(temp.debug_server_ip, "0");
				printf("请输入正确的DNS服务器ip地址\n");
			}
			temp_argc++;
		}

		if (temp_argc >= argc)
			return temp;

		if (strcmp(argv[temp_argc] + strlen(argv[temp_argc]) - 4, ".txt") != 0)
		{
			strcpy(temp.configuration_file, argv[temp_argc]);
			temp_argc++;
		}
		else 
		{
			strcpy(temp.configuration_file, "0");
			printf("请输入正确的配置文件名\n");
		}
	}
	return temp;
}

Bool is_ip_valid(char* ip)
{
	int length = strlen(ip);
	int num = 0;
	int i;
	for (i = 0; i < length; i++) 
	{
		if (ip[i] >= '0' || ip[i] <= '9')
		{
			num = num * 10 + (ip[i] - '0');
		}
		else if (ip[i] == '.') 
		{
			if (num >= 256 || num < 0)
				return False;
			else 
				num = 0;
		}
	}
	if (num >= 256 || num < 0)//最后一组数字没有校验
	{
		return False;
	}
	return True;
}

void printf_d(char* s, ...)//代表可以传多个参数
{
	if (debug_information.debug_level < 2)
	{
		return;
	}
	va_list args;
	va_start(args, s);
	vprintf(s, args);
	va_end(args);
}

void debug_buffer(const unsigned char* buf, int bufSize) 
{
	if (debug_information.debug_level < 2) //只有"-dd"才需要输出调试信息
	{
		return;
	}
	char isEnd = 0;
	if (bufSize > MAX_BUFFER_SIZE)
	{
		printf_d("DebugBuffer() failed, bufSize too big: %d > %d", bufSize, MAX_BUFFER_SIZE);
	}
	else 
	{
		for (int i = 0; i < bufSize; ++i) 
		{
			printf_d("%02x ", buf[i]);
			isEnd = 0;
			if (i % 16 == 15) {
				printf_d("\n");
				isEnd = 1;
			}
		}
		if (!isEnd) 
		{
			printf_d("\n");
		}
	}
}