#include "leaderboard_isolated/leaderboard.hpp"

#include <algorithm>
#include <sstream>

namespace isolated_lb {

namespace {

std::uint64_t make_user_problem_key(std::int64_t user_id, std::int64_t problem_id) {
    return (static_cast<std::uint64_t>(user_id) << 32U) ^ static_cast<std::uint64_t>(problem_id);
}

}  // namespace

bool Leaderboard::ContestProblemKey::operator==(const ContestProblemKey& other) const {
    return contest_id == other.contest_id && user_id == other.user_id && problem_id == other.problem_id;
}

std::size_t Leaderboard::ContestProblemKeyHash::operator()(const ContestProblemKey& key) const {
    const auto h1 = std::hash<std::int64_t> {}(key.contest_id);
    const auto h2 = std::hash<std::int64_t> {}(key.user_id);
    const auto h3 = std::hash<std::int64_t> {}(key.problem_id);
    return (h1 * 1315423911U) ^ (h2 << 1U) ^ (h3 << 7U);
}

void Leaderboard::apply_submission(const SubmissionEvent& event) {
    auto git = global_rows_.find(event.user_id);
    if (git == global_rows_.end()) {
        GlobalRow init;
        init.user_id = event.user_id;
        init.username = event.username.empty() ? ("user_" + std::to_string(event.user_id)) : event.username;
        git = global_rows_.insert({event.user_id, init}).first;
    } else if (!event.username.empty()) {
        git->second.username = event.username;
    }

    git->second.total_submissions += 1;
    git->second.penalty_seconds += event.penalty_seconds;

    auto pit = problem_stats_.find(event.user_id);
    if (pit == problem_stats_.end()) {
        ProblemStats s;
        s.user_id = event.user_id;
        pit = problem_stats_.insert({event.user_id, s}).first;
    }

    if (event.accepted) {
        git->second.score += score_delta(event.difficulty, true);
        git->second.last_accepted_at = event.submit_at;

        const auto key = make_user_problem_key(event.user_id, event.problem_id);
        if (accepted_problem_once_.insert(key).second) {
            git->second.solved_count += 1;
            pit->second.solved_total += 1;
            if (event.difficulty == Difficulty::Easy) {
                pit->second.solved_easy += 1;
            } else if (event.difficulty == Difficulty::Medium) {
                pit->second.solved_medium += 1;
            } else {
                pit->second.solved_hard += 1;
            }
        }
    }

    if (event.contest_id > 0) {
        auto& contest_map = contest_rows_[event.contest_id];
        auto cit = contest_map.find(event.user_id);
        if (cit == contest_map.end()) {
            ContestRow init;
            init.contest_id = event.contest_id;
            init.user_id = event.user_id;
            init.username = git->second.username;
            cit = contest_map.insert({event.user_id, init}).first;
        } else if (!event.username.empty()) {
            cit->second.username = event.username;
        }

        cit->second.penalty_seconds += event.penalty_seconds;
        if (event.accepted) {
            cit->second.score += score_delta(event.difficulty, true);
            cit->second.last_accepted_at = event.submit_at;

            ContestProblemKey contest_key {event.contest_id, event.user_id, event.problem_id};
            if (accepted_contest_problem_once_.insert(contest_key).second) {
                cit->second.solved_count += 1;
            }
        }
    }
}

std::vector<GlobalRow> Leaderboard::get_global(std::int32_t limit, std::int32_t offset) const {
    std::vector<GlobalRow> rows;
    rows.reserve(global_rows_.size());
    for (const auto& [_, row] : global_rows_) {
        rows.push_back(row);
    }
    sort_global_rows(rows);
    return paginate(rows, limit, offset);
}

std::vector<ContestRow> Leaderboard::get_contest(std::int64_t contest_id, std::int32_t limit, std::int32_t offset) const {
    std::vector<ContestRow> rows;
    const auto it = contest_rows_.find(contest_id);
    if (it == contest_rows_.end()) {
        return rows;
    }

    rows.reserve(it->second.size());
    for (const auto& [_, row] : it->second) {
        rows.push_back(row);
    }
    sort_contest_rows(rows);
    return paginate(rows, limit, offset);
}

std::optional<ProblemStats> Leaderboard::get_problem_stats(std::int64_t user_id) const {
    const auto it = problem_stats_.find(user_id);
    if (it == problem_stats_.end()) {
        return std::nullopt;
    }
    return it->second;
}

std::string Leaderboard::get_global_json(std::int32_t limit, std::int32_t offset) const {
    const auto normalized_offset = std::max<std::int32_t>(0, offset);
    const auto rows = get_global(limit, normalized_offset);
    std::ostringstream oss;
    oss << "{";
    oss << "\"limit\":" << limit << ",";
    oss << "\"offset\":" << normalized_offset << ",";
    oss << "\"count\":" << rows.size() << ",";
    oss << "\"data\":[";
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        const auto rank = static_cast<std::int64_t>(normalized_offset) + static_cast<std::int64_t>(i) + 1;
        oss << "{"
            << "\"rank\":" << rank << ","
            << "\"user_id\":" << r.user_id << ","
            << "\"username\":" << quote_json(r.username) << ","
            << "\"solved_count\":" << r.solved_count << ","
            << "\"total_submissions\":" << r.total_submissions << ","
            << "\"penalty_seconds\":" << r.penalty_seconds << ","
            << "\"score\":" << r.score
            << "}";
        if (i + 1 < rows.size()) {
            oss << ",";
        }
    }
    oss << "]}";
    return oss.str();
}

