// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace oj {

// 结构定义：用于组织一组紧密相关的数据
struct MysqlPoolStats {
    size_t pool_size{0};
    size_t in_use{0};
};

// 单例连接池，进程内共享
class MysqlConnectionPool {
public:
    static MysqlConnectionPool& instance();

    // 布尔返回值通常用于表示是否执行成功
    bool initialize(const std::string& host,
                    unsigned port,
                    const std::string& user,
                    const std::string& password,
                    const std::string& database,
                    size_t pool_min,
                    size_t pool_max);

    // 过程型函数：主要通过副作用完成状态更新
    void shutdown();

    // 连接的借出和归还都走这两个接口
    void* acquire();
    void release(void* conn);

    // 布尔返回值通常用于表示是否执行成功
    bool available() const { return initialized_.load(); }
    MysqlPoolStats stats() const;

private:
    MysqlConnectionPool() = default;
    ~MysqlConnectionPool();

    // 布尔返回值通常用于表示是否执行成功
    bool createConnection(void*& out_conn);
    // 过程型函数：主要通过副作用完成状态更新
    void destroyConnection(void* conn);

    mutable std::mutex mutex_;
    std::condition_variable cv_;
    std::queue<void*> idle_;
    std::atomic<bool> initialized_{false};
    std::atomic<size_t> total_created_{0};
    size_t pool_max_{0};
    std::string host_;
    unsigned port_{0};
    std::string user_;
    std::string password_;
    std::string database_;
};

}  // namespace oj
