#pragma once

#include "leaderboard/repository.hpp"

#include <memory>

namespace oj {

class MysqlLeaderboardRepository : public ILeaderboardRepository {
public:
    std::vector<LeaderboardRow> get_global_rows() override;
    std::optional<ProblemCompletionStats> get_problem_stats(std::int64_t user_id) override;
    std::vector<ContestLeaderboardRow> get_contest_rows(std::int64_t contest_id) override;
    void apply_submission_event(const SubmissionEvent& event, std::int64_t score_delta) override;

    static bool database_ready();
};

}  // namespace oj
