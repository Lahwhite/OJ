#include "oj/config.h"

#include <cstdlib>
#include <stdexcept>

namespace oj {

static std::string getenvOr(const char* key, const std::string& def) {
#if defined(_WIN32)
    char* buf = nullptr;
    size_t sz = 0;
    if (_dupenv_s(&buf, &sz, key) == 0 && buf) {
        std::string v(buf);
        std::free(buf);
        return v;
    }
    return def;
#else
    const char* v = std::getenv(key);
    return v ? std::string(v) : def;
#endif
}

static unsigned getenvUInt(const char* key, unsigned def) {
    std::string s = getenvOr(key, "");
    if (s.empty()) {
        return def;
    }
    return static_cast<unsigned>(std::stoul(s));
}

AppConfig loadConfigFromEnv() {
    AppConfig c;
    c.mysql_host = getenvOr("OJ_MYSQL_HOST", c.mysql_host);
    c.mysql_port = getenvUInt("OJ_MYSQL_PORT", c.mysql_port);
    c.mysql_user = getenvOr("OJ_MYSQL_USER", c.mysql_user);
    c.mysql_password = getenvOr("OJ_MYSQL_PASSWORD", c.mysql_password);
    c.mysql_database = getenvOr("OJ_MYSQL_DATABASE", c.mysql_database);
    c.mysql_pool_min = getenvUInt("OJ_MYSQL_POOL_MIN", c.mysql_pool_min);
    c.mysql_pool_max = getenvUInt("OJ_MYSQL_POOL_MAX", c.mysql_pool_max);

    c.redis_host = getenvOr("OJ_REDIS_HOST", c.redis_host);
    c.redis_port = getenvUInt("OJ_REDIS_PORT", c.redis_port);
    c.redis_password = getenvOr("OJ_REDIS_PASSWORD", c.redis_password);
    c.redis_db = static_cast<int>(getenvUInt("OJ_REDIS_DB", static_cast<unsigned>(c.redis_db)));

    c.log_file = getenvOr("OJ_LOG_FILE", c.log_file);
    c.log_level = getenvOr("OJ_LOG_LEVEL", c.log_level);
    return c;
}

}  // namespace oj
