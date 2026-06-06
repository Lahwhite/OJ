#include "discussion_handler.h"
#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/log.h"

#include <cstdlib>
#include <iostream>
#include <string>

namespace {
void shutdownAtExit() {
    // 进程退出时统一关闭 MySQL/Redis 等公共基础设施。
    oj::shutdownInfrastructure();
}

uint16_t loadPortFromEnv() {
#if defined(_WIN32)
    // Windows 使用 _dupenv_s 读取环境变量，避免 CRT 安全告警。
    char* value = nullptr;
    size_t len = 0;
    if (_dupenv_s(&value, &len, "OJ_DISCUSSION_PORT") == 0 && value) {
        try {
            const int parsed = std::stoi(value);
            std::free(value);
            if (parsed > 0 && parsed <= 65535) {
                return static_cast<uint16_t>(parsed);
            }
        } catch (...) {
            // 端口配置错误时回退默认端口，启动脚本会在更早阶段做严格校验。
            std::free(value);
            return 8090;
        }
    }
    return 8090;
#else
    // 非 Windows 编译保留 getenv 分支，方便开发环境直接运行。
    const char* value = std::getenv("OJ_DISCUSSION_PORT");
    if (!value) {
        return 8090;
    }
    try {
        const int parsed = std::stoi(value);
        if (parsed > 0 && parsed <= 65535) {
            // uint16_t 是 Crow 端口参数需要的类型。
            return static_cast<uint16_t>(parsed);
        }
    } catch (...) {
    }
    return 8090;
#endif
}
}  // namespace

int main() {
    try {
        // 公共配置来自环境变量，和用户、判题等模块保持同一套初始化流程。
        oj::AppConfig cfg = oj::loadConfigFromEnv();
        oj::initInfrastructure(cfg);
        std::atexit(shutdownAtExit);

        // DiscussionHandler 构造时已经注册全部路由。
        DiscussionHandler handler;
        const uint16_t port = loadPortFromEnv();
        std::cout << "Discussion server starting on port " << port << "..." << std::endl;
        OJ_LOG_INFO("discussion_server process started");
        handler.startServer(port);
    } catch (const std::exception& e) {
        // 启动阶段失败直接写 stderr，便于脚本和命令行看到错误。
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
