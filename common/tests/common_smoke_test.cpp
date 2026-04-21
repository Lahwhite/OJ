#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/json_error.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

namespace {

struct ScopedEnvVar {
    std::string key;
    std::string original_value;
    bool had_original{false};

    ScopedEnvVar(std::string env_key, std::string value) : key(std::move(env_key)) {
        if (const char* existing = std::getenv(key.c_str())) {
            had_original = true;
            original_value = existing;
        }
#if defined(_WIN32)
        _putenv_s(key.c_str(), value.c_str());
#else
        setenv(key.c_str(), value.c_str(), 1);
#endif
    }

    ~ScopedEnvVar() {
#if defined(_WIN32)
        _putenv_s(key.c_str(), had_original ? original_value.c_str() : "");
#else
        if (had_original) {
            setenv(key.c_str(), original_value.c_str(), 1);
        } else {
            unsetenv(key.c_str());
        }
#endif
    }
};

bool expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "[FAIL] " << message << std::endl;
        return false;
    }
    std::cout << "[PASS] " << message << std::endl;
    return true;
}

bool testLoadConfigFromEnv() {
    ScopedEnvVar mysql_host("OJ_MYSQL_HOST", "192.168.0.10");
    ScopedEnvVar mysql_port("OJ_MYSQL_PORT", "3307");
    ScopedEnvVar mysql_user("OJ_MYSQL_USER", "judge_user");
    ScopedEnvVar redis_db("OJ_REDIS_DB", "5");
    ScopedEnvVar log_level("OJ_LOG_LEVEL", "debug");

    const oj::AppConfig cfg = oj::loadConfigFromEnv();
    return expect(cfg.mysql_host == "192.168.0.10", "config reads mysql host from env") &&
           expect(cfg.mysql_port == 3307, "config reads mysql port from env") &&
           expect(cfg.mysql_user == "judge_user", "config reads mysql user from env") &&
           expect(cfg.redis_db == 5, "config reads redis db from env") &&
           expect(cfg.log_level == "debug", "config reads log level from env");
}

bool testMakeErrorJson() {
    const std::string json = oj::makeErrorJson("bad\ncode", "line1\n\"quoted\"\\tail\t");
    return expect(json == "{\"error_code\":\"bad\\ncode\",\"message\":\"line1\\n\\\"quoted\\\"\\\\tail\\t\"}",
                  "error json escapes control characters correctly");
}

bool testLoggerWritesFile() {
    namespace fs = std::filesystem;
    const fs::path log_path = fs::temp_directory_path() / "oj_common_smoke.log";
    fs::remove(log_path);

    oj::Logger::instance().init(oj::LogLevel::Info, log_path.string());
    oj::Logger::instance().info("common smoke logger test");
    oj::Logger::instance().init(oj::LogLevel::Info);

    std::string content;
    {
        std::ifstream in(log_path);
        content.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }

    const bool ok = expect(fs::exists(log_path), "logger creates output file") &&
                    expect(content.find("common smoke logger test") != std::string::npos,
                           "logger writes message to file");
    fs::remove(log_path);
    return ok;
}

bool testBootstrapWithoutOptionalBackends() {
    namespace fs = std::filesystem;
    const fs::path log_path = fs::temp_directory_path() / "oj_bootstrap_smoke.log";
    fs::remove(log_path);

    oj::AppConfig cfg;
    cfg.log_level = "debug";
    cfg.log_file = log_path.string();
    oj::initInfrastructure(cfg);

    const bool mysql_disabled = expect(!oj::MysqlConnectionPool::instance().available(),
                                       "mysql pool stays disabled when mysql support is off");
    const bool redis_disabled = expect(!oj::RedisCache::instance().connected(),
                                       "redis cache stays disabled when redis support is off");

    oj::shutdownInfrastructure();
    oj::Logger::instance().init(oj::LogLevel::Info);

    std::string content;
    {
        std::ifstream in(log_path);
        content.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }

    const bool log_written = expect(content.find("OJ public infrastructure starting") != std::string::npos,
                                    "bootstrap writes startup log");
    fs::remove(log_path);
    return mysql_disabled && redis_disabled && log_written;
}

}  // namespace

int main() {
    bool ok = true;
    ok = testLoadConfigFromEnv() && ok;
    ok = testMakeErrorJson() && ok;
    ok = testLoggerWritesFile() && ok;
    ok = testBootstrapWithoutOptionalBackends() && ok;

    if (!ok) {
        std::cerr << "Common smoke tests failed." << std::endl;
        return 1;
    }

    std::cout << "Common smoke tests passed." << std::endl;
    return 0;
}
