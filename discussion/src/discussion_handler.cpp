#include "discussion_handler.h"

#include "oj/json_error.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <limits>
#include <string>
#include <vector>

using json = nlohmann::json;

DiscussionHandler::DiscussionHandler() {
    app_ = std::make_unique<crow::Crow<crow::CORSHandler>>();

    CROW_ROUTE((*app_), "/discussion").methods("GET"_method)([] {
        return webFileResponse("index.html", "text/html; charset=utf-8");
    });

    CROW_ROUTE((*app_), "/discussion/static/<path>").methods("GET"_method)([](const std::string& file_path) {
        std::string content_type = "text/plain; charset=utf-8";
        if (file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".css") {
            content_type = "text/css; charset=utf-8";
        } else if (file_path.size() >= 3 && file_path.substr(file_path.size() - 3) == ".js") {
            content_type = "application/javascript; charset=utf-8";
        }
        return webFileResponse(file_path, content_type);
    });

    CROW_ROUTE((*app_), "/uploads/avatars/<path>").methods("GET"_method)([](const std::string& file_path) {
        return userAvatarResponse(file_path);
    });

    CROW_ROUTE((*app_), "/images/<path>").methods("GET"_method)([](const std::string& file_path) {
        return userImageResponse(file_path);
    });

    CROW_ROUTE((*app_), "/api/discussions/topics").methods("POST"_method)([this](const crow::request& req) {
        return handleCreateTopic(req);
    });

    CROW_ROUTE((*app_), "/api/discussions/topics").methods("GET"_method)([this](const crow::request& req) {
        return handleListTopics(req);
    });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>").methods("GET"_method)(
        [this](const crow::request& req, int topic_id) { return handleGetTopic(req, topic_id); });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>").methods("DELETE"_method)(
        [this](const crow::request& req, int topic_id) { return handleDeleteTopic(req, topic_id); });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments").methods("POST"_method)(
        [this](const crow::request& req, int topic_id) { return handleCreateComment(req, topic_id); });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments").methods("GET"_method)(
        [this](const crow::request& req, int topic_id) { return handleListComments(req, topic_id); });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments/<int>").methods("DELETE"_method)(
        [this](const crow::request& req, int topic_id, int comment_id) {
            return handleDeleteComment(req, topic_id, comment_id);
        });

    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/summary").methods("POST"_method)(
        [this](const crow::request& req, int topic_id) { return handleSummarizeTopic(req, topic_id); });

    CROW_ROUTE((*app_), "/health").methods("GET"_method)([this] {
        json body;
        body["status"] = "ok";
        body["service"] = "oj-discussion";
        body["mysql_pool_ready"] = oj::MysqlConnectionPool::instance().available();
        body["discussion_database_ready"] = service_.databaseReady();
        body["redis_ready"] = oj::RedisCache::instance().connected();
        body["gemini_configured"] = GeminiClient::configured();
        body["gemini_model"] = GeminiClient::activeModel();
        auto st = oj::MysqlConnectionPool::instance().stats();
        body["mysql_idle_connections"] = st.pool_size;
        body["mysql_in_use_connections"] = st.in_use;
        return jsonResponse(200, body);
    });
}

crow::response DiscussionHandler::jsonResponse(int status, const nlohmann::json& body) {
    crow::response response(status, body.dump());
    response.add_header("Content-Type", "application/json");
    return response;
}

crow::response DiscussionHandler::webFileResponse(const std::string& relative_path, const std::string& content_type) {
    if (relative_path.find("..") != std::string::npos ||
        relative_path.find(':') != std::string::npos ||
        (!relative_path.empty() && (relative_path.front() == '/' || relative_path.front() == '\\'))) {
        return crow::response(404);
    }

#ifndef DISCUSSION_WEB_DIR
#define DISCUSSION_WEB_DIR "discussion/web"
#endif
    const std::vector<std::filesystem::path> web_roots = {
        std::filesystem::path("web"),
        std::filesystem::path("discussion") / "web",
        std::filesystem::path(DISCUSSION_WEB_DIR),
    };

    std::filesystem::path path = std::filesystem::path(DISCUSSION_WEB_DIR) / relative_path;
    for (const auto& root : web_roots) {
        const auto candidate = root / relative_path;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec)) {
            path = candidate;
            break;
        }
    }

    crow::response response;
    response.set_static_file_info_unsafe(path.string(), content_type);
    return response;
}

