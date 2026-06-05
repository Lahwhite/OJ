// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include <string>

namespace oj {

// MySQL 和 Redis 的默认配置，实际部署时通过环境变量覆盖
inline constexpr const char* kDefaultMysqlHost = "127.0.0.1";
inline constexpr unsigned kDefaultMysqlPort = 3306;
inline constexpr const char* kDefaultMysqlUser = "oj";
inline constexpr const char* kDefaultMysqlDatabase = "myOJ";
// 连接池大小：最少保持 2 个连接避免频繁创建，最多 16 个防止占用过多资源
inline constexpr unsigned kDefaultMysqlPoolMin = 2;
inline constexpr unsigned kDefaultMysqlPoolMax = 16;

inline constexpr const char* kDefaultRedisHost = "127.0.0.1";
inline constexpr unsigned kDefaultRedisPort = 6379;
inline constexpr int kDefaultRedisDb = 0;

inline constexpr const char* kDefaultLogLevel = "info";

// 结构定义：用于组织一组紧密相关的数据
struct AppConfig {
    std::string mysql_host = kDefaultMysqlHost;
    unsigned mysql_port = kDefaultMysqlPort;
    std::string mysql_user = kDefaultMysqlUser;
    std::string mysql_password;
    std::string mysql_database = kDefaultMysqlDatabase;
    unsigned mysql_pool_min = kDefaultMysqlPoolMin;
    unsigned mysql_pool_max = kDefaultMysqlPoolMax;

    std::string redis_host = kDefaultRedisHost;
    // Redis 配置
    unsigned redis_port = kDefaultRedisPort;
    std::string redis_password;
    int redis_db = kDefaultRedisDb;

    std::string log_file;
    std::string log_level = kDefaultLogLevel;
};

AppConfig loadConfigFromEnv();

}  // namespace oj
