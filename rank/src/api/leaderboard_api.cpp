#include "api/leaderboard_api.hpp"

#include <sstream>

namespace oj {

namespace {

std::string quote(const std::string& s) {
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

}  // namespace

LeaderboardApi::LeaderboardApi(std::shared_ptr<LeaderboardService> service) : service_(std::move(service)) {}

std::string LeaderboardApi::get_global_leaderboard_json(std::int32_t limit, std::int32_t offset) {
    const auto rows = service_->get_global_leaderboard(limit, offset);
    std::ostringstream oss;
    oss << "{";
    oss << "\"data\":[";
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        oss << "{"
            << "\"user_id\":" << r.user_id << ","
            << "\"username\":" << quote(r.username) << ","
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

std::string LeaderboardApi::get_problem_stats_json(std::int64_t user_id) {
    const auto stats = service_->get_problem_completion_stats(user_id);
    if (!stats.has_value()) {
        return "{\"error\":\"user not found\"}";
    }

    const auto& v = stats.value();
    std::ostringstream oss;
    oss << "{"
        << "\"user_id\":" << v.user_id << ","
        << "\"solved_total\":" << v.solved_total << ","
        << "\"solved_easy\":" << v.solved_easy << ","
        << "\"solved_medium\":" << v.solved_medium << ","
        << "\"solved_hard\":" << v.solved_hard
        << "}";
    return oss.str();
}

std::string LeaderboardApi::get_contest_leaderboard_json(std::int64_t contest_id, std::int32_t limit, std::int32_t offset) {
    const auto rows = service_->get_contest_leaderboard(contest_id, limit, offset);
    std::ostringstream oss;
    oss << "{";
    oss << "\"contest_id\":" << contest_id << ",";
    oss << "\"data\":[";
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const auto& r = rows[i];
        oss << "{"
            << "\"user_id\":" << r.user_id << ","
            << "\"username\":" << quote(r.username) << ","
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

}  // namespace oj
