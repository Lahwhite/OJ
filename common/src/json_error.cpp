#include "oj/json_error.h"

#include <sstream>

namespace oj {

std::string makeErrorJson(const std::string& error_code, const std::string& message) {
    std::ostringstream oss;
    oss << "{\"error_code\":\"" << error_code << "\",\"message\":\"";
    for (char c : message) {
        if (c == '"' || c == '\\') {
            oss << '\\';
        }
        oss << c;
    }
    oss << "\"}";
    return oss.str();
}

}  // namespace oj