std::string Leaderboard::get_contest_json(std::int64_t contest_id, std::int32_t limit, std::int32_t offset) const {
    const auto normalized_offset = std::max<std::int32_t>(0, offset);
    const auto rows = get_contest(contest_id, limit, normalized_offset);
    std::ostringstream oss;
    oss << "{";
    oss << "\"contest_id\":" << contest_id << ",";
    oss << "\"limit\":" << limit << ",";
    oss << "\"offset\":" << normalized_offset << ",";
    oss << "\"count\":" << rows.size() << ",";
    oss << "\"data\":[";
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        const auto rank = static_cast<std::int64_t>(normalized_offset) + static_cast<std::int64_t>(i) + 1;
        oss << "{"
            << "\"rank\":" << rank << ","
            << "\"user_id\":" << r.user_id << ","
            << "\"username\":" << quote_json(r.username) << ","
            << "\"solved_count\":" << r.solved_count << ","
            << "\"penalty_seconds\":" << r.penalty_seconds << ","
            << "\"score\":" << r.score
            << "}";
        if (i + 1 < rows.size()) {
            oss << ",";
        }
    }
    oss << "]}";
    return oss.str();
}

std::string Leaderboard::get_problem_stats_json(std::int64_t user_id) const {
    const auto stats = get_problem_stats(user_id);
    if (!stats.has_value()) {
        return "{\"error\":\"user not found\"}";
    }

    const auto& s = stats.value();
    std::ostringstream oss;
    oss << "{"
        << "\"user_id\":" << s.user_id << ","
        << "\"solved_total\":" << s.solved_total << ","
        << "\"solved_easy\":" << s.solved_easy << ","
        << "\"solved_medium\":" << s.solved_medium << ","
        << "\"solved_hard\":" << s.solved_hard
        << "}";
    return oss.str();
}

std::int64_t Leaderboard::score_delta(Difficulty difficulty, bool accepted) {
    if (!accepted) {
        return 0;
    }

    if (difficulty == Difficulty::Easy) {
        return 100;
    }
    if (difficulty == Difficulty::Medium) {
        return 220;
    }
    return 400;
}

void Leaderboard::sort_global_rows(std::vector<GlobalRow>& rows) {
    std::sort(rows.begin(), rows.end(), [](const GlobalRow& a, const GlobalRow& b) {
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

void Leaderboard::sort_contest_rows(std::vector<ContestRow>& rows) {
    std::sort(rows.begin(), rows.end(), [](const ContestRow& a, const ContestRow& b) {
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

std::string Leaderboard::quote_json(const std::string& s) {
    std::string out = "\"";
    for (char c : s) {
        if (c == '"' || c == '\\') {
            out.push_back('\\');
        }
        out.push_back(c);
    }
    out.push_back('"');
    return out;
}

}  // namespace isolated_lb
