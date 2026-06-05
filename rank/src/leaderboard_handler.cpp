#include "leaderboard_handler.hpp"

#include "oj/json_error.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"
#include "storage/repository_factory.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <unordered_map>

#if defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace oj {

namespace {

std::filesystem::path executable_directory() {
#if defined(_WIN32)
    std::vector<wchar_t> buffer(512);
    for (;;) {
        const DWORD len = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
        if (len == 0) {
            return {};
        }
        if (len < buffer.size()) {
            buffer.resize(len);
            return std::filesystem::path(buffer.data()).parent_path();
        }
        buffer.resize(buffer.size() * 2);
    }
#else
    std::error_code ec;
    const auto exe = std::filesystem::read_symlink("/proc/self/exe", ec);
    if (!ec) {
        return exe.parent_path();
    }
    return {};
#endif
}

std::vector<std::filesystem::path> web_roots() {
#ifndef RANK_WEB_DIR
#define RANK_WEB_DIR "rank/web"
#endif

    std::vector<std::filesystem::path> roots = {
        std::filesystem::path("web"),
        std::filesystem::path("rank") / "web",
        std::filesystem::path(RANK_WEB_DIR),
    };

    const auto exe_dir = executable_directory();
    if (!exe_dir.empty()) {
        roots.push_back(exe_dir / "web");
        roots.push_back(exe_dir / ".." / "web");
        roots.push_back(exe_dir / ".." / "rank" / "web");
    }

    std::vector<std::filesystem::path> unique_roots;
    for (const auto& root : roots) {
        std::error_code ec;
        const auto canonical = std::filesystem::weakly_canonical(root, ec);
        const auto& normalized = ec ? root : canonical;
        if (std::find(unique_roots.begin(), unique_roots.end(), normalized) == unique_roots.end()) {
            unique_roots.push_back(normalized);
        }
    }
    return unique_roots;
}

std::unordered_map<std::string, std::string> parse_query(const std::string& query) {
    std::unordered_map<std::string, std::string> params;
    std::size_t start = 0;
    while (start < query.size()) {
        const auto amp = query.find('&', start);
        const auto part = query.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        const auto eq = part.find('=');
        if (eq != std::string::npos) {
            params[part.substr(0, eq)] = part.substr(eq + 1);
        }
        if (amp == std::string::npos) {
            break;
        }
        start = amp + 1;
    }
    return params;
}

std::vector<std::string> split_path(const std::string& path) {
    std::vector<std::string> parts;
    std::size_t start = 0;
    while (start < path.size()) {
        const auto slash = path.find('/', start);
        if (slash == std::string::npos) {
            if (slash != start) {
                parts.push_back(path.substr(start));
            }
            break;
        }
        if (slash > start) {
            parts.push_back(path.substr(start, slash - start));
        }
        start = slash + 1;
    }
    return parts;
}

}  // namespace

LeaderboardHandler::LeaderboardHandler(std::shared_ptr<LeaderboardApi> api) : api_(std::move(api)) {}

void LeaderboardHandler::register_routes() {
    auto& router = server_.router();

    router.get("/", [this](const HttpRequest& req) { return handle_root(req); });
    router.get("/rank", [this](const HttpRequest& req) { return handle_rank_page(req); });
    router.get_prefix("/rank/static/", [this](const HttpRequest& req) { return handle_rank_static(req); });

    router.get("/health", [this](const HttpRequest& req) { return handle_health(req); });
    router.get("/api/leaderboard/global", [this](const HttpRequest& req) { return handle_global_leaderboard(req); });
    router.get("/api/leaderboard/global/summary", [this](const HttpRequest& req) { return handle_global_summary(req); });
    router.get("/api/leaderboard/global/export.csv",
        [this](const HttpRequest& req) { return handle_global_export_csv(req); });

    router.get_prefix("/api/leaderboard/contest/", [this](const HttpRequest& req) {
        return handle_contest_leaderboard(req);
    });
    router.get_prefix("/api/leaderboard/users/", [this](const HttpRequest& req) { return handle_user_stats(req); });
}

void LeaderboardHandler::start_server(std::uint16_t port) {
    OJ_LOG_INFO("rank HTTP server starting, port=" + std::to_string(port));
    if (!server_.start(port)) {
        throw std::runtime_error("failed to start rank HTTP server");
    }
}

void LeaderboardHandler::stop_server() {
    server_.stop();
}

HttpResponse LeaderboardHandler::handle_root(const HttpRequest&) const {
    return redirect("/rank");
}

HttpResponse LeaderboardHandler::handle_rank_page(const HttpRequest&) const {
    return web_file("index.html", "text/html; charset=utf-8");
}

HttpResponse LeaderboardHandler::handle_rank_static(const HttpRequest& request) const {
    const std::string prefix = "/rank/static/";
    if (request.path.size() <= prefix.size()) {
        return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "static file not found"));
    }
    const std::string relative = request.path.substr(prefix.size());
    std::string content_type = "text/plain; charset=utf-8";
    if (relative.size() >= 4 && relative.substr(relative.size() - 4) == ".css") {
        content_type = "text/css; charset=utf-8";
    } else if (relative.size() >= 3 && relative.substr(relative.size() - 3) == ".js") {
        content_type = "application/javascript; charset=utf-8";
    }
    return web_file(relative, content_type);
}