namespace {
bool isSafeStaticPath(const std::string& relative_path) {
    return !relative_path.empty() &&
           relative_path.find("..") == std::string::npos &&
           relative_path.find(':') == std::string::npos &&
           relative_path.front() != '/' &&
           relative_path.front() != '\\';
}

std::string imageContentType(const std::string& relative_path) {
    const auto dot = relative_path.find_last_of('.');
    if (dot == std::string::npos) {
        return "application/octet-stream";
    }

    std::string ext = relative_path.substr(dot);
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });

    if (ext == ".png") {
        return "image/png";
    }
    if (ext == ".jpg" || ext == ".jpeg") {
        return "image/jpeg";
    }
    if (ext == ".gif") {
        return "image/gif";
    }
    if (ext == ".webp") {
        return "image/webp";
    }
    if (ext == ".svg") {
        return "image/svg+xml";
    }
    return "application/octet-stream";
}

crow::response staticFileFromRoots(const std::string& relative_path,
                                   const std::vector<std::filesystem::path>& roots,
                                   const std::string& content_type) {
    if (!isSafeStaticPath(relative_path)) {
        return crow::response(404);
    }

    for (const auto& root : roots) {
        const auto candidate = root / relative_path;
        std::error_code ec;
        if (std::filesystem::is_regular_file(candidate, ec)) {
            crow::response response;
            response.set_static_file_info_unsafe(candidate.string(), content_type);
            return response;
        }
    }

    return crow::response(404);
}
}  // namespace

crow::response DiscussionHandler::userAvatarResponse(const std::string& relative_path) {
    const std::vector<std::filesystem::path> avatar_roots = {
        std::filesystem::path("uploads") / "avatars",
        std::filesystem::path("Users") / "uploads" / "avatars",
        std::filesystem::path("..") / "Users" / "uploads" / "avatars",
        std::filesystem::path("..") / ".." / "Users" / "uploads" / "avatars",
    };
    return staticFileFromRoots(relative_path, avatar_roots, imageContentType(relative_path));
}

crow::response DiscussionHandler::userImageResponse(const std::string& relative_path) {
    const std::vector<std::filesystem::path> image_roots = {
        std::filesystem::path("images"),
        std::filesystem::path("Users") / "src" / "main" / "resources" / "static" / "images",
        std::filesystem::path("..") / "Users" / "src" / "main" / "resources" / "static" / "images",
        std::filesystem::path("..") / ".." / "Users" / "src" / "main" / "resources" / "static" / "images",
    };
    return staticFileFromRoots(relative_path, image_roots, imageContentType(relative_path));
}

size_t DiscussionHandler::parsePositiveSize(const char* value, size_t fallback, size_t max_value) {
    if (!value) {
        return fallback;
    }

    try {
        unsigned long parsed = std::stoul(value);
        if (parsed == 0) {
            return fallback;
        }
        return static_cast<size_t>(std::min<unsigned long>(parsed, max_value));
    } catch (...) {
        return fallback;
    }
}

bool DiscussionHandler::validateTopicPayload(const nlohmann::json& body) const {
    return body.contains("problem_id") &&
           (body.contains("username") || body.contains("user_id")) &&
           body.contains("title") &&
           body.contains("content");
}

bool DiscussionHandler::validateCommentPayload(const nlohmann::json& body) const {
    return (body.contains("username") || body.contains("user_id")) && body.contains("content");
}

crow::response DiscussionHandler::handleCreateTopic(const crow::request& req) {
    try {
        json body = json::parse(req.body);
        if (!validateTopicPayload(body)) {
            throw oj::HttpException(400, "bad_request", "missing required field in topic payload");
        }

        const std::string title = body["title"].get<std::string>();
        const std::string content = body["content"].get<std::string>();
        if (title.size() > 200) {
            throw oj::HttpException(400, "bad_request", "title too long, max length is 200");
        }
        if (content.size() > 5000) {
            throw oj::HttpException(400, "bad_request", "content too long, max length is 5000");
        }

        int64_t topic_id = 0;
        if (body.contains("username") && !body["username"].is_null()) {
            topic_id = service_.createTopicByUsername(
                body["problem_id"].get<int64_t>(),
                body["username"].get<std::string>(),
                title,
                content);
        } else {
            topic_id = service_.createTopic(
                body["problem_id"].get<int64_t>(),
                body["user_id"].get<int64_t>(),
                title,
                content);
        }

        auto topic = service_.getTopic(topic_id);
        if (!topic.has_value()) {
            throw oj::HttpException(500, "internal_error", "created topic cannot be loaded");
        }
        OJ_LOG_INFO("discussion topic created, topic_id=" + std::to_string(topic_id));

        json result;
        result["id"] = topic_id;
        result["problem_id"] = topic->problem_id;
        result["user_id"] = topic->user_id;
        result["username"] = topic->username;
        result["nickname"] = topic->nickname;
        result["avatar"] = topic->avatar;
        result["title"] = topic->title;
        result["content"] = topic->content;
        result["comment_count"] = topic->comment_count;
        result["created_at"] = topic->created_at;
        result["updated_at"] = topic->updated_at;
        return jsonResponse(201, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::out_of_range& e) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("user_not_found", e.what())));
    } catch (const std::exception& e) {
        OJ_LOG_ERROR(std::string("handleCreateTopic failed: ") + e.what());
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    }
}

