#include "leaderboard/repository.hpp"

#if defined(ENABLE_REDIS)
// 这里保留给 redis-plus-plus 的真实实现。
// 推荐结构：
// 1) global:zset 维护总榜
// 2) contest:{id}:zset 维护比赛榜
// 3) user:{id}:problem_stats 存题目通过统计
#endif
