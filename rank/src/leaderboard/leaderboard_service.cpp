#include "leaderboard/leaderboard_service.hpp"

#include <algorithm>

namespace oj {

LeaderboardService::LeaderboardService(
    std::shared_ptr<ILeaderboardRepository> repository,
    std::shared_ptr<ILeaderboardCache> cache,
    LeaderboardConfig config)
    : repository_(std::move(repository)), cache_(std::move(cache)), config_(config) {}

std::vector<LeaderboardRow> LeaderboardService::get_global_leaderboard(std::int32_t limit, std::int32_t offset) {
    if (auto cached = cache_->get_global_rows(limit, offset); cached.has_value()) {
        return cached.value();
    }

    auto rows = repository_->get_global_rows();
    sort_global(rows);
    auto sliced = paginate(rows, limit, offset);
    cache_->set_global_rows(sliced, config_.global_cache_ttl_seconds);
    return sliced;
}

std::optional<ProblemCompletionStats> LeaderboardService::get_problem_completion_stats(std::int64_t user_id) {
    return repository_->get_problem_stats(user_id);
}

std::vector<ContestLeaderboardRow> LeaderboardService::get_contest_leaderboard(
    std::int64_t contest_id,
    std::int32_t limit,
    std::int32_t offset) {
    if (auto cached = cache_->get_contest_rows(contest_id, limit, offset); cached.has_value()) {
        return cached.value();
    }

    auto rows = repository_->get_contest_rows(contest_id);
    sort_contest(rows);
    auto sliced = paginate(rows, limit, offset);
    cache_->set_contest_rows(contest_id, sliced, config_.contest_cache_ttl_seconds);
    return sliced;
}

void LeaderboardService::on_submission(const SubmissionEvent& event) {
    const std::int64_t delta = calc_score_delta(event);
    repository_->apply_submission_event(event, delta);
    cache_->invalidate_global_rows();
    if (event.contest_id > 0) {
        cache_->invalidate_contest_rows(event.contest_id);
    }
}

void LeaderboardService::rebuild_global_cache() {
    auto rows = repository_->get_global_rows();
    sort_global(rows);
    cache_->set_global_rows(rows, config_.global_cache_ttl_seconds);
}

std::int64_t LeaderboardService::calc_score_delta(const SubmissionEvent& event) const {
    if (!event.is_accepted) {
        return 0;
    }

    // 排名优化：不同难度给不同权重，鼓励高质量解题。
    switch (event.difficulty) {
        case ProblemDifficulty::Easy:
            return 100;
        case ProblemDifficulty::Medium:
            return 220;
        case ProblemDifficulty::Hard:
            return 400;
        default:
            return 100;
    }
}

void LeaderboardService::sort_global(std::vector<LeaderboardRow>& rows) {
    std::sort(rows.begin(), rows.end(), [](const LeaderboardRow& a, const LeaderboardRow& b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        if (a.solved_count != b.solved_count) {
            return a.solved_count > b.solved_count;
        }
        if (a.penalty_seconds != b.penalty_seconds) {
            return a.penalty_seconds < b.penalty_seconds;
        }
        return a.last_accepted_at < b.last_accepted_at;
    });
}

void LeaderboardService::sort_contest(std::vector<ContestLeaderboardRow>& rows) {
    std::sort(rows.begin(), rows.end(), [](const ContestLeaderboardRow& a, const ContestLeaderboardRow& b) {
        if (a.score != b.score) {
            return a.score > b.score;
        }
        if (a.solved_count != b.solved_count) {
            return a.solved_count > b.solved_count;
        }
        if (a.penalty_seconds != b.penalty_seconds) {
            return a.penalty_seconds < b.penalty_seconds;
        }
        return a.last_accepted_at < b.last_accepted_at;
    });
}

template <typename T>
std::vector<T> LeaderboardService::paginate(const std::vector<T>& rows, std::int32_t limit, std::int32_t offset) {
    if (offset < 0) {
        offset = 0;
    }
    if (limit <= 0) {
        limit = static_cast<std::int32_t>(rows.size());
    }

    const auto begin_idx = static_cast<std::size_t>(std::min<std::int32_t>(offset, static_cast<std::int32_t>(rows.size())));
    const auto end_idx = static_cast<std::size_t>(
        std::min<std::int32_t>(offset + limit, static_cast<std::int32_t>(rows.size())));

    return std::vector<T>(rows.begin() + static_cast<std::ptrdiff_t>(begin_idx), rows.begin() + static_cast<std::ptrdiff_t>(end_idx));
}

template std::vector<LeaderboardRow> LeaderboardService::paginate(
    const std::vector<LeaderboardRow>&,
    std::int32_t,
    std::int32_t);
template std::vector<ContestLeaderboardRow> LeaderboardService::paginate(
    const std::vector<ContestLeaderboardRow>&,
    std::int32_t,
    std::int32_t);

}  // namespace oj
