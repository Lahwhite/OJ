#pragma once

#include "leaderboard/models.hpp"

#include <optional>
#include <vector>

namespace oj {

class ILeaderboardRepository {
public:
    virtual ~ILeaderboardRepository() = default;

    virtual std::vector<LeaderboardRow> get_global_rows() = 0;
    virtual std::optional<ProblemCompletionStats> get_problem_stats(std::int64_t user_id) = 0;
    virtual std::vector<ContestLeaderboardRow> get_contest_rows(std::int64_t contest_id) = 0;
    virtual void apply_submission_event(const SubmissionEvent& event, std::int64_t score_delta) = 0;
};

class ILeaderboardCache {
public:
    virtual ~ILeaderboardCache() = default;

    virtual std::optional<std::vector<LeaderboardRow>> get_global_rows(std::int32_t limit, std::int32_t offset) = 0;
    virtual void set_global_rows(const std::vector<LeaderboardRow>& rows, int ttl_seconds) = 0;
    virtual void invalidate_global_rows() = 0;

    virtual std::optional<std::vector<ContestLeaderboardRow>> get_contest_rows(
        std::int64_t contest_id,
        std::int32_t limit,
        std::int32_t offset) = 0;
    virtual void set_contest_rows(std::int64_t contest_id, const std::vector<ContestLeaderboardRow>& rows, int ttl_seconds) = 0;
    virtual void invalidate_contest_rows(std::int64_t contest_id) = 0;
};

}  // namespace oj
