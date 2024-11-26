# mlcache:利用ucb算法挑选页面驱逐方案
本项目选取的赋分机制为
$$Reward = \hat{\mu}_i(t-1) + \sqrt{\frac{2 \log f(t)}{T_i(t-1)}}$$
其中$f(t) = 1 + t \log^2(t)$
待选臂有

+ 0: lfu
+ 1: lru
+ 2: fifo

其中每五十次页面寻找作为一次$t$ 