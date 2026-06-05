#pragma once

#include "oj/http_request.h"
#include "oj/http_response.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace oj {

class Router {
public:
    using Handler = std::function<HttpResponse(const HttpRequest&)>;

    void get(const std::string& path, Handler handler);
    void post(const std::string& path, Handler handler);
    void put(const std::string& path, Handler handler);
    void del(const std::string& path, Handler handler);

    /** Match paths that start with prefix (longest prefix wins after exact routes). */
    void get_prefix(const std::string& prefix, Handler handler);

    HttpResponse dispatch(const HttpRequest& request) const;

private:
    void add(const std::string& method, const std::string& path, Handler handler);
    static std::string routeKey(const std::string& method, const std::string& path);

    std::unordered_map<std::string, Handler> routes_;
    std::vector<std::pair<std::string, Handler>> prefix_routes_;
};

}  // namespace oj
