// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include <optional>
#include <string>

namespace oj {

// Redis 这里主要是做一个很薄的封装，常用的 set/get/del 够用了
class RedisCache {
public:
    static RedisCache& instance();

    // 布尔返回值通常用于表示是否执行成功
    bool connect(const std::string& host,
                  unsigned port,
                  const std::string& password,
                  int db);

    // 过程型函数：主要通过副作用完成状态更新
    void disconnect();
    // 布尔返回值通常用于表示是否执行成功
    bool connected() const { return connected_; }

    // 布尔返回值通常用于表示是否执行成功
    bool set(const std::string& key, const std::string& value, int ttl_seconds = 0);
    std::optional<std::string> get(const std::string& key);
    // 布尔返回值通常用于表示是否执行成功
    bool del(const std::string& key);

private:
    RedisCache() = default;
    ~RedisCache();

    void* ctx_{nullptr};
    // 布尔返回值通常用于表示是否执行成功
    bool connected_{false};
};

}  // namespace oj
