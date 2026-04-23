#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace oj {

enum class ProblemDifficulty {
    Easy = 0,
    Medium = 1,
    Hard = 2
};

struct LeaderboardRow {
    std::int64_t user_id {};
    std::string username;
    std::int32_t solved_count {};
    std::int32_t total_submissions {};
    std::int64_t penalty_seconds {};
    std::int64_t score {};
    std::int64_t last_accepted_at {};
};

struct ProblemCompletionStats {
    std::int64_t user_id {};
    std::int32_t solved_total {};
    std::int32_t solved_easy {};
    std::int32_t solved_medium {};
    std::int32_t solved_hard {};
};

struct ContestLeaderboardRow {
    std::int64_t contest_id {};
    std::int64_t user_id {};
    std::string username;
    std::int32_t solved_count {};
    std::int64_t penalty_seconds {};
    std::int64_t score {};
    std::int64_t last_accepted_at {};
};

struct SubmissionEvent {
    std::int64_t submission_id {};
    std::int64_t user_id {};
    std::int64_t problem_id {};
    std::int64_t contest_id {-1};
    ProblemDifficulty difficulty {ProblemDifficulty::Easy};
    bool is_accepted {false};
    std::int64_t submit_at {};
    std::int64_t penalty_seconds {};
};

}  // namespace oj
