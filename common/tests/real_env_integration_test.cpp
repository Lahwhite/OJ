#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/http_response.h"
#include "oj/http_server.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

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

namespace {

bool expect(bool cond, const std::string& msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << std::endl;
        return false;
    }
    std::cout << "[PASS] " << msg << std::endl;
    return true;
}

std::string readWholeFile(const std::filesystem::path& p) {
    std::ifstream in(p);
    return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
}

bool tcpPortOpen(const std::string& host, uint16_t port) {
#if defined(_WIN32)
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return false;
    }
#endif
    const int fd = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    if (fd < 0) {
#if defined(_WIN32)
        WSACleanup();
#endif
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host.c_str());

    const bool open = ::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == 0;
#if defined(_WIN32)
    closesocket(static_cast<SOCKET>(fd));
    WSACleanup();
#else
    close(fd);
#endif
    return open;
}

std::string httpGet(const std::string& host, uint16_t port, const std::string& path) {
#if defined(_WIN32)
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
    const int fd = static_cast<int>(::socket(AF_INET, SOCK_STREAM, 0));
    if (fd < 0) {
        return {};
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(host.c_str());
    if (::connect(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
#if defined(_WIN32)
        closesocket(static_cast<SOCKET>(fd));
        WSACleanup();
#else
        close(fd);
#endif
        return {};
    }

    const std::string req = "GET " + path + " HTTP/1.1\r\nHost: " + host + "\r\nConnection: close\r\n\r\n";
#if defined(_WIN32)
    send(static_cast<SOCKET>(fd), req.c_str(), static_cast<int>(req.size()), 0);
#else
    send(fd, req.c_str(), req.size(), 0);
#endif

    std::string response;
    char buf[4096];
    for (;;) {
#if defined(_WIN32)
        const int n = recv(static_cast<SOCKET>(fd), buf, sizeof(buf), 0);
#else
        const int n = recv(fd, buf, sizeof(buf), 0);
#endif
        if (n <= 0) {
            break;
        }
        response.append(buf, buf + n);
    }

#if defined(_WIN32)
    closesocket(static_cast<SOCKET>(fd));
    WSACleanup();
#else
    close(fd);
#endif
    return response;
}

bool testBootstrapLogToFile(const std::filesystem::path& log_path) {
    std::filesystem::remove(log_path);

    oj::AppConfig cfg = oj::loadConfigFromEnv();
    cfg.log_file = log_path.string();
    cfg.log_level = "info";

    oj::initInfrastructure(cfg);
    OJ_LOG_INFO("real env integration marker");
    oj::shutdownInfrastructure();
    oj::Logger::instance().init(oj::LogLevel::Info);

    const std::string content = readWholeFile(log_path);
    const bool ok = expect(std::filesystem::exists(log_path), "bootstrap creates configured log file") &&
                    expect(content.find("OJ public infrastructure starting") != std::string::npos,
                           "bootstrap startup message written to log file") &&
                    expect(content.find("real env integration marker") != std::string::npos,
                           "runtime log message written to log file") &&
                    expect(content.find("OJ public infrastructure stopped") != std::string::npos,
                           "bootstrap shutdown message written to log file");
    std::filesystem::remove(log_path);
    return ok;
}

bool testHttpServerLive() {
    constexpr uint16_t kPort = 18080;
    oj::HttpServer server;
    server.router().get("/health", [](const oj::HttpRequest&) {
        return oj::HttpResponse::json(200, "{\"status\":\"ok\"}");
    });
    server.router().post("/echo", [](const oj::HttpRequest& req) {
        return oj::HttpResponse::json(200, req.body.empty() ? "{\"echo\":\"empty\"}" : req.body);
    });

    std::thread server_thread([&server]() { server.start(kPort); });
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    const std::string health = httpGet("127.0.0.1", kPort, "/health");
    const bool health_ok =
        expect(!health.empty(), "http server accepts TCP connection") &&
        expect(health.find("HTTP/1.1 200") != std::string::npos, "GET /health returns HTTP 200") &&
        expect(health.find("{\"status\":\"ok\"}") != std::string::npos, "GET /health returns expected JSON body");

    const std::string missing = httpGet("127.0.0.1", kPort, "/missing");
    const bool missing_ok = expect(missing.find("HTTP/1.1 404") != std::string::npos,
                                   "unknown route returns HTTP 404 JSON");

    server.stop();
    if (server_thread.joinable()) {
        server_thread.join();
    }
    return health_ok && missing_ok;
}

bool testExternalServicesReachability() {
    const bool mysql_port = tcpPortOpen("127.0.0.1", 3306);
    const bool redis_port = tcpPortOpen("127.0.0.1", 6379);

    expect(mysql_port, "MySQL port 3306 is reachable on localhost");
    expect(!redis_port, "Redis port 6379 is not reachable (expected if Redis not installed/running)");

    oj::AppConfig cfg = oj::loadConfigFromEnv();
    oj::initInfrastructure(cfg);

#ifdef OJ_WITH_MYSQL
    const bool mysql_pool = oj::MysqlConnectionPool::instance().available();
    expect(mysql_pool, "MySQL connection pool initialized with OJ_WITH_MYSQL enabled");
#else
    expect(!oj::MysqlConnectionPool::instance().available(),
           "MySQL pool gracefully degraded (OJ_WITH_MYSQL not enabled at build time)");
#endif

#ifdef OJ_WITH_REDIS
    const bool redis_cache = oj::RedisCache::instance().connected();
    expect(redis_cache, "Redis cache connected with OJ_WITH_REDIS enabled");
#else
    expect(!oj::RedisCache::instance().connected(),
           "Redis cache gracefully degraded (OJ_WITH_REDIS not enabled at build time)");
#endif

    oj::shutdownInfrastructure();
    oj::Logger::instance().init(oj::LogLevel::Info);
    return true;
}

}  // namespace

int main() {
    const auto log_path =
        std::filesystem::temp_directory_path() / "oj_common_real_env_integration.log";

    bool ok = true;
    ok = testBootstrapLogToFile(log_path) && ok;
    ok = testHttpServerLive() && ok;
    ok = testExternalServicesReachability() && ok;

    if (!ok) {
        std::cerr << "Real environment integration tests failed." << std::endl;
        return 1;
    }
    std::cout << "Real environment integration tests passed." << std::endl;
    return 0;
}
