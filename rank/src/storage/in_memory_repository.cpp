#include "storage/in_memory_repository.hpp"

#include <string>

namespace oj {

namespace {

std::uint64_t make_user_problem_key(std::int64_t user_id, std::int64_t problem_id) {
    return (static_cast<std::uint64_t>(user_id) << 32U) ^ static_cast<std::uint64_t>(problem_id);
}

}  // namespace

void InMemoryLeaderboardRepository::seed_users(const std::vector<LeaderboardRow>& rows) {
    for (const auto& r : rows) {
        global_[r.user_id] = r;
    }
}

void InMemoryLeaderboardRepository::seed_problem_stats(const std::vector<ProblemCompletionStats>& stats) {
    for (const auto& s : stats) {
        problem_stats_[s.user_id] = s;
    }
}

void InMemoryLeaderboardRepository::seed_contest_rows(std::int64_t contest_id, const std::vector<ContestLeaderboardRow>& rows) {
    auto& contest = contests_[contest_id];
    for (const auto& r : rows) {
        contest[r.user_id] = r;
    }
}

std::vector<LeaderboardRow> InMemoryLeaderboardRepository::get_global_rows() {
    std::vector<LeaderboardRow> rows;
    rows.reserve(global_.size());
    for (const auto& [_, value] : global_) {
        rows.push_back(value);
    }
    return rows;
}

std::optional<ProblemCompletionStats> InMemoryLeaderboardRepository::get_problem_stats(std::int64_t user_id) {
    auto it = problem_stats_.find(user_id);
    if (it == problem_stats_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::vector<ContestLeaderboardRow> InMemoryLeaderboardRepository::get_contest_rows(std::int64_t contest_id) {
    std::vector<ContestLeaderboardRow> rows;
    auto it = contests_.find(contest_id);
    if (it == contests_.end()) {
        return rows;
    }
    rows.reserve(it->second.size());
    for (const auto& [_, value] : it->second) {
        rows.push_back(value);
    }
    return rows;
}

void InMemoryLeaderboardRepository::apply_submission_event(const SubmissionEvent& event, std::int64_t score_delta) {
    auto git = global_.find(event.user_id);
    if (git == global_.end()) {
        LeaderboardRow init;
        init.user_id = event.user_id;
        init.username = "user_" + std::to_string(event.user_id);
        git = global_.insert({event.user_id, init}).first;
    }

    git->second.total_submissions += 1;
    git->second.penalty_seconds += event.penalty_seconds;

    auto pit = problem_stats_.find(event.user_id);
    if (pit == problem_stats_.end()) {
        pit = problem_stats_.insert({event.user_id, empty_problem_stats(event.user_id)}).first;
    }

    if (event.is_accepted) {
        git->second.score += score_delta;
        git->second.last_accepted_at = event.submit_at;
        const auto key = make_user_problem_key(event.user_id, event.problem_id);
        if (accepted_problem_once_.insert(key).second) {
            git->second.solved_count += 1;
            pit->second.solved_total += 1;
            switch (event.difficulty) {
                case ProblemDifficulty::Easy:
                    pit->second.solved_easy += 1;
                    break;
                case ProblemDifficulty::Medium:
                    pit->second.solved_medium += 1;
                    break;
                case ProblemDifficulty::Hard:
                    pit->second.solved_hard += 1;
                    break;
                default:
                    break;
            }
        }
    }

    if (event.contest_id > 0) {
        auto& contest = contests_[event.contest_id];
        auto cit = contest.find(event.user_id);
        if (cit == contest.end()) {
            cit = contest.insert({event.user_id, empty_contest_row(event.contest_id, event.user_id)}).first;
            cit->second.username = git->second.username;
        }

        cit->second.penalty_seconds += event.penalty_seconds;
        if (event.is_accepted) {
            cit->second.score += score_delta;
            cit->second.last_accepted_at = event.submit_at;
            ContestAcceptedKey contest_key {event.contest_id, event.user_id, event.problem_id};
            if (accepted_contest_problem_once_.insert(contest_key).second) {
                cit->second.solved_count += 1;
            }
        }
    }
}

ProblemCompletionStats InMemoryLeaderboardRepository::empty_problem_stats(std::int64_t user_id) {
    ProblemCompletionStats v;
    v.user_id = user_id;
    return v;
}

ContestLeaderboardRow InMemoryLeaderboardRepository::empty_contest_row(std::int64_t contest_id, std::int64_t user_id) {
    ContestLeaderboardRow row;
    row.contest_id = contest_id;
    row.user_id = user_id;
    row.username = "user_" + std::to_string(user_id);
    return row;
}

}  // namespace oj
