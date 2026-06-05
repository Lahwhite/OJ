#include "storage/repository_factory.hpp"

#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "storage/in_memory_repository.hpp"
#include "storage/mysql_repository.hpp"

#include <memory>

namespace oj {

namespace {

void seed_demo_data(const std::shared_ptr<InMemoryLeaderboardRepository>& repo) {
    repo->seed_users({
        {1, "alice", 128, 420, 1800, 28600, 1712000010},
        {2, "bob", 125, 398, 1650, 28100, 1712000000},
        {3, "carol", 118, 350, 1420, 26400, 1712000100},
        {4, "david", 110, 310, 1300, 24200, 1712000200},
        {5, "eve", 102, 280, 1180, 22100, 1712000300},
        {6, "frank", 95, 250, 1050, 19800, 1712000400},
        {7, "grace", 88, 220, 980, 17600, 1712000500},
        {8, "henry", 80, 200, 900, 15400, 1712000600},
    });
    repo->seed_problem_stats({
        {1, 128, 42, 52, 34},
        {2, 125, 45, 48, 32},
        {3, 118, 40, 46, 32},
        {4, 110, 38, 42, 30},
        {5, 102, 35, 40, 27},
        {6, 95, 32, 38, 25},
        {7, 88, 30, 34, 24},
        {8, 80, 28, 30, 22},
    });
    repo->seed_contest_rows(1001, {
        {1001, 1, "alice", 8, 520, 1800, 1712000010},
        {1001, 2, "bob", 8, 500, 1800, 1712000000},
        {1001, 3, "carol", 7, 480, 1750, 1712000100},
        {1001, 4, "david", 6, 450, 1680, 1712000200},
    });
}

}  // namespace

bool leaderboard_database_ready() {
#ifdef OJ_WITH_MYSQL
    return MysqlLeaderboardRepository::database_ready();
#else
    return false;
#endif
}

std::shared_ptr<ILeaderboardRepository> create_leaderboard_repository(bool seed_demo_when_memory) {
#ifdef OJ_WITH_MYSQL
    if (MysqlConnectionPool::instance().available() && MysqlLeaderboardRepository::database_ready()) {
        OJ_LOG_INFO("rank repository: using MySQL persistence");
        return std::make_shared<MysqlLeaderboardRepository>();
    }

    if (MysqlConnectionPool::instance().available()) {
        OJ_LOG_WARN("rank mysql pool is ready but leaderboard tables are missing; fallback to in-memory repository");
    } else {
        OJ_LOG_WARN("rank mysql is unavailable; fallback to in-memory repository");
    }
#else
    OJ_LOG_WARN("rank built without OJ_ENABLE_MYSQL; using in-memory repository");
#endif

    auto repo = std::make_shared<InMemoryLeaderboardRepository>();
    if (seed_demo_when_memory) {
        seed_demo_data(repo);
    }
    return repo;
}

}  // namespace oj
