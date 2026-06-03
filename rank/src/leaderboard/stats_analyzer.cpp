#include "leaderboard/stats_analyzer.hpp"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <optional>
#include <sstream>

namespace oj {

namespace {

std::string quote_json(const std::string& s) {
    std::ostringstream oss;
    oss << '"';
    for (char c : s) {
        if (c == '"' || c == '\\') {
            oss << '\\';
        }
        oss << c;
    }
    oss << '"';
    return oss.str();
}

}  // namespace

GlobalLeaderboardSummary StatsAnalyzer::analyze_global(const std::vector<LeaderboardRow>& rows) {
    GlobalLeaderboardSummary summary;
    summary.total_users = static_cast<std::int32_t>(rows.size());
    if (rows.empty()) {
        summary.score_buckets = build_default_buckets(rows);
        return summary;
    }

    std::int64_t score_sum = 0;
    std::int64_t solved_sum = 0;
    std::int64_t penalty_sum = 0;
    summary.max_score = rows.front().score;
    summary.min_score = rows.front().score;

    for (const auto& row : rows) {
        score_sum += row.score;
        solved_sum += row.solved_count;
        penalty_sum += row.penalty_seconds;
        summary.max_score = std::max(summary.max_score, row.score);
        summary.min_score = std::min(summary.min_score, row.score);
    }

    summary.average_score = static_cast<double>(score_sum) / static_cast<double>(rows.size());
    summary.average_solved = static_cast<double>(solved_sum) / static_cast<double>(rows.size());
    summary.average_penalty = static_cast<double>(penalty_sum) / static_cast<double>(rows.size());
    summary.median_score = median_score_of(rows);
    summary.top_score_gap = summary.max_score - (rows.size() > 1 ? rows[1].score : rows[0].score);
    summary.score_buckets = build_default_buckets(rows);
    return summary;
}

std::optional<UserRankInsight> StatsAnalyzer::analyze_user(
    const std::vector<LeaderboardRow>& rows,
    std::int64_t user_id) {
    if (rows.empty()) {
        return std::nullopt;
    }

    for (std::size_t i = 0; i < rows.size(); ++i) {
        if (rows[i].user_id != user_id) {
            continue;
        }

        UserRankInsight insight;
        insight.user_id = rows[i].user_id;
        insight.username = rows[i].username;
        insight.rank = static_cast<std::int64_t>(i) + 1;
        insight.score = rows[i].score;
        insight.solved_count = rows[i].solved_count;
        insight.score_gap_to_top = rows.front().score - rows[i].score;

        const double percentile_base = static_cast<double>(rows.size() - i) / static_cast<double>(rows.size());
        insight.score_percentile = std::round(percentile_base * 1000.0) / 10.0;
        return insight;
    }
    return std::nullopt;
}

std::string StatsAnalyzer::summary_to_json(const GlobalLeaderboardSummary& summary) {
    std::ostringstream oss;
    oss << "{";
    oss << "\"total_users\":" << summary.total_users << ",";
    oss << "\"max_score\":" << summary.max_score << ",";
    oss << "\"min_score\":" << summary.min_score << ",";
    oss << "\"average_score\":" << std::fixed << std::setprecision(2) << summary.average_score << ",";
    oss << "\"average_solved\":" << std::fixed << std::setprecision(2) << summary.average_solved << ",";
    oss << "\"average_penalty\":" << std::fixed << std::setprecision(2) << summary.average_penalty << ",";
    oss << "\"median_score\":" << summary.median_score << ",";
    oss << "\"top_score_gap\":" << summary.top_score_gap << ",";
    oss << "\"score_buckets\":[";
    for (std::size_t i = 0; i < summary.score_buckets.size(); ++i) {
        const auto& bucket = summary.score_buckets[i];
        oss << "{"
            << "\"label\":" << quote_json(bucket.label) << ","
            << "\"min_score\":" << bucket.min_score << ","
            << "\"max_score\":" << bucket.max_score << ","
            << "\"user_count\":" << bucket.user_count
            << "}";
        if (i + 1 < summary.score_buckets.size()) {
            oss << ",";
        }
    }
    oss << "]}";
    return oss.str();
}

std::string StatsAnalyzer::insight_to_json(const UserRankInsight& insight) {
    std::ostringstream oss;
    oss << "{"
        << "\"user_id\":" << insight.user_id << ","
        << "\"username\":" << quote_json(insight.username) << ","
        << "\"rank\":" << insight.rank << ","
        << "\"score\":" << insight.score << ","
        << "\"solved_count\":" << insight.solved_count << ","
        << "\"score_percentile\":" << std::fixed << std::setprecision(1) << insight.score_percentile << ","
        << "\"score_gap_to_top\":" << insight.score_gap_to_top
        << "}";
    return oss.str();
}

std::string StatsAnalyzer::rows_to_csv(const std::vector<LeaderboardRow>& rows, bool include_rank) {
    std::ostringstream oss;
    if (include_rank) {
        oss << "rank,user_id,username,score,solved_count,total_submissions,penalty_seconds\n";
    } else {
        oss << "user_id,username,score,solved_count,total_submissions,penalty_seconds\n";
    }

    for (std::size_t i = 0; i < rows.size(); ++i) {
        const auto& row = rows[i];
        if (include_rank) {
            oss << (i + 1) << ',';
        }
        oss << row.user_id << ','
            << csv_escape(row.username) << ','
            << row.score << ','
            << row.solved_count << ','
            << row.total_submissions << ','
            << row.penalty_seconds << '\n';
    }
    return oss.str();
}

std::int64_t StatsAnalyzer::median_score_of(const std::vector<LeaderboardRow>& rows) {
    std::vector<std::int64_t> scores;
    scores.reserve(rows.size());
    for (const auto& row : rows) {
        scores.push_back(row.score);
    }
    std::sort(scores.begin(), scores.end());
    return scores[scores.size() / 2];
}

std::vector<ScoreBucket> StatsAnalyzer::build_default_buckets(const std::vector<LeaderboardRow>& rows) {
    const std::vector<ScoreBucket> definitions = {
        {"Bronze", 0, 999},
        {"Silver", 1000, 1999},
        {"Gold", 2000, 2999},
        {"Master", 3000, 999999},
    };

    std::vector<ScoreBucket> buckets = definitions;
    for (const auto& row : rows) {
        for (auto& bucket : buckets) {
            if (row.score >= bucket.min_score && row.score <= bucket.max_score) {
                bucket.user_count += 1;
                break;
            }
        }
    }
    return buckets;
}

std::string StatsAnalyzer::csv_escape(const std::string& value) {
    if (value.find_first_of(",\"\n") == std::string::npos) {
        return value;
    }
    std::string escaped = "\"";
    for (char c : value) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped.push_back(c);
        }
    }
    escaped += '"';
    return escaped;
}

}  // namespace oj
