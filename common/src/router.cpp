#include "oj/router.h"

#include "oj/http_response.h"
#include "oj/json_error.h"

namespace oj {

void Router::get(const std::string& path, Handler handler) {
    add("GET", path, std::move(handler));
}

void Router::post(const std::string& path, Handler handler) {
    add("POST", path, std::move(handler));
}

void Router::put(const std::string& path, Handler handler) {
    add("PUT", path, std::move(handler));
}

void Router::del(const std::string& path, Handler handler) {
    add("DELETE", path, std::move(handler));
}

HttpResponse Router::dispatch(const HttpRequest& request) const {
    const auto key = routeKey(request.method, request.path);
    auto it = routes_.find(key);
    if (it == routes_.end()) {
        return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "route not found"));
    }

    try {
        return it->second(request);
    } catch (...) {
        return HttpResponse::json(500, makeErrorJson("INTERNAL_ERROR", "handler failed"));
    }
}

void Router::add(const std::string& method, const std::string& path, Handler handler) {
    routes_[routeKey(method, path)] = std::move(handler);
}

std::string Router::routeKey(const std::string& method, const std::string& path) {
    return method + " " + path;
}

}  // namespace oj
