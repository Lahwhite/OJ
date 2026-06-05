// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/http_server.h"

#include "oj/http_response.h"
#include "oj/json_error.h"
#include "oj/log.h"

#include <chrono>
#include <cstring>
#include <sstream>
#include <string>
#include <thread>
#include <unordered_map>

#if defined(_WIN32)
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

namespace oj {

namespace {

std::string trim(const std::string& s) {
    // header 解析时会用到，主要去掉前后空白
    size_t start = 0;
    while (start < s.size() && (s[start] == ' ' || s[start] == '\t' || s[start] == '\r')) {
        ++start;
    }
    size_t end = s.size();
    // 循环处理：持续消费输入或等待条件满足
    while (end > start && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r')) {
        --end;
    }
    // 返回当前阶段的处理结果或默认兜底值
    return s.substr(start, end - start);
}

int toSocketLen(size_t n) {
    // 返回当前阶段的处理结果或默认兜底值
    return static_cast<int>(n);
}

// 过程型函数：主要通过副作用完成状态更新
void closeSocket(int fd) {
#if defined(_WIN32)
    closesocket(static_cast<SOCKET>(fd));
#else
    close(fd);
#endif
}

}  // namespace

HttpServer::HttpServer() = default;

HttpServer::~HttpServer() {
    stop();
}

Router& HttpServer::router() {
    // 返回当前阶段的处理结果或默认兜底值
    return router_;
}

// 布尔返回值通常用于表示是否执行成功
bool HttpServer::start(uint16_t port) {
    // 这个实现先走最直白的阻塞式监听逻辑
    if (running_.load()) {
        OJ_LOG_WARN("http server already running");
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }

    // 条件判断：根据运行时状态决定后续流程
    if (!initSocketLayer()) {
        OJ_LOG_ERROR("http server socket layer init failed");
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }

    listen_fd_ = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    // 条件判断：根据运行时状态决定后续流程
    if (listen_fd_ < 0) {
        OJ_LOG_ERROR("http server create socket failed");
        cleanupSocketLayer();
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR,
               reinterpret_cast<const char*>(&opt), sizeof(opt));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // 条件判断：根据运行时状态决定后续流程
    if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
        OJ_LOG_ERROR("http server bind failed");
        closeSocket(listen_fd_);
        listen_fd_ = -1;
        cleanupSocketLayer();
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }

    // 条件判断：根据运行时状态决定后续流程
    if (::listen(listen_fd_, 64) != 0) {
        OJ_LOG_ERROR("http server listen failed");
        closeSocket(listen_fd_);
        listen_fd_ = -1;
        cleanupSocketLayer();
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }

    running_.store(true);
    OJ_LOG_INFO("http server started on port " + std::to_string(port));
    serveLoop();
    // 返回当前阶段的处理结果或默认兜底值
    return true;
}

// 过程型函数：主要通过副作用完成状态更新
void HttpServer::stop() {
    // 条件判断：根据运行时状态决定后续流程
    if (!running_.load() && listen_fd_ < 0) {
        return;
    }
    running_.store(false);

    // 条件判断：根据运行时状态决定后续流程
    if (listen_fd_ >= 0) {
        closeSocket(listen_fd_);
        listen_fd_ = -1;
    }

    cleanupSocketLayer();
    OJ_LOG_INFO("http server stopped");
}

// 布尔返回值通常用于表示是否执行成功
bool HttpServer::initSocketLayer() {
#if defined(_WIN32)
    WSADATA wsaData;
    // 返回当前阶段的处理结果或默认兜底值
    return WSAStartup(MAKEWORD(2, 2), &wsaData) == 0;
#else
    // 返回当前阶段的处理结果或默认兜底值
    return true;
#endif
}

// 过程型函数：主要通过副作用完成状态更新
void HttpServer::cleanupSocketLayer() {
#if defined(_WIN32)
    WSACleanup();
#endif
}

