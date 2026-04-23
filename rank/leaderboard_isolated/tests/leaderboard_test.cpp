#include "leaderboard_isolated/leaderboard.hpp"

#include <cassert>
#include <iostream>

namespace {

void test_global_rank_sorting() {
    isolated_lb::Leaderboard board;
    board.apply_submission({1, 1, "alice", 11, -1, isolated_lb::Difficulty::Easy, true, 110, 30});
    board.apply_submission({2, 2, "bob", 12, -1, isolated_lb::Difficulty::Easy, true, 100, 20});

    const auto rows = board.get_global(10, 0);
    assert(rows.size() == 2);
    assert(rows[0].user_id == 2);
    assert(rows[1].user_id == 1);
}

void test_accept_same_problem_once_global() {
    isolated_lb::Leaderboard board;
    board.apply_submission({1, 7, "dave", 3001, -1, isolated_lb::Difficulty::Hard, true, 100, 10});
    board.apply_submission({2, 7, "dave", 3001, -1, isolated_lb::Difficulty::Hard, true, 120, 10});

    const auto rows = board.get_global(10, 0);
    assert(rows.size() == 1);
    assert(rows[0].solved_count == 1);

    const auto stats = board.get_problem_stats(7);
    assert(stats.has_value());
    assert(stats->solved_total == 1);
    assert(stats->solved_hard == 1);
}

void test_accept_same_problem_once_contest() {
    isolated_lb::Leaderboard board;
    board.apply_submission({1, 10, "eva", 4001, 88, isolated_lb::Difficulty::Medium, true, 100, 5});
    board.apply_submission({2, 10, "eva", 4001, 88, isolated_lb::Difficulty::Medium, true, 120, 5});

    const auto contest_rows = board.get_contest(88, 10, 0);
    assert(contest_rows.size() == 1);
    assert(contest_rows[0].solved_count == 1);
}

void test_pagination() {
    isolated_lb::Leaderboard board;
    board.apply_submission({1, 1, "u1", 1, -1, isolated_lb::Difficulty::Easy, true, 100, 10});
    board.apply_submission({2, 2, "u2", 2, -1, isolated_lb::Difficulty::Easy, true, 100, 10});
    board.apply_submission({3, 3, "u3", 3, -1, isolated_lb::Difficulty::Easy, true, 100, 10});

    const auto page2 = board.get_global(1, 1);
    assert(page2.size() == 1);
    assert(page2[0].user_id == 2);
}

}  // namespace

int main() {
    test_global_rank_sorting();
    test_accept_same_problem_once_global();
    test_accept_same_problem_once_contest();
    test_pagination();
    std::cout << "leaderboard_isolated tests passed\n";
    return 0;
}
