#pragma once

#include "leaderboard/models.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace oj {

struct ScoreBucket {
    std::string label;
    std::int64_t min_score {};
    std::int64_t max_score {};
    std::int32_t user_count {};
};

struct GlobalLeaderboardSummary {
    std::int32_t total_users {};
    std::int64_t max_score {};
    std::int64_t min_score {};
    double average_score {};
    double average_solved {};
    double average_penalty {};
    std::int64_t median_score {};
    std::int64_t top_score_gap {};
    std::vector<ScoreBucket> score_buckets;
};

struct UserRankInsight {
    std::int64_t user_id {};
    std::string username;
    std::int64_t rank {};
    std::int64_t score {};
    std::int32_t solved_count {};
    double score_percentile {};
    std::int64_t score_gap_to_top {};
};

class StatsAnalyzer {
public:
    static GlobalLeaderboardSummary analyze_global(const std::vector<LeaderboardRow>& rows);
    static std::optional<UserRankInsight> analyze_user(const std::vector<LeaderboardRow>& rows, std::int64_t user_id);

    static std::string summary_to_json(const GlobalLeaderboardSummary& summary);
    static std::string insight_to_json(const UserRankInsight& insight);
    static std::string rows_to_csv(const std::vector<LeaderboardRow>& rows, bool include_rank);

private:
    static std::int64_t median_score_of(const std::vector<LeaderboardRow>& rows);
    static std::vector<ScoreBucket> build_default_buckets(const std::vector<LeaderboardRow>& rows);
    static std::string csv_escape(const std::string& value);
};

}  // namespace oj
