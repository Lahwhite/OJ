// common 模块实现文件：负责公共能力的具体实现与底层细节
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
    // 返回当前阶段的处理结果或默认兜底值
    return pool;
}

MysqlConnectionPool::~MysqlConnectionPool() {
    shutdown();
}

// 布尔返回值通常用于表示是否执行成功
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
    // 返回当前阶段的处理结果或默认兜底值
    return false;
#else
    std::lock_guard<std::mutex> lock(mutex_);
    // 条件判断：根据运行时状态决定后续流程
    if (initialized_) {
        // 返回当前阶段的处理结果或默认兜底值
        return true;
    }
    // 条件判断：根据运行时状态决定后续流程
    if (pool_max == 0 || pool_min > pool_max) {
        OJ_LOG_ERROR("MySQL pool: invalid pool size");
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
    host_ = host;
    port_ = port;
    user_ = user;
    password_ = password;
    database_ = database;
    pool_max_ = pool_max;

    // 循环处理：逐项遍历并构造结果或执行操作
    for (size_t i = 0; i < pool_min; ++i) {
        // 提前建好最小连接数，避免第一批请求现建现等
        void* c = nullptr;
        if (!createConnection(c) || !c) {
            OJ_LOG_ERROR("MySQL pool: failed to create initial connection");
            // 循环处理：持续消费输入或等待条件满足
            while (!idle_.empty()) {
                void* x = idle_.front();
                idle_.pop();
                destroyConnection(x);
            }
            total_created_ = 0;
            // 返回当前阶段的处理结果或默认兜底值
            return false;
        }
        idle_.push(c);
        total_created_++;
    }
    initialized_ = true;
    OJ_LOG_INFO("MySQL pool: initialized, min=" + std::to_string(pool_min) +
                " max=" + std::to_string(pool_max));
    // 返回当前阶段的处理结果或默认兜底值
    return true;
#endif
}

// 过程型函数：主要通过副作用完成状态更新
void MysqlConnectionPool::shutdown() {
#ifndef OJ_WITH_MYSQL
    return;
#else
    std::queue<void*> q;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // 条件判断：根据运行时状态决定后续流程
        if (!initialized_) {
            return;
        }
        initialized_ = false;
        q.swap(idle_);
    }
    // 循环处理：持续消费输入或等待条件满足
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
    // 返回当前阶段的处理结果或默认兜底值
    return nullptr;
#else
    std::unique_lock<std::mutex> lock(mutex_);
    // 条件判断：根据运行时状态决定后续流程
    if (!initialized_) {
        // 返回当前阶段的处理结果或默认兜底值
        return nullptr;
    }
    // 循环等待可用连接，要么从 idle 池里取，要么新建一个
    for (;;) {
        while (!idle_.empty()) {
            void* c = idle_.front();
            idle_.pop();
            // 返回当前阶段的处理结果或默认兜底值
            return c;
        }
        // 池子用完了，但还没达到上限，尝试新建
        if (total_created_ < pool_max_) {
            void* c = nullptr;
            lock.unlock();
            // 布尔返回值通常用于表示是否执行成功
            bool ok = createConnection(c);
            lock.lock();
            // 条件判断：根据运行时状态决定后续流程
            if (ok && c) {
                total_created_++;
                // 返回当前阶段的处理结果或默认兜底值
                return c;
            }
            OJ_LOG_ERROR("MySQL pool: createConnection failed under load");
            // 返回当前阶段的处理结果或默认兜底值
            return nullptr;
        }
        // 等不到就超时返回 null
        if (cv_.wait_for(lock, std::chrono::seconds(30)) == std::cv_status::timeout) {
            OJ_LOG_WARN("MySQL pool: acquire timeout");
            // 返回当前阶段的处理结果或默认兜底值
            return nullptr;
        }
    }
#endif
}

// 过程型函数：主要通过副作用完成状态更新
void MysqlConnectionPool::release(void* conn) {
#ifndef OJ_WITH_MYSQL
    (void)conn;
#else
    // 条件判断：根据运行时状态决定后续流程
    if (!conn) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    // 条件判断：根据运行时状态决定后续流程
    if (!initialized_) {
        destroyConnection(conn);
        return;
    }
    idle_.push(conn);
    cv_.notify_one();
#endif
}

MysqlPoolStats MysqlConnectionPool::stats() const {
    // 这里只给出最基础的池状态统计
    MysqlPoolStats s;
    std::lock_guard<std::mutex> lock(mutex_);
    const size_t idle = idle_.size();
    const size_t total = total_created_.load();
    s.pool_size = idle;
    s.in_use = total > idle ? total - idle : 0;
    // 返回当前阶段的处理结果或默认兜底值
    return s;
}

#ifndef OJ_WITH_MYSQL

// 布尔返回值通常用于表示是否执行成功
bool MysqlConnectionPool::createConnection(void*& out_conn) {
    out_conn = nullptr;
    // 返回当前阶段的处理结果或默认兜底值
    return false;
}

// 过程型函数：主要通过副作用完成状态更新
void MysqlConnectionPool::destroyConnection(void* conn) {
    (void)conn;
}

#else

// 布尔返回值通常用于表示是否执行成功
bool MysqlConnectionPool::createConnection(void*& out_conn) {
    out_conn = nullptr;
    try {
        sql::Driver* driver = get_driver_instance();
        std::string url = "tcp://" + host_ + ":" + std::to_string(port_);
        std::unique_ptr<sql::Connection> con(
            driver->connect(url, user_, password_));
        con->setSchema(database_);
        out_conn = con.release();
        // 返回当前阶段的处理结果或默认兜底值
        return true;
    } catch (sql::SQLException& e) {
        OJ_LOG_ERROR(std::string("MySQL connect: ") + e.what());
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
}

// 过程型函数：主要通过副作用完成状态更新
void MysqlConnectionPool::destroyConnection(void* conn) {
    // 条件判断：根据运行时状态决定后续流程
    if (!conn) {
        return;
    }
    delete static_cast<sql::Connection*>(conn);
}

#endif

}  // namespace oj
