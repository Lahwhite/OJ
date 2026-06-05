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
    // Crow 应用在构造阶段完成路由注册，startServer 只负责绑定端口和启动。
    app_ = std::make_unique<crow::Crow<crow::CORSHandler>>();

    // 讨论区单页入口，所有前端交互都由 web/app.js 再调用后端 API。
    CROW_ROUTE((*app_), "/discussion").methods("GET"_method)([] {
        return webFileResponse("index.html", "text/html; charset=utf-8");
    });

    // 静态资源只开放讨论区 web 目录下的文件，避免暴露运行目录里的其他内容。
    CROW_ROUTE((*app_), "/discussion/static/<path>").methods("GET"_method)([](const std::string& file_path) {
        std::string content_type = "text/plain; charset=utf-8";
        // 根据后缀返回浏览器需要的 MIME 类型，当前页面只依赖 CSS 和 JS。
        if (file_path.size() >= 4 && file_path.substr(file_path.size() - 4) == ".css") {
            content_type = "text/css; charset=utf-8";
        } else if (file_path.size() >= 3 && file_path.substr(file_path.size() - 3) == ".js") {
            content_type = "application/javascript; charset=utf-8";
        }
        return webFileResponse(file_path, content_type);
    });

    // 用户模块上传的头像通过讨论区服务代理，解决跨端部署时静态路径不一致的问题。
    CROW_ROUTE((*app_), "/uploads/avatars/<path>").methods("GET"_method)([](const std::string& file_path) {
        return userAvatarResponse(file_path);
    });

    // 默认头像等用户模块图片也允许从讨论区域名访问，前端无需判断服务来源。
    CROW_ROUTE((*app_), "/images/<path>").methods("GET"_method)([](const std::string& file_path) {
        return userImageResponse(file_path);
    });

    // 创建主题，支持 username 或 user_id 两种身份字段。
    CROW_ROUTE((*app_), "/api/discussions/topics").methods("POST"_method)([this](const crow::request& req) {
        return handleCreateTopic(req);
    });

    // 列表接口支持 limit/offset 分页和可选 problem_id 过滤。
    CROW_ROUTE((*app_), "/api/discussions/topics").methods("GET"_method)([this](const crow::request& req) {
        return handleListTopics(req);
    });

    // 主题详情只返回主题本身，评论由独立接口加载，方便前端并发请求。
    CROW_ROUTE((*app_), "/api/discussions/topics/<int>").methods("GET"_method)(
        [this](const crow::request& req, int topic_id) { return handleGetTopic(req, topic_id); });

    // 删除主题会同时删除评论树，权限在 service 层按作者校验。
    CROW_ROUTE((*app_), "/api/discussions/topics/<int>").methods("DELETE"_method)(
        [this](const crow::request& req, int topic_id) { return handleDeleteTopic(req, topic_id); });

    // 评论创建接口用路径里的 topic_id 固定归属，body 里只传正文和可选父评论。
    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments").methods("POST"_method)(
        [this](const crow::request& req, int topic_id) { return handleCreateComment(req, topic_id); });

    // 评论按创建顺序返回，前端根据 parent_comment_id 展示回复关系。
    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments").methods("GET"_method)(
        [this](const crow::request& req, int topic_id) { return handleListComments(req, topic_id); });

    // 删除某条评论时会级联删除它的子回复，并返回实际删除数量。
    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/comments/<int>").methods("DELETE"_method)(
        [this](const crow::request& req, int topic_id, int comment_id) {
            return handleDeleteComment(req, topic_id, comment_id);
        });

    // AI 总结只在用户主动点击时触发，避免列表加载时消耗外部 API 配额。
    CROW_ROUTE((*app_), "/api/discussions/topics/<int>/summary").methods("POST"_method)(
        [this](const crow::request& req, int topic_id) { return handleSummarizeTopic(req, topic_id); });

    // 健康检查同时暴露依赖状态，方便启动脚本和人工排查环境问题。
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
    // 所有 API 响应统一走 JSON，前端 requestJson 会按这个格式解析错误信息。
    crow::response response(status, body.dump());
    response.add_header("Content-Type", "application/json");
    return response;
}