// 过程型函数：主要通过副作用完成状态更新
void HttpServer::serveLoop() {
    // 主循环只负责 accept，请求处理扔给子线程
    while (running_.load()) {
        sockaddr_in client_addr{};
#if defined(_WIN32)
        int len = sizeof(client_addr);
#else
        socklen_t len = sizeof(client_addr);
#endif
        const int client_fd = static_cast<int>(::accept(
            listen_fd_, reinterpret_cast<sockaddr*>(&client_addr), &len));

        // 条件判断：根据运行时状态决定后续流程
        if (client_fd < 0) {
            // 条件判断：根据运行时状态决定后续流程
            if (running_.load()) {
                OJ_LOG_WARN("http server accept failed");
            }
            continue;
        }

        std::thread([this, client_fd]() {
            auto startTime = std::chrono::steady_clock::now();
            std::string requestRaw;
            char buffer[4096];
            int r = 0;

#if defined(_WIN32)
            r = recv(static_cast<SOCKET>(client_fd), buffer, sizeof(buffer), 0);
#else
            r = recv(client_fd, buffer, sizeof(buffer), 0);
#endif
            // 条件判断：根据运行时状态决定后续流程
            if (r > 0) {
                requestRaw.assign(buffer, buffer + r);
            }

            HttpRequest req;
            HttpResponse res;
            // 条件判断：根据运行时状态决定后续流程
            if (requestRaw.empty()) {
                res = HttpResponse::json(400, makeErrorJson("BAD_REQUEST", "empty request"));
            } else {
                size_t lineEnd = requestRaw.find("\r\n");
                std::string firstLine = lineEnd == std::string::npos ? requestRaw : requestRaw.substr(0, lineEnd);
                std::istringstream first(firstLine);
                std::string uri;
                first >> req.method >> uri;

                size_t q = uri.find('?');
                req.path = q == std::string::npos ? uri : uri.substr(0, q);
                req.query = q == std::string::npos ? "" : uri.substr(q + 1);

                size_t headerStart = lineEnd == std::string::npos ? std::string::npos : lineEnd + 2;
                size_t bodyStart = requestRaw.find("\r\n\r\n");
                // 条件判断：根据运行时状态决定后续流程
                if (bodyStart != std::string::npos) {
                    bodyStart += 4;
                    req.body = requestRaw.substr(bodyStart);
                }

                // 条件判断：根据运行时状态决定后续流程
                if (headerStart != std::string::npos) {
                    size_t cursor = headerStart;
                    // 循环处理：持续消费输入或等待条件满足
                    while (cursor < requestRaw.size()) {
                        size_t next = requestRaw.find("\r\n", cursor);
                        // 条件判断：根据运行时状态决定后续流程
                        if (next == std::string::npos || next == cursor) {
                            break;
                        }
                        std::string line = requestRaw.substr(cursor, next - cursor);
                        size_t c = line.find(':');
                        // 条件判断：根据运行时状态决定后续流程
                        if (c != std::string::npos) {
                            std::string key = trim(line.substr(0, c));
                            std::string value = trim(line.substr(c + 1));
                            req.headers[key] = value;
                        }
                        cursor = next + 2;
                    }
                }

                // 条件判断：根据运行时状态决定后续流程
                if (req.method.empty() || req.path.empty()) {
                    res = HttpResponse::json(400, makeErrorJson("BAD_REQUEST", "invalid request line"));
                } else {
                    res = router_.dispatch(req);
                }
            }

            const std::string wire = res.toHttpString();
#if defined(_WIN32)
            send(static_cast<SOCKET>(client_fd), wire.c_str(), toSocketLen(wire.size()), 0);
#else
            send(client_fd, wire.c_str(), wire.size(), 0);
#endif

            closeSocket(client_fd);
            auto endTime = std::chrono::steady_clock::now();
            auto costMs = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
            OJ_LOG_INFO("http " + req.method + " " + req.path + " -> " + std::to_string(res.status) +
                        " " + std::to_string(costMs) + "ms");
        }).detach();
    }
}

}  // namespace oj
