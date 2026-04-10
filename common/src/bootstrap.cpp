#include "oj/bootstrap.h"

#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

namespace oj {

static LogLevel parseLogLevel(const std::string& s) {
    if (s == "debug") {
        return LogLevel::Debug;
    }
    if (s == "warning" || s == "warn") {
        return LogLevel::Warning;
    }
    if (s == "error") {
        return LogLevel::Error;
    }
    return LogLevel::Info;
}

void initInfrastructure(const AppConfig& config) {
    Logger::instance().init(parseLogLevel(config.log_level), config.log_file);
    OJ_LOG_INFO("OJ public infrastructure starting");

    MysqlConnectionPool::instance().initialize(
        config.mysql_host,
        config.mysql_port,
        config.mysql_user,
        config.mysql_password,
        config.mysql_database,
        config.mysql_pool_min,
        config.mysql_pool_max);

    RedisCache::instance().connect(
        config.redis_host,
        config.redis_port,
        config.redis_password,
        config.redis_db);
}

void shutdownInfrastructure() {
    RedisCache::instance().disconnect();
    MysqlConnectionPool::instance().shutdown();
    OJ_LOG_INFO("OJ public infrastructure stopped");
}

}  // namespace oj