crow::response DiscussionHandler::handleListTopics(const crow::request& req) {
    try {
        std::optional<int64_t> problem_id;
        const char* problem_id_param = req.url_params.get("problem_id");
        if (problem_id_param) {
            problem_id = std::stoll(problem_id_param);
        }

        const size_t limit = parsePositiveSize(req.url_params.get("limit"), 20, 200);
        const size_t offset = parsePositiveSize(req.url_params.get("offset"), 0, 50000);
        const auto topics = service_.listTopics(problem_id, limit, offset);

        json result = json::array();
        for (const auto& topic : topics) {
            json item;
            item["id"] = topic.id;
            item["problem_id"] = topic.problem_id;
            item["user_id"] = topic.user_id;
            item["username"] = topic.username;
            item["nickname"] = topic.nickname;
            item["avatar"] = topic.avatar;
            item["title"] = topic.title;
            item["content"] = topic.content;
            item["comment_count"] = topic.comment_count;
            item["created_at"] = topic.created_at;
            item["updated_at"] = topic.updated_at;
            result.push_back(item);
        }

        return jsonResponse(200, result);
    } catch (const std::exception& e) {
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    }
}

crow::response DiscussionHandler::handleGetTopic(const crow::request& req, int64_t topic_id) {
    (void)req;
    try {
        auto topic = service_.getTopic(topic_id);
        if (!topic.has_value()) {
            throw oj::HttpException(404, "topic_not_found", "topic does not exist");
        }

        json result;
        result["id"] = topic->id;
        result["problem_id"] = topic->problem_id;
        result["user_id"] = topic->user_id;
        result["username"] = topic->username;
        result["nickname"] = topic->nickname;
        result["avatar"] = topic->avatar;
        result["title"] = topic->title;
        result["content"] = topic->content;
        result["comment_count"] = topic->comment_count;
        result["created_at"] = topic->created_at;
        result["updated_at"] = topic->updated_at;
        return jsonResponse(200, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::exception& e) {
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleDeleteTopic(const crow::request& req, int64_t topic_id) {
    try {
        json body = req.body.empty() ? json::object() : json::parse(req.body);
        if (!body.contains("username") && !body.contains("user_id")) {
            throw oj::HttpException(400, "bad_request", "missing username or user_id");
        }

        int deleted_count = 0;
        if (body.contains("username") && !body["username"].is_null()) {
            deleted_count = service_.deleteTopicByUsername(topic_id, body["username"].get<std::string>());
        } else {
            deleted_count = service_.deleteTopic(topic_id, body["user_id"].get<int64_t>());
        }

        OJ_LOG_INFO("discussion topic deleted, topic_id=" + std::to_string(topic_id));

        json result;
        result["topic_id"] = topic_id;
        result["deleted_count"] = deleted_count;
        return jsonResponse(200, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::domain_error& e) {
        return jsonResponse(403, json::parse(oj::makeErrorJson("permission_denied", e.what())));
    } catch (const std::out_of_range& e) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("not_found", e.what())));
    } catch (const std::invalid_argument& e) {
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    } catch (const std::exception& e) {
        OJ_LOG_ERROR(std::string("handleDeleteTopic failed: ") + e.what());
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleCreateComment(const crow::request& req, int64_t topic_id) {
    try {
        json body = json::parse(req.body);
        if (!validateCommentPayload(body)) {
            throw oj::HttpException(400, "bad_request", "missing required field in comment payload");
        }

        const std::string content = body["content"].get<std::string>();
        if (content.size() > 3000) {
            throw oj::HttpException(400, "bad_request", "content too long, max length is 3000");
        }

        std::optional<int64_t> parent_comment_id;
        if (body.contains("parent_comment_id") && !body["parent_comment_id"].is_null()) {
            parent_comment_id = body["parent_comment_id"].get<int64_t>();
        }

        int64_t comment_id = 0;
        if (body.contains("username") && !body["username"].is_null()) {
            comment_id = service_.createCommentByUsername(
                topic_id,
                body["username"].get<std::string>(),
                content,
                parent_comment_id);
        } else {
            comment_id = service_.createComment(
                topic_id,
                body["user_id"].get<int64_t>(),
                content,
                parent_comment_id);
        }

        OJ_LOG_INFO("discussion comment created, topic_id=" + std::to_string(topic_id) +
                    " comment_id=" + std::to_string(comment_id));

        const auto comments = service_.listComments(topic_id);
        auto created_comment = std::find_if(
            comments.begin(), comments.end(), [comment_id](const DiscussionComment& comment) {
                return comment.id == comment_id;
            });
        if (created_comment == comments.end()) {
            throw oj::HttpException(500, "internal_error", "created comment cannot be loaded");
        }

        json result;
        result["id"] = comment_id;
        result["topic_id"] = topic_id;
        result["user_id"] = created_comment->user_id;
        result["username"] = created_comment->username;
        result["nickname"] = created_comment->nickname;
        result["avatar"] = created_comment->avatar;
        result["parent_comment_id"] = created_comment->parent_comment_id;
        result["content"] = created_comment->content;
        result["created_at"] = created_comment->created_at;
        return jsonResponse(201, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::out_of_range& e) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("not_found", e.what())));
    } catch (const std::invalid_argument& e) {
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    } catch (const std::exception& e) {
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleListComments(const crow::request& req, int64_t topic_id) {
    (void)req;
    try {
        const auto comments = service_.listComments(topic_id);

        json result = json::array();
        for (const auto& comment : comments) {
            json item;
            item["id"] = comment.id;
            item["topic_id"] = comment.topic_id;
            item["user_id"] = comment.user_id;
            item["username"] = comment.username;
            item["nickname"] = comment.nickname;
            item["avatar"] = comment.avatar;
            item["parent_comment_id"] = comment.parent_comment_id;
            item["content"] = comment.content;
            item["created_at"] = comment.created_at;
            result.push_back(item);
        }

        return jsonResponse(200, result);
    } catch (const std::out_of_range&) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("topic_not_found", "topic does not exist")));
    } catch (const std::exception& e) {
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleDeleteComment(const crow::request& req,
                                                      int64_t topic_id,
                                                      int64_t comment_id) {
    try {
        json body = req.body.empty() ? json::object() : json::parse(req.body);
        if (!body.contains("username") && !body.contains("user_id")) {
            throw oj::HttpException(400, "bad_request", "missing username or user_id");
        }

        int deleted_count = 0;
        if (body.contains("username") && !body["username"].is_null()) {
            deleted_count = service_.deleteCommentByUsername(
                topic_id,
                comment_id,
                body["username"].get<std::string>());
        } else {
            deleted_count = service_.deleteComment(
                topic_id,
                comment_id,
                body["user_id"].get<int64_t>());
        }

        OJ_LOG_INFO("discussion comment deleted, topic_id=" + std::to_string(topic_id) +
                    " comment_id=" + std::to_string(comment_id) +
                    " deleted_count=" + std::to_string(deleted_count));

        json result;
        result["topic_id"] = topic_id;
        result["comment_id"] = comment_id;
        result["deleted_count"] = deleted_count;
        return jsonResponse(200, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::domain_error& e) {
        return jsonResponse(403, json::parse(oj::makeErrorJson("permission_denied", e.what())));
    } catch (const std::out_of_range& e) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("not_found", e.what())));
    } catch (const std::invalid_argument& e) {
        return jsonResponse(400, json::parse(oj::makeErrorJson("bad_request", e.what())));
    } catch (const std::exception& e) {
        OJ_LOG_ERROR(std::string("handleDeleteComment failed: ") + e.what());
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleSummarizeTopic(const crow::request& req, int64_t topic_id) {
    (void)req;
    try {
        if (!GeminiClient::configured()) {
            throw oj::HttpException(503, "gemini_not_configured", "Gemini API key is not configured");
        }

        auto topic = service_.getTopic(topic_id);
        if (!topic.has_value()) {
            throw oj::HttpException(404, "topic_not_found", "topic does not exist");
        }

        const auto comments = service_.listComments(topic_id);
        const auto summary = gemini_client_.summarizeDiscussion(*topic, comments);

        json result;
        result["topic_id"] = topic_id;
        result["model"] = summary.model;
        result["summary"] = summary.summary;
        result["comment_count"] = comments.size();
        return jsonResponse(200, result);
    } catch (const oj::HttpException& e) {
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::out_of_range&) {
        return jsonResponse(404, json::parse(oj::makeErrorJson("topic_not_found", "topic does not exist")));
    } catch (const std::exception& e) {
        OJ_LOG_ERROR(std::string("handleSummarizeTopic failed: ") + e.what());
        return jsonResponse(502, json::parse(oj::makeErrorJson("gemini_request_failed", e.what())));
    }
}

void DiscussionHandler::startServer(uint16_t port) {
    app_->get_middleware<crow::CORSHandler>()
        .global()
        .origin("*")
        .methods("GET"_method,
                 "POST"_method,
                 "PUT"_method,
                 "DELETE"_method,
                 "OPTIONS"_method);

    OJ_LOG_INFO("discussion HTTP server starting, port=" + std::to_string(port));
    app_->port(port).multithreaded().run();
}
