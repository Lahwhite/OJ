#pragma once

#include <string>
#include <unordered_map>

namespace oj {

struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string header(const std::string& key) const;
};

}  // namespace oj
