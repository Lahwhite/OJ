#pragma once

#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

namespace oj {

struct MysqlPoolStats {
    size_t pool_size{0};
    size_t in_use{0};
};

class MysqlConnectionPool {
public:
    static MysqlConnectionPool& instance();

    bool initialize(const std::string& host,
                    unsigned port,
                    const std::string& user,
                    const std::string& password,
                    const std::string& database,
                    size_t pool_min,
                    size_t pool_max);

    void shutdown();

    void* acquire();
    void release(void* conn);

    bool available() const { return initialized_.load(); }
    MysqlPoolStats stats() const;

private:
    MysqlConnectionPool() = default;
    ~MysqlConnectionPool();

    bool createConnection(void*& out_conn);
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
