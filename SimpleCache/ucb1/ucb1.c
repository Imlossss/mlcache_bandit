
#include "ucb1.h"

int integerSqrt(int n)
{
	int smallCandidate;
	int largeCandidate;
	if (n < 2)
	{
		return n;
	}
	else
	{
		smallCandidate = integerSqrt(n >> 2) << 1;
		largeCandidate = smallCandidate + 1;
		if (largeCandidate * largeCandidate > n)
		{
			return smallCandidate;
		}
		else
		{
			return largeCandidate;
		}
	}
}

// TODO: finish this
//  find the log base 2 of 32-bit v
// might need to scale v up
int integerLog(uint32_t v)
{
	int r; // result goes here

	static const int MultiplyDeBruijnBitPosition[32] =
		{
			0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
			8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31};

	v |= v >> 1; // first round down to one less than a power of 2
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;

	return MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
}

int upperBound(int step, int numPlays)
{
	// indexing from 0
	if (step != 0 && numPlays != 0)
		return integerSqrt(SCALEUP * SCALEUP * (2 * integerLog(step + 1) / numPlays));
	return 0;
}

// TODO: this should take into account some kind of "popularity"
// for the page throughout the learning stage
int updateUCBscores(int choice, int hit, struct Cache *cache)
{
	if (hit == -1)
	{
		int address_space_number = choice % ADDRESS_SPACE;
		cache->theUCB->ucbs[choice] = cache->theUCB->weights[address_space_number];
	}
	else
	{
		cache->theUCB->ucbs[choice] -= BY;
		cache->theUCB->ucbs[choice] += upperBound(cache->theUCB->t - 1, cache->theUCB->numPlays[choice]) - upperBound(cache->theUCB->t, cache->theUCB->numPlays[choice]) * cache->theUCB->numPlays[choice];
	}
}

int pull(struct UCB_struct *ucb, struct Cache *cache)
{
	int action = -1;
	// get action that maximizes gain.
	// if training, just use best action overall
	int i = 0;
	if (cache == NULL)
	{
		action = 0;
		for (i = 0; i < ucb->numActions; i++)
		{
			if (ucb->ucbs[i] > ucb->ucbs[action])
			{
				action = i;
			}
		}
		// else use best action in cache
	}
	else
	{
		action = cache->blocks_array[0];
		for (i = 0; i < cache->cache_size; i++)
		{
			if (cache->theUCB->ucbs[cache->blocks_array[i]] > cache->theUCB->ucbs[action])
			{
				action = cache->blocks_array[i];
			}
		}
		ucb->t += SCALEUP;
	}

	ucb->ucbs[action] = updateUCBscores(action, 1, cache);
	ucb->numPlays[action] += SCALEUP;
	return action;
}

// TODO
void getWeightAverage(struct Cache *cache)
{
	int *cnt = (int *)malloc(ADDRESS_SPACE * sizeof(int));
	for (int i = 0; i < ADDRESS_SPACE; i++)
		cnt[i] = 0;

	for (int i = 0; i < ADDRESS_SPACE; i++)
		cache->theUCB->weights[i] = 0;

	for (int i = 0; i < cache->cache_size; i++)
	{
		if (cache->blocks_array[i] != -1)
		{
			int tempBlockNo = cache->blocks_array[i];
			int address_space_number = tempBlockNo % ADDRESS_SPACE;
			cache->theUCB->weights[address_space_number] += cache->theUCB->ucbs[i];
			cnt[address_space_number]++;
		}
	}
	if (cache->curr_size > 0)
		for (int i = 0; i < ADDRESS_SPACE; i++)
			if (cnt[i] == 0)
				cache->theUCB->weights[i] = 0;
			else
				cache->theUCB->weights[i] = cache->theUCB->weights[i] / cnt[i];

	free(cnt);
	return;
}

// This function should be called after a cache hit, it does two things:
// decrease weight of blocks in cache that weren't referenced
// increase weight of block in cache that was referenced
void updateInCache(int actionToReward, struct Cache *cache)
{
	int i = 0;
	getWeightAverage(cache);
	int address_space_number = actionToReward % ADDRESS_SPACE;
	for (i = 0; i < cache->cache_size; i++)
	{
		int cacheBlock = cache->blocks_array[i];
		if (cacheBlock == -1)
			continue;
		if (actionToReward != cacheBlock)
		{
			if (address_space_number == cacheBlock % ADDRESS_SPACE)
				// cache->theUCB->payoffSums[cacheBlock] += reward(cacheBlock, cache->theUCB->t, -1);
				cache->theUCB->ucbs[cacheBlock] = updateUCBscores(cacheBlock, -1, cache);
		}
		else
		{
			// cache->theUCB->payoffSums[actionToReward] += reward(actionToReward, cache->theUCB->t, 1);
			cache->theUCB->ucbs[cacheBlock] = updateUCBscores(actionToReward, 1, cache);
		}
	}
}
