#include "leaderboard_isolated/leaderboard.hpp"

#include <iostream>

int main() {
    isolated_lb::Leaderboard board;

    board.apply_submission({1, 101, "alice", 1001, -1, isolated_lb::Difficulty::Easy, true, 100, 20});
    board.apply_submission({2, 102, "bob", 1001, -1, isolated_lb::Difficulty::Easy, true, 120, 10});
    board.apply_submission({3, 101, "alice", 1002, 9001, isolated_lb::Difficulty::Hard, true, 200, 40});
    board.apply_submission({4, 102, "bob", 1002, 9001, isolated_lb::Difficulty::Medium, true, 220, 30});

    std::cout << board.get_global_json(10, 0) << "\n";
    std::cout << board.get_contest_json(9001, 10, 0) << "\n";
    std::cout << board.get_problem_stats_json(101) << "\n";

    return 0;
}
