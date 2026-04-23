#include "leaderboard/leaderboard_service.hpp"
#include "storage/in_memory_cache.hpp"
#include "storage/in_memory_repository.hpp"

#include <cassert>
#include <iostream>
#include <memory>

namespace {

void test_global_rank_with_tiebreak() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo->seed_users({
        {1, "alice", 10, 30, 1100, 2000, 100},
        {2, "bob", 10, 28, 800, 2000, 90},
        {3, "carol", 9, 20, 600, 1800, 80},
    });

    oj::LeaderboardService service(repo, cache);
    auto rows = service.get_global_leaderboard(10, 0);
    assert(rows.size() == 3);
    assert(rows[0].user_id == 2);
    assert(rows[1].user_id == 1);
}

void test_problem_stats_update() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo->seed_users({{1, "alice", 0, 0, 0, 0, 0}});
    repo->seed_problem_stats({{1, 0, 0, 0, 0}});

    oj::LeaderboardService service(repo, cache);
    service.on_submission({100, 1, 2001, -1, oj::ProblemDifficulty::Hard, true, 120, 30});

    const auto stats = service.get_problem_completion_stats(1);
    assert(stats.has_value());
    assert(stats->solved_total == 1);
    assert(stats->solved_hard == 1);
}

void test_contest_rank() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo->seed_contest_rows(99, {
        {99, 1, "alice", 3, 500, 700, 200},
        {99, 2, "bob", 3, 450, 700, 180},
    });

    oj::LeaderboardService service(repo, cache);
    auto rows = service.get_contest_leaderboard(99, 10, 0);
    assert(rows.size() == 2);
    assert(rows[0].user_id == 2);
}

}  // namespace

int main() {
    test_global_rank_with_tiebreak();
    test_problem_stats_update();
    test_contest_rank();
    std::cout << "all tests passed\n";
    return 0;
}
