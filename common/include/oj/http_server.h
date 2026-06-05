#pragma once

#include "oj/router.h"

#include <atomic>
#include <cstdint>

namespace oj {

// 负责监听端口并把请求交给 Router 处理
class HttpServer {
public:
    HttpServer();
    ~HttpServer();

    Router& router();

    bool start(uint16_t port);
    void stop();

private:
    bool initSocketLayer();
    void cleanupSocketLayer();
    void serveLoop();

    std::atomic<bool> running_{false};
    int listen_fd_{-1};
    Router router_;
};

}  // namespace oj
