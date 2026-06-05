#pragma once

#include <optional>
#include <string>

namespace oj {

// Redis 这里主要是做一个很薄的封装，常用的 set/get/del 够用了
class RedisCache {
public:
    static RedisCache& instance();

    bool connect(const std::string& host,
                  unsigned port,
                  const std::string& password,
                  int db);

    void disconnect();
    bool connected() const { return connected_; }

    bool set(const std::string& key, const std::string& value, int ttl_seconds = 0);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);

private:
    RedisCache() = default;
    ~RedisCache();

    void* ctx_{nullptr};
    bool connected_{false};
};

}  // namespace oj