HttpResponse LeaderboardHandler::handle_health(const HttpRequest&) const {
    std::ostringstream body;
    body << "{"
         << "\"status\":\"ok\","
         << "\"service\":\"oj-rank\","
         << "\"mysql_pool_ready\":" << (MysqlConnectionPool::instance().available() ? "true" : "false") << ","
         << "\"rank_database_ready\":" << (leaderboard_database_ready() ? "true" : "false") << ","
         << "\"redis_ready\":" << (RedisCache::instance().connected() ? "true" : "false");
    const auto st = MysqlConnectionPool::instance().stats();
    body << ",\"mysql_idle_connections\":" << st.pool_size
         << ",\"mysql_in_use_connections\":" << st.in_use
         << "}";
    return json_body(200, body.str());
}

HttpResponse LeaderboardHandler::handle_global_leaderboard(const HttpRequest& request) const {
    const auto limit = query_int(request, "limit", 50, 1, 200);
    const auto offset = query_int(request, "offset", 0, 0, 100000);
    OJ_LOG_INFO("rank global leaderboard, limit=" + std::to_string(limit) + " offset=" + std::to_string(offset));
    return json_body(200, api_->get_global_leaderboard_json(limit, offset));
}

HttpResponse LeaderboardHandler::handle_global_summary(const HttpRequest&) const {
    return json_body(200, api_->get_global_summary_json());
}

HttpResponse LeaderboardHandler::handle_global_export_csv(const HttpRequest& request) const {
    const auto limit = query_int(request, "limit", 200, 1, 500);
    const auto offset = query_int(request, "offset", 0, 0, 100000);
    auto response = HttpResponse::text(200, api_->get_global_leaderboard_csv(limit, offset), "text/csv; charset=utf-8");
    response.headers["Content-Disposition"] = "attachment; filename=\"global_leaderboard.csv\"";
    return response;
}

HttpResponse LeaderboardHandler::handle_contest_leaderboard(const HttpRequest& request) const {
    const auto parts = split_path(request.path);
    if (parts.size() != 4 || parts[0] != "api" || parts[1] != "leaderboard" || parts[2] != "contest") {
        return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "contest route not found"));
    }

    const auto contest_id = parse_path_int64(parts[3]);
    const auto limit = query_int(request, "limit", 50, 1, 200);
    const auto offset = query_int(request, "offset", 0, 0, 100000);
    OJ_LOG_INFO("rank contest leaderboard, contest_id=" + std::to_string(contest_id));
    return json_body(200, api_->get_contest_leaderboard_json(contest_id, limit, offset));
}

HttpResponse LeaderboardHandler::handle_user_stats(const HttpRequest& request) const {
    const auto parts = split_path(request.path);
    if (parts.size() != 5 || parts[0] != "api" || parts[1] != "leaderboard" || parts[2] != "users") {
        return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "user route not found"));
    }

    const auto user_id = parse_path_int64(parts[3]);
    if (parts[4] == "stats") {
        return json_body(200, api_->get_problem_stats_json(user_id));
    }
    if (parts[4] == "insight") {
        return json_body(200, api_->get_user_insight_json(user_id));
    }
    return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "user route not found"));
}

HttpResponse LeaderboardHandler::json_body(int status, const std::string& body) {
    return HttpResponse::json(status, body);
}

HttpResponse LeaderboardHandler::redirect(const std::string& location) {
    HttpResponse response;
    response.status = 302;
    response.headers["Location"] = location;
    return response;
}

HttpResponse LeaderboardHandler::web_file(const std::string& relative_path, const std::string& content_type) {
    const auto path = resolve_web_path(relative_path);
    if (!path.has_value()) {
        OJ_LOG_WARN("rank static file not found: " + relative_path);
        return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "static file not found"));
    }

    std::ifstream input(*path, std::ios::binary);
    if (!input) {
        OJ_LOG_ERROR("rank static file open failed: " + path->string());
        return HttpResponse::json(500, makeErrorJson("INTERNAL_ERROR", "failed to read static file"));
    }

    std::ostringstream body;
    body << input.rdbuf();
    return HttpResponse::text(200, body.str(), content_type);
}

std::optional<std::filesystem::path> LeaderboardHandler::resolve_web_path(const std::string& relative_path) {
    if (relative_path.find("..") != std::string::npos) {
        return std::nullopt;
    }
    if (!relative_path.empty() && (relative_path.front() == '/' || relative_path.front() == '\\')) {
        return std::nullopt;
    }

    for (const auto& root : web_roots()) {
        const auto candidate = root / relative_path;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec) && !ec) {
            return candidate;
        }
    }
    return std::nullopt;
}

std::int32_t LeaderboardHandler::query_int(
    const HttpRequest& request,
    const std::string& key,
    std::int32_t fallback,
    std::int32_t min_v,
    std::int32_t max_v) {
    const auto params = parse_query(request.query);
    const auto it = params.find(key);
    if (it == params.end()) {
        return fallback;
    }
    try {
        const long parsed = std::stol(it->second);
        return static_cast<std::int32_t>(std::clamp(parsed, static_cast<long>(min_v), static_cast<long>(max_v)));
    } catch (...) {
        return fallback;
    }
}

std::int64_t LeaderboardHandler::parse_path_int64(const std::string& segment) {
    try {
        return std::stoll(segment);
    } catch (...) {
        throw HttpException(400, "bad_request", "invalid numeric path segment");
    }
}

}  // namespace oj
