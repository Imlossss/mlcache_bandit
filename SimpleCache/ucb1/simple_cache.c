#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>

#include "simple_cache.h"
#include "ucb1.h"
struct Cache *cache; // global cache
struct Node *head;	 // global head of list
struct Node *tail;	 // global tail of list

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
	struct Node *tmp = tail->prev;
	tmp->next = NULL;
	free(tail);
	tail = tmp;
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

int findBlock(int blockNo)
{
	int i = 0;
	for (i = 0; i < cache->curr_size; i++)
	{
		if (blockNo == cache->blocks_array[i])
		{
			return i;
		}
	}
	return -1;
}

int findEmptyLine()
{
	int i = 0;
	for (i = 0; i < cache->curr_size; i++)
	{
		if (cache->blocks_array[i] == -1)
		{
			return i;
		}
	}
}

void insertUCB(int blockNo, int idx)
{
	cache->blocks_array[idx] = blockNo;
}

int removeFromCacheUCB()
{
	// pull arm, and find block referenced by chosen action
	int idx = findBlock(pull(cache->theUCB, cache));
	cache->blocks_array[idx] = -1;
	return idx;
}

void writeToCacheUCB(int blockNo)
{
	// if cache is at limit size, evict block with lowest ucb
	int toInsert = -1;
	if (cache->curr_size == CACHE_SIZE)
	{
		toInsert = removeFromCacheUCB();
		cache->curr_size--;
	}
	if (toInsert == -1)
	{
		toInsert = findEmptyLine();
	}
	insertUCB(blockNo, toInsert);
	cache->curr_size++;
}

int readFromCacheUCB(int blockNo)
{
	int toRead = findBlock(blockNo);
	if (toRead == -1)
	{
		// block to read isn't in cache, get it from memory
		// penalize blocks in cache
		updateInCache(-1, cache);

		cache->misses++;

		writeToCacheUCB(blockNo);
		cache->writes++;
	}
	else
	{
		// block to read is in cache
		// reward referenced block, penalize others
		updateInCache(cache->blocks_array[toRead], cache);

		cache->hits++;
	}
	cache->reads++;
	return toRead;
}

void resetCache()
{
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

	if (!cache->theUCB->numPlays)
		free(cache->theUCB->numPlays);

	if (!cache->theUCB->ucbs)
		free(cache->theUCB->ucbs);

	if (!cache->theUCB->weights)
		free(cache->theUCB->weights);

	free(cache->theUCB);
}
void activateUCB(int maxBlockNo)
{
	// 我们假设编号为 i 的 page 位于编号为 i % ADDRESS_SPACE 的进程树下
	cache->theUCB = (struct UCB_struct *)malloc(sizeof(struct UCB_struct));
	cache->theUCB->numPlays = (int *)malloc(maxBlockNo * sizeof(int));
	cache->theUCB->ucbs = (int *)malloc(maxBlockNo * sizeof(int));
	cache->theUCB->weights = (int *)malloc(ADDRESS_SPACE * sizeof(int));
	cache->theUCB->t = 0;

	for (int i = 0; i < ADDRESS_SPACE; i++)
		cache->theUCB->weights[i] = 0;

	for (int i = 0; i < maxBlockNo; i++)
	{
		cache->theUCB->numPlays[i] = 0;
		cache->theUCB->ucbs[i] = 0;
	}
	for (int i = 0; i < CACHE_SIZE; i++)
		cache->blocks_array[i] = -1;

	return;
}
void test1()
{
	// sequential reads, all misses;
	int maxBlockNo = 10000;
	activateUCB(maxBlockNo);

	for (int i = 0; i < maxBlockNo; i++)
		readFromCacheUCB(i);

	printf("sequential reads \n");
	printf("All done, cache misses: %d, cache hits: %d, cache reads: %d, cache writes: %d \n\n", cache->misses, cache->hits, cache->reads, cache->writes);

	resetCache();
	return;
}

void test2()
{
	// looping size of cache, cache_size misses, all rest hits
	activateUCB(CACHE_SIZE);

	for (int i = 0; i < 10000; i++)
		readFromCacheUCB(i % cache->cache_size);

	printf("looping size of cache \n");
	printf("All done, cache misses: %d, cache hits: %d, cache reads: %d, cache writes: %d \n\n", cache->misses, cache->hits, cache->reads, cache->writes);

	resetCache();
	return;
}
void test3()
{
	// random reads
	srand(time(NULL));
	int maxBlockNo = rand() * rand() % 1145;
	activateUCB(maxBlockNo);
	for (int i = 0; i < 10000; i++)
		readFromCacheUCB(rand() % maxBlockNo);

	printf("random reads \n");
	printf("All done, cache misses: %d, cache hits: %d, cache reads: %d, cache writes: %d \n\n", cache->misses, cache->hits, cache->reads, cache->writes);

	resetCache();
	return;
}
int main()
{
	head = NULL;
	tail = NULL;
	cache = (struct Cache *)malloc(sizeof(struct Cache));
	cache->hits = 0;
	cache->misses = 0;
	cache->reads = 0;
	cache->writes = 0;
	cache->cache_size = CACHE_SIZE;
	cache->curr_size = 0;
	cache->blocks = head;

	test1();
	test2();
	test3();

	return 0;
}
