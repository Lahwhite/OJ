#include "api/leaderboard_api.hpp"
#include "leaderboard/stats_analyzer.hpp"
#include "leaderboard/leaderboard_service.hpp"
#include "storage/in_memory_cache.hpp"
#include "storage/in_memory_repository.hpp"

#include <cassert>
#include <iostream>
#include <memory>
#include <string>

namespace {

void test_stats_summary_empty() {
    const auto summary = oj::StatsAnalyzer::analyze_global({});
    assert(summary.total_users == 0);
    assert(summary.score_buckets.size() == 4);
    const auto json = oj::StatsAnalyzer::summary_to_json(summary);
    assert(json.find("\"total_users\":0") != std::string::npos);
}

void test_stats_summary_metrics() {
    std::vector<oj::LeaderboardRow> rows = {
        {1, "alice", 10, 20, 100, 3000, 1},
        {2, "bob", 8, 18, 200, 2500, 2},
        {3, "carol", 6, 15, 300, 1800, 3},
    };
    const auto summary = oj::StatsAnalyzer::analyze_global(rows);
    assert(summary.total_users == 3);
    assert(summary.max_score == 3000);
    assert(summary.min_score == 1800);
    assert(summary.median_score == 2500);
    assert(summary.top_score_gap == 500);
    assert(summary.average_solved > 7.0);
}

void test_stats_user_insight() {
    std::vector<oj::LeaderboardRow> rows = {
        {1, "alice", 10, 20, 100, 3000, 1},
        {2, "bob", 8, 18, 200, 2500, 2},
    };
    const auto insight = oj::StatsAnalyzer::analyze_user(rows, 2);
    assert(insight.has_value());
    assert(insight->rank == 2);
    assert(insight->score_gap_to_top == 500);
    assert(insight->score_percentile > 0.0);
}

void test_stats_csv_escape() {
    std::vector<oj::LeaderboardRow> rows = {
        {9, "name,with,comma", 1, 1, 0, 100, 0},
    };
    const auto csv = oj::StatsAnalyzer::rows_to_csv(rows, true);
    assert(csv.find("\"name,with,comma\"") != std::string::npos);
    assert(csv.find("rank,user_id") != std::string::npos);
}

void test_api_summary_json() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo->seed_users({
        {1, "alice", 5, 10, 50, 1200, 1},
        {2, "bob", 4, 9, 60, 900, 2},
    });
    oj::LeaderboardApi api(std::make_shared<oj::LeaderboardService>(repo, cache));
    const auto json = api.get_global_summary_json();
    assert(json.find("\"total_users\":2") != std::string::npos);
    assert(json.find("score_buckets") != std::string::npos);
}

void test_api_user_insight_json() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo->seed_users({
        {1, "alice", 5, 10, 50, 1200, 1},
        {2, "bob", 4, 9, 60, 900, 2},
    });
    oj::LeaderboardApi api(std::make_shared<oj::LeaderboardService>(repo, cache));
    const auto json = api.get_user_insight_json(2);
    assert(json.find("\"rank\":2") != std::string::npos);
    assert(json.find("score_percentile") != std::string::npos);
}

void test_api_csv_export() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo->seed_users({{3, "carol", 2, 3, 10, 500, 1}});
    oj::LeaderboardApi api(std::make_shared<oj::LeaderboardService>(repo, cache));
    const auto csv = api.get_global_leaderboard_csv(10, 0);
    assert(csv.find("carol") != std::string::npos);
    assert(csv.find("500") != std::string::npos);
}

void test_submission_score_weights() {
    auto repo2 = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo2->seed_users({{8, "tester", 0, 0, 0, 0, 0}});
    oj::LeaderboardService svc(repo2, cache);
    svc.on_submission({1, 8, 101, -1, oj::ProblemDifficulty::Easy, true, 1, 0});
    svc.on_submission({2, 8, 102, -1, oj::ProblemDifficulty::Medium, true, 2, 0});
    svc.on_submission({3, 8, 103, -1, oj::ProblemDifficulty::Hard, true, 3, 0});
    const auto rows = svc.get_global_leaderboard(10, 0);
    assert(rows.size() == 1);
    assert(rows[0].score == 100 + 220 + 400);
}

void test_global_json_contains_rank_field() {
    auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
    auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
    repo->seed_users({
        {1, "a", 1, 1, 0, 100, 0},
        {2, "b", 1, 1, 0, 90, 0},
    });
    oj::LeaderboardApi api(std::make_shared<oj::LeaderboardService>(repo, cache));
    const auto json = api.get_global_leaderboard_json(10, 5);
    assert(json.find("\"rank\":6") != std::string::npos || json.find("\"data\":[]") != std::string::npos);
}

}  // namespace

int main() {
    test_stats_summary_empty();
    test_stats_summary_metrics();
    test_stats_user_insight();
    test_stats_csv_escape();
    test_api_summary_json();
    test_api_user_insight_json();
    test_api_csv_export();
    test_submission_score_weights();
    test_global_json_contains_rank_field();
    std::cout << "extended tests passed\n";
    return 0;
}
