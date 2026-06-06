#pragma once

#include <string>

struct st_mysql;
using MYSQL = st_mysql;

namespace oj {

struct MysqlCConfig {
    std::string host = "127.0.0.1";
    unsigned port = 3306;
    std::string user = "oj";
    std::string password;
    std::string database = "myOJ";
};

MysqlCConfig load_mysql_config();

class MysqlCConnection {
public:
    MysqlCConnection(const MysqlCConfig& cfg);
    ~MysqlCConnection();

    MYSQL* get() { return conn_; }
    bool connected() const { return conn_ != nullptr; }

    MysqlCConnection(const MysqlCConnection&) = delete;
    MysqlCConnection& operator=(const MysqlCConnection&) = delete;

private:
    MYSQL* conn_ = nullptr;
};

}  // namespace oj