crow::response DiscussionHandler::webFileResponse(const std::string& relative_path, const std::string& content_type) {
    // 禁止目录穿越、绝对路径和 Windows 盘符，防止静态文件接口读出任意文件。
    if (relative_path.find("..") != std::string::npos ||
        relative_path.find(':') != std::string::npos ||
        (!relative_path.empty() && (relative_path.front() == '/' || relative_path.front() == '\\'))) {
        return crow::response(404);
    }

#ifndef DISCUSSION_WEB_DIR
#define DISCUSSION_WEB_DIR "discussion/web"
#endif
    // 同一可执行文件可能从源码目录、构建目录或 runtime_package 启动，所以准备多个候选根。
    const std::vector<std::filesystem::path> web_roots = {
        std::filesystem::path("web"),
        std::filesystem::path("discussion") / "web",
        std::filesystem::path(DISCUSSION_WEB_DIR),
    };

    std::filesystem::path path = std::filesystem::path(DISCUSSION_WEB_DIR) / relative_path;
    for (const auto& root : web_roots) {
        const auto candidate = root / relative_path;
        std::error_code ec;
        // 使用 error_code 版本避免缺文件时抛异常，静态资源缺失应只返回 404。
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
    // 头像和图片代理也复用同一套相对路径安全规则。
    return !relative_path.empty() &&
           relative_path.find("..") == std::string::npos &&
           relative_path.find(':') == std::string::npos &&
           relative_path.front() != '/' &&
           relative_path.front() != '\\';
}

std::string imageContentType(const std::string& relative_path) {
    // 图片代理需要正确 Content-Type，否则浏览器可能把头像当作下载文件处理。
    const auto dot = relative_path.find_last_of('.');
    if (dot == std::string::npos) {
        return "application/octet-stream";
    }

    std::string ext = relative_path.substr(dot);
    // 扩展名大小写不影响识别，便于兼容用户上传的不同文件名。
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
        // 静态资源路径非法时返回 404，避免向外透露本地文件系统规则。
        return crow::response(404);
    }

    for (const auto& root : roots) {
        const auto candidate = root / relative_path;
        std::error_code ec;
        // 按候选根顺序查找，优先使用当前运行目录中的 runtime_package 资源。
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
    // 兼容从 discussion 目录、项目根目录、runtime_package 等不同位置启动服务。
    const std::vector<std::filesystem::path> avatar_roots = {
        std::filesystem::path("uploads") / "avatars",
        std::filesystem::path("Users") / "uploads" / "avatars",
        std::filesystem::path("..") / "Users" / "uploads" / "avatars",
        std::filesystem::path("..") / ".." / "Users" / "uploads" / "avatars",
    };
    return staticFileFromRoots(relative_path, avatar_roots, imageContentType(relative_path));
}

crow::response DiscussionHandler::userImageResponse(const std::string& relative_path) {
    // 默认头像在用户模块 resources/static/images 下，本服务提供一个轻量代理入口。
    const std::vector<std::filesystem::path> image_roots = {
        std::filesystem::path("images"),
        std::filesystem::path("Users") / "src" / "main" / "resources" / "static" / "images",
        std::filesystem::path("..") / "Users" / "src" / "main" / "resources" / "static" / "images",
        std::filesystem::path("..") / ".." / "Users" / "src" / "main" / "resources" / "static" / "images",
    };
    return staticFileFromRoots(relative_path, image_roots, imageContentType(relative_path));
}

size_t DiscussionHandler::parsePositiveSize(const char* value, size_t fallback, size_t max_value) {
    // 分页参数在 HTTP 层做上限裁剪，避免一次请求拉取过多主题。
    if (!value) {
        return fallback;
    }

    try {
        unsigned long parsed = std::stoul(value);
        if (parsed == 0) {
            return fallback;
        }
        // max_value 是服务端保护值，调用方传更大数字也不会突破。
        return static_cast<size_t>(std::min<unsigned long>(parsed, max_value));
    } catch (...) {
        // 非数字参数按默认值处理，让列表接口对手输 URL 更宽容。
        return fallback;
    }
}

bool DiscussionHandler::validateTopicPayload(const nlohmann::json& body) const {
    // 身份字段允许 username 或 user_id，其余业务长度限制交给具体 handler 和 service。
    return body.contains("problem_id") &&
           (body.contains("username") || body.contains("user_id")) &&
           body.contains("title") &&
           body.contains("content");
}

bool DiscussionHandler::validateCommentPayload(const nlohmann::json& body) const {
    // 评论不需要 problem_id，topic_id 已经由路径参数确定。
    return (body.contains("username") || body.contains("user_id")) && body.contains("content");
}

crow::response DiscussionHandler::handleCreateTopic(const crow::request& req) {
    try {
        // 这里直接解析 JSON，解析失败会被下面的 bad_request 捕获。
        json body = json::parse(req.body);
        if (!validateTopicPayload(body)) {
            throw oj::HttpException(400, "bad_request", "missing required field in topic payload");
        }

        // HTTP 层提前做长度检查，可以返回更贴近用户输入的错误信息。
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
            // 浏览器入口优先使用 username，避免把内部 user_id 暴露给前端状态。
            topic_id = service_.createTopicByUsername(
                body["problem_id"].get<int64_t>(),
                body["username"].get<std::string>(),
                title,
                content);
        } else {
            // user_id 分支保留给测试或后端内部调用。
            topic_id = service_.createTopic(
                body["problem_id"].get<int64_t>(),
                body["user_id"].get<int64_t>(),
                title,
                content);
        }

        auto topic = service_.getTopic(topic_id);
        if (!topic.has_value()) {
            // 创建成功后立即回查，确保返回给前端的头像和昵称已经关联完整。
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
        // 显式抛出的 HTTP 异常保留原状态码。
        return jsonResponse(e.http_status, json::parse(oj::makeErrorJson(e.error_code, e.what())));
    } catch (const std::out_of_range& e) {
        // 目前 create topic 的 out_of_range 主要来自用户不存在。
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
            // problem_id 是可选过滤条件，不传时展示全部讨论。
            problem_id = std::stoll(problem_id_param);
        }

        // limit 和 offset 都经过 parsePositiveSize 兜底，避免非法分页参数击穿服务。
        const size_t limit = parsePositiveSize(req.url_params.get("limit"), 20, 200);
        const size_t offset = parsePositiveSize(req.url_params.get("offset"), 0, 50000);
        const auto topics = service_.listTopics(problem_id, limit, offset);

        json result = json::array();
        for (const auto& topic : topics) {
            // 列表页也返回 content 摘要原文，前端用 line-clamp 展示预览。
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
        // 详情接口不读取 body，topic_id 完全来自路径。
        auto topic = service_.getTopic(topic_id);
        if (!topic.has_value()) {
            throw oj::HttpException(404, "topic_not_found", "topic does not exist");
        }

        json result;
        // 字段名保持和列表接口一致，前端切换主题时可以复用渲染函数。
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
        // DELETE 请求允许空 body，但真正删除必须携带身份字段。
        json body = req.body.empty() ? json::object() : json::parse(req.body);
        if (!body.contains("username") && !body.contains("user_id")) {
            throw oj::HttpException(400, "bad_request", "missing username or user_id");
        }

        int deleted_count = 0;
        if (body.contains("username") && !body["username"].is_null()) {
            // 前端传 username，服务层会转换成 user_id 并校验作者权限。
            deleted_count = service_.deleteTopicByUsername(topic_id, body["username"].get<std::string>());
        } else {
            // user_id 分支主要方便服务端测试和内部调用。
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
        // service 层用 domain_error 表示权限不足。
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
        // 评论 body 只包含身份、正文和可选父评论，主题由 URL 确定。
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
            // parent_comment_id 为空表示顶层评论，非空时 service 会校验它属于当前主题。
            parent_comment_id = body["parent_comment_id"].get<int64_t>();
        }

        int64_t comment_id = 0;
        if (body.contains("username") && !body["username"].is_null()) {
            // 浏览器发评论不需要知道 user_id。
            comment_id = service_.createCommentByUsername(
                topic_id,
                body["username"].get<std::string>(),
                content,
                parent_comment_id);
        } else {
            // 保留 user_id 入口，方便服务端测试直接构造请求。
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
            // 回查刚创建的评论，补全昵称和头像后再返回给前端。
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
        // 评论列表不分页，当前内容长度和评论长度限制足以控制响应体大小。
        const auto comments = service_.listComments(topic_id);

        json result = json::array();
        for (const auto& comment : comments) {
            // parent_comment_id 为 0 时前端会当作顶层评论展示。
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
        // service 层会把非法 topic_id 和不存在主题都归为 out_of_range。
        return jsonResponse(404, json::parse(oj::makeErrorJson("topic_not_found", "topic does not exist")));
    } catch (const std::exception& e) {
        return jsonResponse(500, json::parse(oj::makeErrorJson("internal_error", e.what())));
    }
}

crow::response DiscussionHandler::handleDeleteComment(const crow::request& req,
                                                      int64_t topic_id,
                                                      int64_t comment_id) {
    try {
        // 删除评论同样需要身份字段，防止只知道 comment_id 就能删除内容。
        json body = req.body.empty() ? json::object() : json::parse(req.body);
        if (!body.contains("username") && !body.contains("user_id")) {
            throw oj::HttpException(400, "bad_request", "missing username or user_id");
        }

        int deleted_count = 0;
        if (body.contains("username") && !body["username"].is_null()) {
            // username 分支给前端使用，权限仍由 service 层按作者 id 判断。
            deleted_count = service_.deleteCommentByUsername(
                topic_id,
                comment_id,
                body["username"].get<std::string>());
        } else {
            // user_id 分支保留给后端调用。
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
        // 没有配置密钥时直接返回 503，让前端显示“稍后再试”而不是卡住。
        if (!GeminiClient::configured()) {
            throw oj::HttpException(503, "gemini_not_configured", "Gemini API key is not configured");
        }

        // 先加载主题，再加载评论，保证提示词中的上下文完整。
        auto topic = service_.getTopic(topic_id);
        if (!topic.has_value()) {
            throw oj::HttpException(404, "topic_not_found", "topic does not exist");
        }

        const auto comments = service_.listComments(topic_id);
        // 外部 API 调用集中在 GeminiClient，handler 只负责组装 HTTP 响应。
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
        // AI 服务失败属于上游依赖错误，返回 502 便于和本地业务错误区分。
        OJ_LOG_ERROR(std::string("handleSummarizeTopic failed: ") + e.what());
        return jsonResponse(502, json::parse(oj::makeErrorJson("gemini_request_failed", e.what())));
    }
}

void DiscussionHandler::startServer(uint16_t port) {
    // 讨论区通常会从用户模块页面跳转过来，CORS 放开方便本地多端口联调。
    app_->get_middleware<crow::CORSHandler>()
        .global()
        .origin("*")
        .methods("GET"_method,
                 "POST"_method,
                 "PUT"_method,
                 "DELETE"_method,
                 "OPTIONS"_method);

    // 绑定 0.0.0.0 方便局域网访问，启动脚本会提示实际访问地址。
    OJ_LOG_INFO("discussion HTTP server starting, bindaddr=0.0.0.0, port=" + std::to_string(port));
    app_->bindaddr("0.0.0.0").port(port).multithreaded().run();
}
