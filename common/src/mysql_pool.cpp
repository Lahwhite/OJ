#include "oj/mysql_pool.h"

#include "oj/log.h"

#include <chrono>
#include <stdexcept>

#ifdef OJ_WITH_MYSQL
#if __has_include(<mysql_driver.h>)
#include <mysql_driver.h>
#elif __has_include(<jdbc/mysql_driver.h>)
#include <jdbc/mysql_driver.h>
#endif
#include <cppconn/driver.h>
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#endif

namespace oj {

MysqlConnectionPool& MysqlConnectionPool::instance() {
    static MysqlConnectionPool pool;
    return pool;
}

MysqlConnectionPool::~MysqlConnectionPool() {
    shutdown();
}

bool MysqlConnectionPool::initialize(const std::string& host,
                                       unsigned port,
                                       const std::string& user,
                                       const std::string& password,
                                       const std::string& database,
                                       size_t pool_min,
                                       size_t pool_max) {
#ifndef OJ_WITH_MYSQL
    (void)host;
    (void)port;
    (void)user;
    (void)password;
    (void)database;
    (void)pool_min;
    (void)pool_max;
    OJ_LOG_WARN("MySQL pool: built without OJ_WITH_MYSQL; pool disabled");
    return false;
#else
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_) {
        return true;
    }
    if (pool_max == 0 || pool_min > pool_max) {
        OJ_LOG_ERROR("MySQL pool: invalid pool size");
        return false;
    }
    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    database_ = database;
    pool_max_ = pool_max;

    for (size_t i = 0; i < pool_min; ++i) {
        void* c = nullptr;
        if (!createConnection(c) || !c) {
            OJ_LOG_ERROR("MySQL pool: failed to create initial connection");
            while (!idle_.empty()) {
                void* x = idle_.front();
                idle_.pop();
                destroyConnection(x);
            }
            total_created_ = 0;
            return false;
        }
        idle_.push(c);
        total_created_++;
    }
    initialized_ = true;
    OJ_LOG_INFO("MySQL pool: initialized, min=" + std::to_string(pool_min) +
                " max=" + std::to_string(pool_max));
    return true;
#endif
}

void MysqlConnectionPool::shutdown() {
#ifndef OJ_WITH_MYSQL
    return;
#else
    std::queue<void*> q;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!initialized_) {
            return;
        }
        initialized_ = false;
        q.swap(idle_);
    }
    while (!q.empty()) {
        destroyConnection(q.front());
        q.pop();
    }
    total_created_ = 0;
    OJ_LOG_INFO("MySQL pool: shutdown complete");
#endif
}

void* MysqlConnectionPool::acquire() {
#ifndef OJ_WITH_MYSQL
    return nullptr;
#else
    std::unique_lock<std::mutex> lock(mutex_);
    if (!initialized_) {
        return nullptr;
    }
    for (;;) {
        while (!idle_.empty()) {
            void* c = idle_.front();
            idle_.pop();
            return c;
        }
        if (total_created_ < pool_max_) {
            void* c = nullptr;
            lock.unlock();
            bool ok = createConnection(c);
            lock.lock();
            if (ok && c) {
                total_created_++;
                return c;
            }
            OJ_LOG_ERROR("MySQL pool: createConnection failed under load");
            return nullptr;
        }
        if (cv_.wait_for(lock, std::chrono::seconds(30)) == std::cv_status::timeout) {
            OJ_LOG_WARN("MySQL pool: acquire timeout");
            return nullptr;
        }
    }
#endif
}

void MysqlConnectionPool::release(void* conn) {
#ifndef OJ_WITH_MYSQL
    (void)conn;
#else
    if (!conn) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!initialized_) {
        destroyConnection(conn);
        return;
    }
    idle_.push(conn);
    cv_.notify_one();
#endif
}

MysqlPoolStats MysqlConnectionPool::stats() const {
    MysqlPoolStats s;
    std::lock_guard<std::mutex> lock(mutex_);
    const size_t idle = idle_.size();
    const size_t total = total_created_.load();
    s.pool_size = idle;
    s.in_use = total > idle ? total - idle : 0;
    return s;
}

#ifndef OJ_WITH_MYSQL

bool MysqlConnectionPool::createConnection(void*& out_conn) {
    out_conn = nullptr;
    return false;
}

void MysqlConnectionPool::destroyConnection(void* conn) {
    (void)conn;
}

#else

bool MysqlConnectionPool::createConnection(void*& out_conn) {
    out_conn = nullptr;
    try {
        sql::Driver* driver = get_driver_instance();
        std::string url = "tcp://" + host_ + ":" + std::to_string(port_);
        std::unique_ptr<sql::Connection> con(
            driver->connect(url, user_, password_));
        con->setSchema(database_);
        out_conn = con.release();
        return true;
    } catch (sql::SQLException& e) {
        OJ_LOG_ERROR(std::string("MySQL connect: ") + e.what());
        return false;
    }
}

void MysqlConnectionPool::destroyConnection(void* conn) {
    if (!conn) {
        return;
    }
    delete static_cast<sql::Connection*>(conn);
}

#endif

}  // namespace oj
