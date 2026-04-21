#include "oj/json_error.h"

#include <sstream>

namespace oj {

std::string makeErrorJson(const std::string& error_code, const std::string& message) {
    auto appendJsonString = [](std::ostringstream& out, const std::string& value) {
        for (unsigned char c : value) {
            switch (c) {
                case '"': out << "\\\""; break;
                case '\\': out << "\\\\"; break;
                case '\b': out << "\\b"; break;
                case '\f': out << "\\f"; break;
                case '\n': out << "\\n"; break;
                case '\r': out << "\\r"; break;
                case '\t': out << "\\t"; break;
                default:
                    if (c < 0x20) {
                        static const char* hex = "0123456789abcdef";
                        out << "\\u00" << hex[(c >> 4) & 0x0F] << hex[c & 0x0F];
                    } else {
                        out << static_cast<char>(c);
                    }
            }
        }
    };

    std::ostringstream oss;
    oss << "{\"error_code\":\"";
    appendJsonString(oss, error_code);
    oss << "\",\"message\":\"";
    appendJsonString(oss, message);
    oss << "\"}";
    return oss.str();
}

}  // namespace oj
