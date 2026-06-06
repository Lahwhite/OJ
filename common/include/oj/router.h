// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include "oj/http_request.h"
#include "oj/http_response.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace oj {

// method + path 映射到一个 handler，先保持实现简单
class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    // 先支持最常见的四种方法
    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    // 过程型函数：主要通过副作用完成状态更新
    void put(const std::string& path, Handler handler);
    // 过程型函数：主要通过副作用完成状态更新
    void del(const std::string& path, Handler handler);

    /** Match paths that start with prefix (longest prefix wins after exact routes). */
    void get_prefix(const std::string& prefix, Handler handler);

    HttpResponse dispatch(const HttpRequest& request) const;

private:
    // 过程型函数：主要通过副作用完成状态更新
    void add(const std::string& method, const std::string& path, Handler handler);
    static std::string routeKey(const std::string& method, const std::string& path);

    std::unordered_map<std::string, Handler> routes_;
    std::vector<std::pair<std::string, Handler>> prefix_routes_;
};

}  // namespace oj
