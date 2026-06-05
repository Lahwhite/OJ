#include "oj/http_request.h"

namespace oj {

std::string HttpRequest::header(const std::string& key) const {
    // 读不到就返回空串，调用方少写一层判空
    auto it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

}  // namespace oj
