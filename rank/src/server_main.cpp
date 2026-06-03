#include "api/leaderboard_api.hpp"
#include "leaderboard_handler.hpp"
#include "storage/in_memory_cache.hpp"
#include "storage/in_memory_repository.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>

namespace {

std::uint16_t load_port_from_env() {
#if defined(_WIN32)
    char* value = nullptr;
    size_t len = 0;
    if (_dupenv_s(&value, &len, "OJ_RANK_PORT") == 0 && value) {
        try {
            const int parsed = std::stoi(value);
            std::free(value);
            if (parsed > 0 && parsed <= 65535) {
                return static_cast<std::uint16_t>(parsed);
            }
        } catch (...) {
            std::free(value);
            return 8092;
        }
    }
    return 8092;
#else
    const char* value = std::getenv("OJ_RANK_PORT");
    if (!value) {
        return 8092;
    }
    try {
        const int parsed = std::stoi(value);
        if (parsed > 0 && parsed <= 65535) {
            return static_cast<std::uint16_t>(parsed);
        }
    } catch (...) {
    }
    return 8092;
#endif
}

void seed_demo_data(const std::shared_ptr<oj::InMemoryLeaderboardRepository>& repo) {
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

int main() {
    try {
        auto repo = std::make_shared<oj::InMemoryLeaderboardRepository>();
        auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();
        seed_demo_data(repo);

        auto service = std::make_shared<oj::LeaderboardService>(repo, cache);
        auto api = std::make_shared<oj::LeaderboardApi>(service);
        oj::LeaderboardHandler handler(api);

        const auto port = load_port_from_env();
        std::cout << "Rank server starting on http://127.0.0.1:" << port << "/rank" << std::endl;
        handler.start_server(port);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
