#pragma once

#include "leaderboard/repository.hpp"

#include <memory>

namespace oj {

struct LeaderboardConfig {
    int global_cache_ttl_seconds {20};
    int contest_cache_ttl_seconds {10};
};

class LeaderboardService {
public:
    LeaderboardService(
        std::shared_ptr<ILeaderboardRepository> repository,
        std::shared_ptr<ILeaderboardCache> cache,
        LeaderboardConfig config = {});

    std::vector<LeaderboardRow> get_global_leaderboard(std::int32_t limit, std::int32_t offset);
    std::optional<ProblemCompletionStats> get_problem_completion_stats(std::int64_t user_id);
    std::vector<ContestLeaderboardRow> get_contest_leaderboard(std::int64_t contest_id, std::int32_t limit, std::int32_t offset);

    void on_submission(const SubmissionEvent& event);
    void rebuild_global_cache();

private:
    std::int64_t calc_score_delta(const SubmissionEvent& event) const;

    static void sort_global(std::vector<LeaderboardRow>& rows);
    static void sort_contest(std::vector<ContestLeaderboardRow>& rows);

    template <typename T>
    static std::vector<T> paginate(const std::vector<T>& rows, std::int32_t limit, std::int32_t offset);

    std::shared_ptr<ILeaderboardRepository> repository_;
    std::shared_ptr<ILeaderboardCache> cache_;
    LeaderboardConfig config_;
};

template <typename T>
std::vector<T> LeaderboardService::paginate(const std::vector<T>& rows, std::int32_t limit, std::int32_t offset) {
    if (limit <= 0 || offset < 0 || rows.empty()) {
        return {};
    }

    const auto begin_index = static_cast<std::size_t>(offset);
    if (begin_index >= rows.size()) {
        return {};
    }

    const auto end_index = std::min(rows.size(), begin_index + static_cast<std::size_t>(limit));
    return std::vector<T>(rows.begin() + static_cast<std::ptrdiff_t>(begin_index), rows.begin() + static_cast<std::ptrdiff_t>(end_index));
}

}  // namespace oj
