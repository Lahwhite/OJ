#pragma once

#include "oj/http_request.h"
#include "oj/http_response.h"

#include <functional>
#include <string>
#include <unordered_map>

namespace oj {

// method + path 映射到一个 handler，先保持实现简单
class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    // 先支持最常见的四种方法
    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    void put(const std::string& path, Handler handler);
    void del(const std::string& path, Handler handler);

    HttpResponse dispatch(const HttpRequest& request) const;

private:
    void add(const std::string& method, const std::string& path, Handler handler);
    static std::string routeKey(const std::string& method, const std::string& path);

    std::unordered_map<std::string, Handler> routes_;
};

}  // namespace oj
