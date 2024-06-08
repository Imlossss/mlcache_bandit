#ifndef _ucb1
#define _ucb1

#include <stdio.h>
#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
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

void updateInCache(int actionToReward, struct Cache *cache);

int pull(struct UCB_struct *ucb, struct Cache *cache);

void updateUCB(struct UCB_struct *ucb);

void getWeightAverage(struct Cache *cache);

// #include "ucb1.c"

#endif
