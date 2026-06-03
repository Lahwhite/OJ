#include "leaderboard_handler.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

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

}  // namespace

LeaderboardHandler::LeaderboardHandler(std::shared_ptr<LeaderboardApi> api) : api_(std::move(api)) {
    app_ = std::make_unique<crow::Crow<crow::CORSHandler>>();

    CROW_ROUTE((*app_), "/").methods("GET"_method)([] {
        crow::response response(302);
        response.add_header("Location", "/rank");
        return response;
    });

    CROW_ROUTE((*app_), "/rank").methods("GET"_method)([] {
        return web_file("index.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE((*app_), "/rank/static/<path>").methods("GET"_method)([](const std::string& file_path) {
        std::string content_type = "text/plain; charset=utf-8";
        if (file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".css") {
            content_type = "text/css; charset=utf-8";
        } else if (file_path.size() >= 3 && file_path.substr(file_path.size() - 3) == ".js") {
            content_type = "application/javascript; charset=utf-8";
        }
        return web_file(file_path, content_type);
    });

    CROW_ROUTE((*app_), "/api/leaderboard/global").methods("GET"_method)([this](const crow::request& req) {
        const auto limit = parse_int_param(req.url_params.get("limit"), 50, 1, 200);
        const auto offset = parse_int_param(req.url_params.get("offset"), 0, 0, 100000);
        return json_body(200, api_->get_global_leaderboard_json(limit, offset));
    });

    CROW_ROUTE((*app_), "/api/leaderboard/global/summary").methods("GET"_method)([this] {
        return json_body(200, api_->get_global_summary_json());
    });

    CROW_ROUTE((*app_), "/api/leaderboard/global/export.csv").methods("GET"_method)([this](const crow::request& req) {
        const auto limit = parse_int_param(req.url_params.get("limit"), 200, 1, 500);
        const auto offset = parse_int_param(req.url_params.get("offset"), 0, 0, 100000);
        crow::response response(200, api_->get_global_leaderboard_csv(limit, offset));
        response.add_header("Content-Type", "text/csv; charset=utf-8");
        response.add_header("Content-Disposition", "attachment; filename=\"global_leaderboard.csv\"");
        return response;
    });

    CROW_ROUTE((*app_), "/api/leaderboard/users/<int>/insight").methods("GET"_method)(
        [this](std::int64_t user_id) { return json_body(200, api_->get_user_insight_json(user_id)); });

    CROW_ROUTE((*app_), "/api/leaderboard/contest/<int>").methods("GET"_method)(
        [this](const crow::request& req, std::int64_t contest_id) {
            const auto limit = parse_int_param(req.url_params.get("limit"), 50, 1, 200);
            const auto offset = parse_int_param(req.url_params.get("offset"), 0, 0, 100000);
            return json_body(200, api_->get_contest_leaderboard_json(contest_id, limit, offset));
        });

    CROW_ROUTE((*app_), "/api/leaderboard/users/<int>/stats").methods("GET"_method)(
        [this](std::int64_t user_id) { return json_body(200, api_->get_problem_stats_json(user_id)); });

    CROW_ROUTE((*app_), "/health").methods("GET"_method)([] {
        return json_body(200, R"({"status":"ok","service":"oj-rank"})");
    });
}

void LeaderboardHandler::start_server(std::uint16_t port) {
    app_->port(port).multithreaded().run();
}

crow::response LeaderboardHandler::json_body(int status, const std::string& body) {
    crow::response response(status, body);
    response.add_header("Content-Type", "application/json; charset=utf-8");
    return response;
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

crow::response LeaderboardHandler::web_file(const std::string& relative_path, const std::string& content_type) {
    const auto path = resolve_web_path(relative_path);
    if (!path.has_value()) {
        return crow::response(404);
    }

    std::ifstream input(*path, std::ios::binary);
    if (!input) {
        return crow::response(404);
    }

    std::ostringstream body;
    body << input.rdbuf();

    crow::response response(200, body.str());
    response.add_header("Content-Type", content_type);
    return response;
}

std::int32_t LeaderboardHandler::parse_int_param(
    const char* value,
    std::int32_t fallback,
    std::int32_t min_v,
    std::int32_t max_v) {
    if (!value) {
        return fallback;
    }
    try {
        const long parsed = std::stol(value);
        return static_cast<std::int32_t>(std::clamp(parsed, static_cast<long>(min_v), static_cast<long>(max_v)));
    } catch (...) {
        return fallback;
    }
}

}  // namespace oj
