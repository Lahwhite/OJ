#pragma once

#include "api/leaderboard_api.hpp"

#include "oj/http_server.h"

#include <cstdint>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>

namespace oj {

class LeaderboardHandler {
public:
    explicit LeaderboardHandler(std::shared_ptr<LeaderboardApi> api);

    void register_routes();
    void start_server(std::uint16_t port);
    void stop_server();

private:
    HttpResponse handle_root(const HttpRequest& request) const;
    HttpResponse handle_rank_page(const HttpRequest& request) const;
    HttpResponse handle_rank_static(const HttpRequest& request) const;
    HttpResponse handle_health(const HttpRequest& request) const;
    HttpResponse handle_global_leaderboard(const HttpRequest& request) const;
    HttpResponse handle_global_summary(const HttpRequest& request) const;
    HttpResponse handle_global_export_csv(const HttpRequest& request) const;
    HttpResponse handle_contest_leaderboard(const HttpRequest& request) const;
    HttpResponse handle_user_stats(const HttpRequest& request) const;

    static HttpResponse json_body(int status, const std::string& body);
    static HttpResponse redirect(const std::string& location);
    static HttpResponse web_file(const std::string& relative_path, const std::string& content_type);
    static std::optional<std::filesystem::path> resolve_web_path(const std::string& relative_path);
    static std::int32_t query_int(
        const HttpRequest& request,
        const std::string& key,
        std::int32_t fallback,
        std::int32_t min_v,
        std::int32_t max_v);
    static std::int64_t parse_path_int64(const std::string& segment);

    std::shared_ptr<LeaderboardApi> api_;
    HttpServer server_;
};

}  // namespace oj
