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

void Router::get_prefix(const std::string& prefix, Handler handler) {
    prefix_routes_.emplace_back(prefix, std::move(handler));
}

HttpResponse Router::dispatch(const HttpRequest& request) const {
    const auto key = routeKey(request.method, request.path);
    auto it = routes_.find(key);
    if (it != routes_.end()) {
        try {
            return it->second(request);
        } catch (const HttpException& e) {
            return HttpResponse::json(e.http_status, makeErrorJson(e.error_code, e.what()));
        } catch (...) {
            return HttpResponse::json(500, makeErrorJson("INTERNAL_ERROR", "handler failed"));
        }
    }

    if (request.method == "GET") {
        const Handler* best = nullptr;
        std::size_t best_len = 0;
        for (const auto& [prefix, handler] : prefix_routes_) {
            if (request.path.size() < prefix.size()) {
                continue;
            }
            if (request.path.compare(0, prefix.size(), prefix) != 0) {
                continue;
            }
            if (request.path.size() > prefix.size() && prefix.back() != '/' &&
                request.path[prefix.size()] != '/') {
                continue;
            }
            if (prefix.size() >= best_len) {
                best_len = prefix.size();
                best = &handler;
            }
        }
        if (best != nullptr) {
            try {
                return (*best)(request);
            } catch (const HttpException& e) {
                return HttpResponse::json(e.http_status, makeErrorJson(e.error_code, e.what()));
            } catch (...) {
                return HttpResponse::json(500, makeErrorJson("INTERNAL_ERROR", "handler failed"));
            }
        }
    }

    return HttpResponse::json(404, makeErrorJson("NOT_FOUND", "route not found"));
}

void Router::add(const std::string& method, const std::string& path, Handler handler) {
    routes_[routeKey(method, path)] = std::move(handler);
}

std::string Router::routeKey(const std::string& method, const std::string& path) {
    return method + " " + path;
}

}  // namespace oj
