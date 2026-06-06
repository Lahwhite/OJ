#include "storage/mysql_c_wrapper.hpp"
#include "oj/log.h"

#include <cstdlib>
#include <mysql/mysql.h>

namespace oj {

static std::string getenv_str(const char* key, const std::string& def) {
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

static unsigned getenv_uint(const char* key, unsigned def) {
    const std::string s = getenv_str(key, "");
    if (s.empty()) return def;
    return static_cast<unsigned>(std::stoul(s));
}

MysqlCConfig load_mysql_config() {
    MysqlCConfig cfg;
    cfg.host = getenv_str("OJ_MYSQL_HOST", cfg.host);
    cfg.port = getenv_uint("OJ_MYSQL_PORT", cfg.port);
    cfg.user = getenv_str("OJ_MYSQL_USER", cfg.user);
    cfg.password = getenv_str("OJ_MYSQL_PASSWORD", cfg.password);
    cfg.database = getenv_str("OJ_MYSQL_DATABASE", cfg.database);
    return cfg;
}

MysqlCConnection::MysqlCConnection(const MysqlCConfig& cfg) {
    conn_ = mysql_init(nullptr);
    if (!conn_) {
        OJ_LOG_ERROR("mysql_c: mysql_init failed");
        return;
    }
    if (!mysql_real_connect(conn_, cfg.host.c_str(), cfg.user.c_str(),
                            cfg.password.c_str(), cfg.database.c_str(),
                            cfg.port, nullptr, 0)) {
        OJ_LOG_ERROR(std::string("mysql_c: connect failed: ") + mysql_error(conn_));
        mysql_close(conn_);
        conn_ = nullptr;
        return;
    }
    OJ_LOG_INFO("mysql_c: connected to " + cfg.host + ":" + std::to_string(cfg.port) +
                "/" + cfg.database);
}

MysqlCConnection::~MysqlCConnection() {
    if (conn_) {
        mysql_close(conn_);
    }
}

}  // namespace oj
