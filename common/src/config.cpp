// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/config.h"

#include <cstdlib>
#include <stdexcept>

namespace oj {

namespace {
constexpr const char* kEnvMysqlHost = "OJ_MYSQL_HOST";
constexpr const char* kEnvMysqlPort = "OJ_MYSQL_PORT";
constexpr const char* kEnvMysqlUser = "OJ_MYSQL_USER";
constexpr const char* kEnvMysqlPassword = "OJ_MYSQL_PASSWORD";
constexpr const char* kEnvMysqlDatabase = "OJ_MYSQL_DATABASE";
constexpr const char* kEnvMysqlPoolMin = "OJ_MYSQL_POOL_MIN";
constexpr const char* kEnvMysqlPoolMax = "OJ_MYSQL_POOL_MAX";

constexpr const char* kEnvRedisHost = "OJ_REDIS_HOST";
constexpr const char* kEnvRedisPort = "OJ_REDIS_PORT";
constexpr const char* kEnvRedisPassword = "OJ_REDIS_PASSWORD";
constexpr const char* kEnvRedisDb = "OJ_REDIS_DB";

constexpr const char* kEnvLogFile = "OJ_LOG_FILE";
constexpr const char* kEnvLogLevel = "OJ_LOG_LEVEL";
}  // namespace

// Windows 下 getenv 不安全，用 _dupenv_s 代替
// Linux 直接用标准库的 getenv 就行
static std::string getenvOr(const char* key, const std::string& def) {
#if defined(_WIN32)
    char* buf = nullptr;
    size_t sz = 0;
    // 条件判断：根据运行时状态决定后续流程
    if (_dupenv_s(&buf, &sz, key) == 0 && buf) {
        std::string v(buf);
        std::free(buf);
        // 返回当前阶段的处理结果或默认兜底值
        return v;
    }
    // 返回当前阶段的处理结果或默认兜底值
    return def;
#else
    const char* v = std::getenv(key);
    // 返回当前阶段的处理结果或默认兜底值
    return v ? std::string(v) : def;
#endif
}

static unsigned getenvUInt(const char* key, unsigned def) {
    // 这里统一把数字配置从字符串转成无符号整数
    std::string s = getenvOr(key, "");
    if (s.empty()) {
        // 返回当前阶段的处理结果或默认兜底值
        return def;
    }
    // 返回当前阶段的处理结果或默认兜底值
    return static_cast<unsigned>(std::stoul(s));
}

AppConfig loadConfigFromEnv() {
    // 默认配置先兜底，再让环境变量覆盖
    AppConfig c;
    c.mysql_host = getenvOr(kEnvMysqlHost, c.mysql_host);
    c.mysql_port = getenvUInt(kEnvMysqlPort, c.mysql_port);
    c.mysql_user = getenvOr(kEnvMysqlUser, c.mysql_user);
    c.mysql_password = getenvOr(kEnvMysqlPassword, c.mysql_password);
    c.mysql_database = getenvOr(kEnvMysqlDatabase, c.mysql_database);
    c.mysql_pool_min = getenvUInt(kEnvMysqlPoolMin, c.mysql_pool_min);
    c.mysql_pool_max = getenvUInt(kEnvMysqlPoolMax, c.mysql_pool_max);

    c.redis_host = getenvOr(kEnvRedisHost, c.redis_host);
    c.redis_port = getenvUInt(kEnvRedisPort, c.redis_port);
    c.redis_password = getenvOr(kEnvRedisPassword, c.redis_password);
    c.redis_db = static_cast<int>(getenvUInt(kEnvRedisDb, static_cast<unsigned>(c.redis_db)));

    c.log_file = getenvOr(kEnvLogFile, c.log_file);
    c.log_level = getenvOr(kEnvLogLevel, c.log_level);
    // 返回当前阶段的处理结果或默认兜底值
    return c;
}

}  // namespace oj
