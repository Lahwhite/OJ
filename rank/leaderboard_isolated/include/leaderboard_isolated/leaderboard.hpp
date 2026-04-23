#pragma once

#include <algorithm>
#include <cstdint>
#include <optional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace isolated_lb {

enum class Difficulty {
    Easy = 0,
    Medium = 1,
    Hard = 2
};

struct SubmissionEvent {
    std::int64_t submission_id {};
    std::int64_t user_id {};
    std::string username;
    std::int64_t problem_id {};
    std::int64_t contest_id {-1};
    Difficulty difficulty {Difficulty::Easy};
    bool accepted {false};
    std::int64_t submit_at {};
    std::int64_t penalty_seconds {};
};

struct GlobalRow {
    std::int64_t user_id {};
    std::string username;
    std::int32_t solved_count {};
    std::int32_t total_submissions {};
    std::int64_t penalty_seconds {};
    std::int64_t score {};
    std::int64_t last_accepted_at {};
};

struct ContestRow {
    std::int64_t contest_id {};
    std::int64_t user_id {};
    std::string username;
    std::int32_t solved_count {};
    std::int64_t penalty_seconds {};
    std::int64_t score {};
    std::int64_t last_accepted_at {};
};

struct ProblemStats {
    std::int64_t user_id {};
    std::int32_t solved_total {};
    std::int32_t solved_easy {};
    std::int32_t solved_medium {};
    std::int32_t solved_hard {};
};

class Leaderboard {
public:
    void apply_submission(const SubmissionEvent& event);

    std::vector<GlobalRow> get_global(std::int32_t limit, std::int32_t offset) const;
    std::vector<ContestRow> get_contest(std::int64_t contest_id, std::int32_t limit, std::int32_t offset) const;
    std::optional<ProblemStats> get_problem_stats(std::int64_t user_id) const;

    std::string get_global_json(std::int32_t limit, std::int32_t offset) const;
    std::string get_contest_json(std::int64_t contest_id, std::int32_t limit, std::int32_t offset) const;
    std::string get_problem_stats_json(std::int64_t user_id) const;

private:
    struct ContestProblemKey {
        std::int64_t contest_id {};
        std::int64_t user_id {};
        std::int64_t problem_id {};

        bool operator==(const ContestProblemKey& other) const;
    };

    struct ContestProblemKeyHash {
        std::size_t operator()(const ContestProblemKey& key) const;
    };

    static std::int64_t score_delta(Difficulty difficulty, bool accepted);
    static void sort_global_rows(std::vector<GlobalRow>& rows);
    static void sort_contest_rows(std::vector<ContestRow>& rows);

    template <typename T>
    static std::vector<T> paginate(const std::vector<T>& rows, std::int32_t limit, std::int32_t offset) {
        if (rows.empty()) {
            return {};
        }

        if (offset < 0) {
            offset = 0;
        }
        if (limit <= 0) {
            limit = static_cast<std::int32_t>(rows.size());
        }

        const auto begin_idx = static_cast<std::size_t>(offset);
        if (begin_idx >= rows.size()) {
            return {};
        }

        const auto end_idx = std::min(rows.size(), begin_idx + static_cast<std::size_t>(limit));
        return std::vector<T>(rows.begin() + static_cast<std::ptrdiff_t>(begin_idx), rows.begin() + static_cast<std::ptrdiff_t>(end_idx));
    }

    static std::string quote_json(const std::string& s);

    std::unordered_map<std::int64_t, GlobalRow> global_rows_;
    std::unordered_map<std::int64_t, ProblemStats> problem_stats_;
    std::unordered_map<std::int64_t, std::unordered_map<std::int64_t, ContestRow>> contest_rows_;

    std::unordered_set<std::uint64_t> accepted_problem_once_;
    std::unordered_set<ContestProblemKey, ContestProblemKeyHash> accepted_contest_problem_once_;
};

}  // namespace isolated_lb
