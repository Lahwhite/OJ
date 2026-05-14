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
    struct ContestAcceptedKey {
        std::int64_t contest_id {};
        std::int64_t user_id {};
        std::int64_t problem_id {};

        bool operator==(const ContestAcceptedKey& other) const {
            return contest_id == other.contest_id && user_id == other.user_id && problem_id == other.problem_id;
        }
    };

    struct ContestAcceptedKeyHash {
        std::size_t operator()(const ContestAcceptedKey& key) const {
            const auto h1 = std::hash<std::int64_t> {}(key.contest_id);
            const auto h2 = std::hash<std::int64_t> {}(key.user_id);
            const auto h3 = std::hash<std::int64_t> {}(key.problem_id);
            return (h1 * 1315423911U) ^ (h2 << 1U) ^ (h3 << 7U);
        }
    };

    static ProblemCompletionStats empty_problem_stats(std::int64_t user_id);
    static ContestLeaderboardRow empty_contest_row(std::int64_t contest_id, std::int64_t user_id);

    std::unordered_map<std::int64_t, LeaderboardRow> global_;
    std::unordered_map<std::int64_t, ProblemCompletionStats> problem_stats_;
    std::unordered_map<std::int64_t, std::unordered_map<std::int64_t, ContestLeaderboardRow>> contests_;
    std::unordered_set<std::uint64_t> accepted_problem_once_;
    std::unordered_set<ContestAcceptedKey, ContestAcceptedKeyHash> accepted_contest_problem_once_;
};

}  // namespace oj
