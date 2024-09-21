#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#define ARMS 2
#define SIZE 100000
#define BLOCK 40

int f; 
// Node for doubly-linked list
struct Node
{
	int blockNo;
	int time;
	struct Node *next;
	struct Node *prev;
};

struct record
{
	int miss;
	int hit;
};
// Struct for cache
struct Cache
{
	int hits;
	int misses;
	int reads;
	int writes;
	int cache_size;
	int block_size;
	int numLines;
	int write_policy;
	int curr_size;
	struct record *arms;
	struct Node *blocks;
};

struct Cache *cache; // global cache
struct Node *head;	 // global head of list
struct Node *tail;	 // global tail of list

// TODO
int ETC(int t)
{
	if (f) return f;
	int re = 0;
	if (t <= sqrt(SIZE))
	{
		re = t % (ARMS * BLOCK);
		re /= BLOCK;
	}
	else
	{
		for (int i = 1;i < ARMS;i ++)
		{
			if (cache->arms[i].hit > cache->arms[re].hit) 
				re = i;
		}
		f = re;
	}

	return re;
}

// Created a new node with data x
struct Node *newNode(int blockNo)
{
	struct Node *newNode = (struct Node *)malloc(sizeof(struct Node));
	newNode->blockNo = blockNo;
	newNode->prev = NULL;
	newNode->next = NULL;
	return newNode;
}

// removes last node from tail
void removeFromTail()
{
	if (head == tail) 
	{
		head = NULL;
		tail = NULL;
	}
	else
	{
		struct Node *tmp = tail->prev;
		tmp->next = NULL;
		free(tail);
		tail = tmp;
	}

}
void removeFromCache(struct Node *tmp)
{
	tmp->prev->next = tmp->next;
	tmp->next->prev = tmp->prev;
	free(tmp);
}

// Inserts a Node with data x at the head of the list
void insertAtHead(struct Node *nodeToInsert)
{
	if (head == NULL)
	{
		head = nodeToInsert;
		tail = nodeToInsert;
	}
	else
	{
		head->prev = nodeToInsert;
		nodeToInsert->next = head;
		head = nodeToInsert;
	}
	nodeToInsert->time = cache->reads;
}

// moves node blockNo to head
void bringToHead(struct Node *toHead)
{
	toHead->prev->next = toHead->next;
	if (toHead->next)
	{
		toHead->next->prev = toHead->prev;
	}
	toHead->next = NULL;
	toHead->prev = NULL;
	insertAtHead(toHead);
}

struct Node *findNode(int blockNo)
{
	struct Node *tmp = head;
	while (tmp != NULL)
	{
		if (tmp->blockNo == blockNo)
		{
			return tmp;
		}
		tmp = tmp->next;
	}
	return NULL;
}

void writeToCache(int blockNo)
{
	struct Node *toInsert = newNode(blockNo);
	// if cache is at limit size, evict LRU block
	if (cache->curr_size == cache->cache_size)
	{
		cache->curr_size--;
		removeFromTail();
	}
	insertAtHead(toInsert);
	cache->curr_size++;
}

struct Node *readFromCache(int blockNo)
{
	struct Node *toRead = findNode(blockNo);
	int che = ETC(cache->reads);
	if (che == 0)
	{
		if (toRead == NULL)
		{
			// block to read isn't in cache, get it from memory
			cache->misses++;
			writeToCache(blockNo);
			cache->writes++;
			cache->reads++;
			cache->arms[che].miss++;
		}
		else
		{
			// block to read is in cache
			cache->hits++;
			if (toRead != head) bringToHead(toRead); 
			cache->reads++;
			cache->arms[che].hit++;
		}


	}
	else
	{
		// TODO
		if (toRead == NULL)
		{
			struct Node *tmp = head;
			struct Node *re = head;
			while (tmp != NULL)
			{
				if (tmp->time < re->time) re = tmp; 
				tmp = tmp->next;
			}

			cache->misses++;
			if (toRead == NULL) ;
			else if (toRead == tail) removeFromTail(); 
			else removeFromCache(tmp);
			writeToCache(blockNo);
			cache->writes++;
			cache->reads++;
			cache->arms[che].miss++;

		}
		else
		{
			// block to read is in cache
			cache->hits++;
			if (toRead != head) bringToHead(toRead);
			cache->reads++;
			cache->arms[che].hit++;
		}
	}
	/*
	
	*/
	return head;
}

// resets the cache
void resetCache()
{
	f = 0;
	struct Node *tmp1 = head;
	struct Node *tmp2;
	while (tmp1 != NULL)
	{
		tmp2 = tmp1;
		tmp1 = tmp1->next;
		free(tmp2);
	}
	head = NULL;
	tail = NULL;
	cache->hits = 0;
	cache->misses = 0;
	cache->reads = 0;
	cache->writes = 0;
	cache->curr_size = 0;
	cache->blocks = head;
	for (int i=0;i<ARMS;i++) 
	{
		cache->arms[i].miss = 0;
		cache->arms[i].hit = 0;
	}
}
int init()
{
	head = NULL;
	tail = NULL;
	cache = (struct Cache *)malloc(sizeof(struct Cache));
	cache->hits = 0;
	cache->misses = 0;
	cache->reads = 0;
	cache->writes = 0;
	cache->cache_size = 1000;
	cache->curr_size = 0;
	cache->blocks = head;
	f = 0;
	cache->arms = (struct record *)malloc(sizeof(struct record) * ARMS);
	for (int i=0;i<ARMS;i++) 
	{
		cache->arms[i].miss = 0;
		cache->arms[i].hit = 0;
	}
}
void test1()
{
	// sequential reads, all misses;
	int i = 0;

	for (i = 0; i < SIZE; i++)
	{
		readFromCache(i);
	}
	printf("All done, cache misses: %d, cache hits: %d, cache reads: %d, cache writes: %d \n", cache->misses, cache->hits, cache->reads, cache->writes);

	resetCache();
	return;
}
void test2()
{
	int i = 0;
	// looping size of cache, cache_size misses, all rest hits
	for (i = 0; i < SIZE; i++)
	{
		readFromCache(i % cache->cache_size);
	}

	printf("All done, cache misses: %d, cache hits: %d, cache reads: %d, cache writes: %d \n", cache->misses, cache->hits, cache->reads, cache->writes);

	resetCache();
	return;
}
void test3()
{
	// random reads
	srand(time(NULL));
	int maxBlockNo = rand() * rand() % 1145;
	for (int i = 0; i < SIZE; i++)
		readFromCache(rand() % SIZE);

	printf("random reads \n");
	printf("All done, cache misses: %d, cache hits: %d, cache reads: %d, cache writes: %d \n\n", cache->misses, cache->hits, cache->reads, cache->writes);
	printf("chosen arm : %d\n",f);
	resetCache();
	return;
}
int main()
{
	init();

	test1();
	test2();
	test3();

	return 0;
}
