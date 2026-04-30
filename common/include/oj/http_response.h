#pragma once

#include <string>
#include <unordered_map>

namespace oj {

class HttpResponse {
public:
    int status{200};
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    static HttpResponse text(int status, const std::string& body,
                             const std::string& contentType = "text/plain; charset=utf-8");
    static HttpResponse json(int status, const std::string& body);

    std::string toHttpString() const;

private:
    static const char* statusText(int status);
};

}  // namespace oj
