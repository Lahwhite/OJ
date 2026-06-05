#include "leaderboard/leaderboard_service.hpp"
#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/mysql_pool.h"
#include "storage/in_memory_cache.hpp"
#include "storage/repository_factory.hpp"

#include <iostream>
#include <memory>

int main() {
#ifdef OJ_WITH_MYSQL
    try {
        oj::AppConfig cfg = oj::loadConfigFromEnv();
        oj::initInfrastructure(cfg);

        if (!oj::MysqlConnectionPool::instance().available()) {
            std::cout << "leaderboard mysql tests skipped: mysql pool unavailable\n";
            oj::shutdownInfrastructure();
            return 0;
        }
        if (!oj::leaderboard_database_ready()) {
            std::cout << "leaderboard mysql tests skipped: rank tables missing\n";
            oj::shutdownInfrastructure();
            return 0;
        }

        auto repo = oj::create_leaderboard_repository(false);
        auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
        oj::LeaderboardService service(repo, cache);

        const auto rows = service.get_global_leaderboard(10, 0);
        std::cout << "leaderboard mysql tests passed, rows=" << rows.size() << "\n";
        oj::shutdownInfrastructure();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "leaderboard mysql tests failed: " << e.what() << "\n";
        oj::shutdownInfrastructure();
        return 1;
    }
#else
    std::cout << "leaderboard mysql tests skipped: built without OJ_ENABLE_MYSQL\n";
    return 0;
#endif
}
