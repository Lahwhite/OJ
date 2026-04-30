#include "oj/http_request.h"

namespace oj {

std::string HttpRequest::header(const std::string& key) const {
    auto it = headers.find(key);
    if (it != headers.end()) {
        return it->second;
    }
    return "";
}

}  // namespace oj
