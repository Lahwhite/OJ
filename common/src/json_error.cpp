// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/json_error.h"

#include <sstream>

namespace oj {

std::string makeErrorJson(const std::string& error_code, const std::string& message) {
    // 这里只手动转义一小部分常见字符，够错误信息返回用了
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
                    // 条件判断：根据运行时状态决定后续流程
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
    // 返回当前阶段的处理结果或默认兜底值
    return oss.str();
}

}  // namespace oj
