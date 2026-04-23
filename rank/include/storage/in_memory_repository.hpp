#pragma once

#include "leaderboard/repository.hpp"

#include <unordered_map>
#include <unordered_set>

namespace oj {

class InMemoryLeaderboardRepository : public ILeaderboardRepository {
public:
    void seed_users(const std::vector<LeaderboardRow>& rows);
    void seed_problem_stats(const std::vector<ProblemCompletionStats>& stats);
    void seed_contest_rows(std::int64_t contest_id, const std::vector<ContestLeaderboardRow>& rows);

    std::vector<LeaderboardRow> get_global_rows() override;
    std::optional<ProblemCompletionStats> get_problem_stats(std::int64_t user_id) override;
    std::vector<ContestLeaderboardRow> get_contest_rows(std::int64_t contest_id) override;
    void apply_submission_event(const SubmissionEvent& event, std::int64_t score_delta) override;

private:
    static ProblemCompletionStats empty_problem_stats(std::int64_t user_id);
    static ContestLeaderboardRow empty_contest_row(std::int64_t contest_id, std::int64_t user_id);

    std::unordered_map<std::int64_t, LeaderboardRow> global_;
    std::unordered_map<std::int64_t, ProblemCompletionStats> problem_stats_;
    std::unordered_map<std::int64_t, std::unordered_map<std::int64_t, ContestLeaderboardRow>> contests_;
    std::unordered_set<std::uint64_t> accepted_problem_once_;
};

}  // namespace oj
