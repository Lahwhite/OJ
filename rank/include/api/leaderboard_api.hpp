#pragma once

#include "leaderboard/leaderboard_service.hpp"

#include <memory>
#include <string>

namespace oj {

class LeaderboardApi {
public:
    explicit LeaderboardApi(std::shared_ptr<LeaderboardService> service);

    std::string get_global_leaderboard_json(std::int32_t limit, std::int32_t offset);
    std::string get_problem_stats_json(std::int64_t user_id);
    std::string get_contest_leaderboard_json(std::int64_t contest_id, std::int32_t limit, std::int32_t offset);

private:
    std::shared_ptr<LeaderboardService> service_;
};

}  // namespace oj
