#ifndef _ucb1
#define _ucb1

#include "simple_cache.h"

#define SCALEUP 100
#define BY 50

struct UCB_struct
{
	int numActions;
	int trials;
	int t;
	int *weights;
	int *numPlays;
	int *ucbs;
};

int updateUCBscores(int choice, int hit, struct Cache *cache);

// struct UCB_struct *ucb1(int numActions, int trials, struct Cache *cache /*might want to pass function pointer for reward in future*/);

void updateInCache(int actionToReward, struct Cache *cache);

int pull(struct UCB_struct *ucb, struct Cache *cache);

void updateUCB(struct UCB_struct *ucb);

void getWeightAverage(struct Cache *cache);

// void updateUCBinCache(struct Cache *cache, int BlockNo, int by);

#include "ucb1.c"

#endif
