// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/http_request.h"

namespace oj {

std::string HttpRequest::header(const std::string& key) const {
    // 读不到就返回空串，调用方少写一层判空
    auto it = headers.find(key);
    if (it != headers.end()) {
        // 返回当前阶段的处理结果或默认兜底值
        return it->second;
    }
    // 返回当前阶段的处理结果或默认兜底值
    return "";
}

}  // namespace oj
