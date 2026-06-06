// common 模块头文件：声明公共组件对外暴露的接口与数据结构
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

    // 布尔返回值通常用于表示是否执行成功
    bool start(uint16_t port);
    // 过程型函数：主要通过副作用完成状态更新
    void stop();

private:
    // 布尔返回值通常用于表示是否执行成功
    bool initSocketLayer();
    // 过程型函数：主要通过副作用完成状态更新
    void cleanupSocketLayer();
    // 过程型函数：主要通过副作用完成状态更新
    void serveLoop();

    std::atomic<bool> running_{false};
    int listen_fd_{-1};
    Router router_;
};

}  // namespace oj
