#pragma once

#include "api/leaderboard_api.hpp"

#include "crow_all.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace oj {

class LeaderboardHandler {
public:
    explicit LeaderboardHandler(std::shared_ptr<LeaderboardApi> api);

    void start_server(std::uint16_t port);

private:
    static crow::response json_body(int status, const std::string& body);
    static crow::response web_file(const std::string& relative_path, const std::string& content_type);
    static std::optional<std::filesystem::path> resolve_web_path(const std::string& relative_path);
    static std::int32_t parse_int_param(const char* value, std::int32_t fallback, std::int32_t min_v, std::int32_t max_v);

    std::shared_ptr<LeaderboardApi> api_;
    std::unique_ptr<crow::Crow<crow::CORSHandler>> app_;
};

}  // namespace oj
