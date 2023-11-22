#include "resolve.h"

const int Port = 53;

Bool init_socket()
{
	WSADATA wasData;
	if (WSAStartup(MAKEWORD(2, 2), &wasData) != 0)
	{
		return False;
	}

	sockSrv = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (sockSrv == INVALID_SOCKET)
	{
		return False;
	}

	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(Port);
	addrSrv.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(addrSrv)) == SOCKET_ERROR)
	{
		return False;
	}
	return True;
}

void release_socket()
{
	closesocket(sockSrv);
	WSACleanup();
}

int resolve_question(char* recvData, char* sendData, int recvSize, SOCKADDR_IN* addrSrc) 
{
	char domainname[MAX_DOMAINNAME_LENGTH] = { '\0' };
	char ip[MAX_IP_LENGTH] = { '\0' };
	unsigned short type;
	int sendSize = 0;
	int timepassed = 0;

	int domainlen = get_domainName(recvData + 12, domainname);
	type = ntohs(*((short*)(recvData + 12 + domainlen)));

	printf("domainname is %s  type = %hd\n", domainname, type);

	
	if (is_hit_cache(domainname, sendData, type, &sendSize, &timepassed)) {
		printf_d("-------------------hit cache------------------\n");
		((DNS_header_field*)sendData)->id = ((DNS_header_field*)recvData)->id;//修改id

		unsigned short an_count = ntohs(((DNS_header_field*)sendData)->ancount);
		for (int i = 0, p = 12 + domainlen + 4; i < an_count; i++)//p首先跳过报头，问题域
		{
			p = p + 6;//跳过name, type, class

			int tempttl = ntohl(*((int*)(sendData + p)));
			tempttl = tempttl - timepassed;
			*((int*)(sendData + p)) = htonl(tempttl);

			p = p + 4;//跳过ttl
			short datalen = ntohs(*((short*)(sendData + p)));
			p = p + 2 + datalen;//跳过datalen, data
		}//修改ttl
		return sendSize;
	}
	else if (type == TYPE_A && is_hit_database(domainname, ip))
	{
		printf_d("------------------hit database------------------\n");
		memcpy(sendData, recvData, recvSize);//先复制
		((DNS_header_field*)sendData)->qr = 1;//修改QR
		((DNS_header_field*)sendData)->aa = 1;//修改AA
		((DNS_header_field*)sendData)->ra = 1;//修改RA

		if (!strcmp(ip, "0.0.0.0"))//不良网站拦截
		{
			((DNS_header_field*)sendData)->ancount = 0;
			((DNS_header_field*)sendData)->rcode = 3;
			printf_d("Illegal website has been intercept!\n");
			return recvSize;
		}
		((DNS_header_field*)sendData)->ancount = htons(1);


		unsigned char answer_rr[16] = { '\0' };
		answer_rr[0] = 0xc0;	
		answer_rr[1] = 0x0c;    //指针指向问题域名c00c
		answer_rr[3] = 0x01;	//type = TYPE_A
		answer_rr[5] = 0x01;	//class = 1			
		answer_rr[9] = 0x78;	//ttl = 120		
		answer_rr[11] = 0x04;   //data length = 4
		inet_pton(AF_INET, ip, answer_rr + 12);	//点分十进制转为二进制
		memcpy(sendData + recvSize, answer_rr, 16);
		return recvSize + 16;//rr包的16字节
	}
	else //cache以及静态库中都没有
	{
		printf_d("ip is not in database and cache，ask Internet DNS for help\n");
		
		unsigned short origin_id = ntohs(((DNS_header_field*)recvData)->id);
		unsigned short new_id = 0;

		if (push_id_queue(addrSrc, origin_id, &new_id)) //把客户端的地址还有原id传入
		{
			//复制
			memcpy(sendData, recvData, recvSize);
			//改id
			((DNS_header_field*)sendData)->id = new_id;
			//更改目标地址
			addrSrc->sin_family = AF_INET;
			addrSrc->sin_port = htons(53);
			inet_pton(AF_INET, debug_information.debug_server_ip, &addrSrc->sin_addr);//最主要的修改地址在这部分
			return recvSize;//发送大小和接收大小相同
		}
		return -1;//队列已经满了，直接把数据包丢弃
	}
}

int resolve_answer(char* recvData, char* sendData, int recvSize, SOCKADDR_IN* addrSrc) 
{
	char domainname[MAX_DOMAINNAME_LENGTH] = { '\0' };//域名
	int domainlen;//域名长度
	unsigned short type;
	unsigned short qd_count ,an_count ,ns_count ,ar_count;
	int minttl = INT_MAX;//报文中最短ttl
	IdEntry id_entry = { 0 };

	qd_count = ntohs(((DNS_header_field*)recvData)->qdcount);
	an_count = ntohs(((DNS_header_field*)recvData)->ancount);
	ns_count = ntohs(((DNS_header_field*)recvData)->nscount);
	ar_count = ntohs(((DNS_header_field*)recvData)->arcount);
	domainlen = get_domainName(recvData + 12, domainname); //获取域名和域名长度
	type = ntohs(*((short*)(recvData + 12 + domainlen))); //获取类型

	printf("receive domainname answer message: %s\n" ,domainname);
	printf("QDCOUNT = %d	 ANCOUNT = %d	NSCOUNT = %d	ARCOUNT = %d	TYPE = %d\n"
		, qd_count, an_count, ns_count, ar_count ,type);

	for (int i = 0, p = 12 + domainlen + 4; i < an_count; i++)//i代表answer的数量 ，p就是指针
	{
		p = p + 6;
		int tempttl = ntohl(*((int*)(recvData + p)));
		minttl = tempttl < minttl ? tempttl : minttl;
		p = p + 4;
		short datalen = ntohs(*((short*)(recvData + p)));
		p = p + 2 + datalen;
	}

	if (((DNS_header_field*)recvData)->rcode == 0 && an_count != 0) 
	{
		printf_d("add to cache : cache count =  %d\n", cache_count);
		insert_buffer(domainname, recvData, type, minttl, recvSize);
	}
	else
	{
		printf_d("domainname is wrong or ANCOUNT = 0 ,not add to cache\n");
	}
	unsigned short new_id = ((DNS_header_field*)recvData)->id;

	pop_id_queue();

	if (get_id_entry(new_id, &id_entry) == 1) 
	{
		if (id_entry.is_replyed) 
		{
			printf_d("id = %d has been replied\n" , new_id);
			return -1;
		}

		set_id_replied(new_id);
		pop_id_queue();

		memcpy(sendData, recvData, recvSize);
		DNS_header_field* sendData_header = (DNS_header_field*)sendData;
		sendData_header->id = htons(id_entry.Id);//找到对应的id
		*addrSrc = id_entry.addr;//找到客户端

		return recvSize;
	}
	printf_d("id = %d has been deleted\n" ,new_id);
	return -1;//id转换表中没有这个记录
}

int get_domainName(const unsigned char* nName, unsigned char* pName) 
{
	memcpy(pName, nName + 1, strlen((char*)nName));//去掉第一个字节
	int length;
	length = strlen((char*)nName) + 1;
	int i = nName[0];
	while (pName[i]) //解读压缩域名，压缩的"."的位置为偏移量
	{
		int offset = pName[i];
		pName[i] = '.';
		i += offset + 1;
	}
	return length;
}
