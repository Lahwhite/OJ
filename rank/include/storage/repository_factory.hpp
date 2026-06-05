#pragma once

#include "leaderboard/repository.hpp"

#include <memory>

namespace oj {

std::shared_ptr<ILeaderboardRepository> create_leaderboard_repository(bool seed_demo_when_memory = false);
bool leaderboard_database_ready();

}  // namespace oj
