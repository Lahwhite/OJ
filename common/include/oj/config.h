#pragma once

#include <string>

namespace oj {

struct AppConfig {
    std::string mysql_host = "127.0.0.1";
    unsigned mysql_port = 3306;
    std::string mysql_user = "oj";
    std::string mysql_password;
    std::string mysql_database = "oj";
    unsigned mysql_pool_min = 2;
    unsigned mysql_pool_max = 16;

    std::string redis_host = "127.0.0.1";
    unsigned redis_port = 6379;
    std::string redis_password;
    int redis_db = 0;

    std::string log_file;
    std::string log_level = "info";
};

AppConfig loadConfigFromEnv();

}  // namespace oj
