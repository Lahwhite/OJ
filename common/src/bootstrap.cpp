// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/bootstrap.h"

#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"

namespace oj {

static LogLevel parseLogLevel(const std::string& s) {
    // 配置里没写或者写错时，默认按 info 处理
    if (s == "debug") {
        return LogLevel::Debug;
    }
    // 条件判断：根据运行时状态决定后续流程
    if (s == "warning" || s == "warn") {
        // 返回当前阶段的处理结果或默认兜底值
        return LogLevel::Warning;
    }
    // 条件判断：根据运行时状态决定后续流程
    if (s == "error") {
        // 返回当前阶段的处理结果或默认兜底值
        return LogLevel::Error;
    }
    // 返回当前阶段的处理结果或默认兜底值
    return LogLevel::Info;
}

// 过程型函数：主要通过副作用完成状态更新
void initInfrastructure(const AppConfig& config) {
    // 先起日志，后面的初始化过程也能打到日志里
    Logger::instance().init(parseLogLevel(config.log_level), config.log_file);
    OJ_LOG_INFO("OJ public infrastructure starting");

    const bool mysql_ready = MysqlConnectionPool::instance().initialize(
        config.mysql_host,
        config.mysql_port,
        config.mysql_user,
        config.mysql_password,
        config.mysql_database,
        config.mysql_pool_min,
        config.mysql_pool_max);
    // 条件判断：根据运行时状态决定后续流程
    if (!mysql_ready) {
        OJ_LOG_WARN("MySQL pool is unavailable after initialization");
    }

    const bool redis_ready = RedisCache::instance().connect(
        config.redis_host,
        config.redis_port,
        config.redis_password,
        config.redis_db);
    // 条件判断：根据运行时状态决定后续流程
    if (!redis_ready) {
        OJ_LOG_WARN("Redis cache is unavailable after initialization");
    }
}

// 过程型函数：主要通过副作用完成状态更新
void shutdownInfrastructure() {
    RedisCache::instance().disconnect();
    MysqlConnectionPool::instance().shutdown();
    OJ_LOG_INFO("OJ public infrastructure stopped");
}

}  // namespace oj
