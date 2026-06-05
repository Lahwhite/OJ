#include "api/leaderboard_api.hpp"
#include "leaderboard_handler.hpp"
#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/log.h"
#include "storage/in_memory_cache.hpp"
#include "storage/repository_factory.hpp"

#include <cstdlib>
#include <iostream>
#include <memory>

namespace {

void shutdown_at_exit() {
    oj::shutdownInfrastructure();
}

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

}  // namespace

int main() {
    try {
        oj::AppConfig cfg = oj::loadConfigFromEnv();
        oj::initInfrastructure(cfg);
        std::atexit(shutdown_at_exit);

        auto repo = oj::create_leaderboard_repository(true);
        auto cache = std::make_shared<oj::InMemoryLeaderboardCache>();

        auto service = std::make_shared<oj::LeaderboardService>(repo, cache);
        auto api = std::make_shared<oj::LeaderboardApi>(service);
        oj::LeaderboardHandler handler(api);
        handler.register_routes();

        const auto port = load_port_from_env();
        std::cout << "Rank server starting on http://127.0.0.1:" << port << "/rank" << std::endl;
        OJ_LOG_INFO("leaderboard_server process started");
        handler.start_server(port);
    } catch (const std::exception& e) {
        OJ_LOG_ERROR(std::string("leaderboard_server failed: ") + e.what());
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
