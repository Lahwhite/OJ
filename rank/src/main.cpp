#include "api/leaderboard_api.hpp"
#include "storage/in_memory_cache.hpp"
#include "storage/in_memory_repository.hpp"

#include <iostream>
#include <memory>

int main() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();

    repo->seed_users({
        {1, "alice", 12, 40, 1200, 2600, 1712000010},
        {2, "bob", 12, 38, 900, 2600, 1712000000},
        {3, "carol", 10, 25, 700, 2000, 1712000100},
    });
    repo->seed_problem_stats({
        {1, 12, 5, 4, 3},
        {2, 12, 6, 4, 2},
        {3, 10, 6, 3, 1},
    });
    repo->seed_contest_rows(1001, {
        {1001, 1, "alice", 4, 500, 900, 1712000010},
        {1001, 2, "bob", 4, 470, 900, 1712000000},
    });

    auto service = std::make_shared<oj::LeaderboardService>(repo, cache);
    oj::LeaderboardApi api(service);

    std::cout << "global=" << api.get_global_leaderboard_json(10, 0) << "\n";
    std::cout << "stats=" << api.get_problem_stats_json(2) << "\n";
    std::cout << "contest=" << api.get_contest_leaderboard_json(1001, 10, 0) << "\n";

    return 0;
}
