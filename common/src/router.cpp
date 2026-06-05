// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/router.h"

#include "oj/http_response.h"
#include "oj/json_error.h"

namespace oj {

// 过程型函数：主要通过副作用完成状态更新
void Router::get(const std::string& path, Handler handler) {
    add("GET", path, std::move(handler));
}

// 过程型函数：主要通过副作用完成状态更新
void Router::post(const std::string& path, Handler handler) {
    add("POST", path, std::move(handler));
}

// 过程型函数：主要通过副作用完成状态更新
void Router::put(const std::string& path, Handler handler) {
    add("PUT", path, std::move(handler));
}

// 过程型函数：主要通过副作用完成状态更新
void Router::del(const std::string& path, Handler handler) {
    add("DELETE", path, std::move(handler));
}

HttpResponse Router::dispatch(const HttpRequest& request) const {
    const auto key = routeKey(request.method, request.path);
    auto it = routes_.find(key);
    // 条件判断：根据运行时状态决定后续流程
    if (it == routes_.end()) {
        // 返回当前阶段的处理结果或默认兜底值
        return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "route not found"));
    }

    try {
        // 返回当前阶段的处理结果或默认兜底值
        return it->second(request);
    } catch (...) {
        // 返回当前阶段的处理结果或默认兜底值
        return HttpResponse::json(500, makeErrorJson("INTERNAL_ERROR", "handler failed"));
    }
}

// 过程型函数：主要通过副作用完成状态更新
void Router::add(const std::string& method, const std::string& path, Handler handler) {
    routes_[routeKey(method, path)] = std::move(handler);
}

std::string Router::routeKey(const std::string& method, const std::string& path) {
    // 返回当前阶段的处理结果或默认兜底值
    return method + " " + path; // "METHOD /path" 格式，查找是 O(1)
}

}  // namespace oj
