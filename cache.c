#include "cache.h"

data database[MAX_DATABASE_CAPACITY];
CACHE* cache;
int database_count = 0;
int cache_count = 0;
IdQueue id_queue;


CACHE* init_cache() 
{
	CACHE* temp = (CACHE*)malloc(sizeof(CACHE) * 1);
	temp->next = NULL;
	return temp;
}

void insert_cache(CACHE* target_data) 
{
	target_data->next = cache->next;
	cache->next = target_data;
	if (cache_count < MAX_CACHE_CAPACITY)
	{
		cache_count++;
	}
	else 
	{
		CACHE* temp = cache;
		while (temp->next->next != NULL) 
		{
			temp = temp->next;
		}
		CACHE* delete_cache = temp->next;
		temp->next = NULL;
		free(delete_cache);
	}
}

Bool is_hit_cache(char* domainname, char* buffer, unsigned short type, int* buffersize, int* timepassed) 
{
	CACHE* temp = cache->next;
	CACHE* pre_temp = cache;


	while (temp != NULL) 
	{
		if (type == temp->type && strcmp(domainname, temp->domainname) == 0)
		{
			if (temp->timeout < time(NULL))
			{
				break;
			}
			
			memcpy(buffer, temp->buffer, temp->buffersize);
			*buffersize = temp->buffersize;
			*timepassed = time(NULL) - temp->insert_time;

			pre_temp->next = temp->next;//???е?cache??????????
			temp->next = cache->next;
			cache->next = temp;
			return True;
		}
		pre_temp = pre_temp->next;
		temp = temp->next;
	}

	return False;
}

void get_database() 
{
	FILE* file = fopen(debug_information.configuration_file, "r");

	if (file == NULL)
	{
		return;
	}

	while (!feof(file) && database_count < MAX_DATABASE_CAPACITY) 
	{
		fscanf(file, "%s %s", database[database_count].ip, database[database_count].domainname);
		database_count++;
	}
}

Bool is_hit_database(char* domainname, char* ip) 
{
	int i;
	for (i = 0; i < database_count; i++) 
	{
		if (strcmp(database[i].domainname, domainname) == 0) 
		{
			strcpy(ip, database[i].ip);
			return True;
		}
	}
	return False;
}

void insert_buffer(char* domainname, char* buffer, unsigned short type, int ttl, int buffersize)
{
	CACHE* temp = (CACHE*)malloc(sizeof(CACHE) * 1);
	strcpy(temp->domainname, domainname);
	memcpy(temp->buffer, buffer, buffersize);
	temp->timeout = ttl + time(NULL);
	temp->type = type;
	temp->buffersize = buffersize;
	temp->next = NULL;
	temp->insert_time = time(NULL);
	insert_cache(temp);
}

void init_id_queue() 
{
	id_queue.front = 0;
	id_queue.rear = 0;
}

Bool push_id_queue(SOCKADDR_IN* addr, unsigned short originId, unsigned short* new_id)
{
	if (((id_queue.rear + 1) % MAX_QUEUE_SIZE) == id_queue.front) 
	{
		printf_d("ID table queue is in his max capacity??\n");
		return False;
	}
	id_queue.queue[id_queue.rear].addr = *addr;
	id_queue.queue[id_queue.rear].Id = originId;
	id_queue.queue[id_queue.rear].is_replyed = 0;
	id_queue.queue[id_queue.rear].timeout = time(NULL) + TIMEOUT;

	*new_id = id_queue.rear;
	id_queue.rear = (id_queue.rear + 1) % MAX_QUEUE_SIZE;
	return True;
}

Bool pop_id_queue() 
{
	if (id_queue.rear == id_queue.front) 
	{
		printf_d("queue is empty??pop_id_queue() failed??\n");
		return False;
	}
	while (id_queue.front != id_queue.rear && (id_queue.queue[id_queue.front].is_replyed == 1 
		|| id_queue.queue[id_queue.front].timeout < time(NULL))) 
	{

		printf_d("pop id is = %d, pop_id_queue() succeed\n", id_queue.front);

		id_queue.front = (id_queue.front + 1) % MAX_QUEUE_SIZE;
	}
	return True;

}

Bool set_id_replied(unsigned short id) 
{
	if ((id_queue.front <= id_queue.rear && id < id_queue.rear && id >= id_queue.front)
		|| (id_queue.front > id_queue.rear && (id < id_queue.rear || id >= id_queue.front))
		|| (!id_queue.queue[id].is_replyed))
	{//?ж???????м?????????
		id_queue.queue[id].is_replyed = 1;
		return True;
	}
	printf_d("the message has arrived before??set_id_replied() failed\n");
	return False;
}

Bool get_id_entry(unsigned short id ,IdEntry* id_entry) 
{
	if ((id_queue.front <= id_queue.rear && id < id_queue.rear && id >= id_queue.front)
		|| (id_queue.front > id_queue.rear && (id < id_queue.rear || id >= id_queue.front)))
	{//?ж???????м?
		*id_entry = id_queue.queue[id];
		return True;
	}
	printf_d("the message of id = %d is not in queue ,get_id_entry()failed??\n" ,id);
	return False;
}