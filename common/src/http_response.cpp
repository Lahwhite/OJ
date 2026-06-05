// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/http_response.h"

#include <sstream>

namespace oj {

HttpResponse HttpResponse::text(int statusCode, const std::string& bodyText,
                                const std::string& contentType) {
    HttpResponse res;
    res.status = statusCode;
    res.body = bodyText;
    res.headers["Content-Type"] = contentType;
    // 返回当前阶段的处理结果或默认兜底值
    return res;
}

HttpResponse HttpResponse::json(int statusCode, const std::string& bodyText) {
    // 返回当前阶段的处理结果或默认兜底值
    return text(statusCode, bodyText, "application/json; charset=utf-8");
}

std::string HttpResponse::toHttpString() const {
    // 这里直接手动拼 HTTP 报文，简单也够用
    std::ostringstream out;
    out << "HTTP/1.1 " << status << " " << statusText(status) << "\r\n";

    // 布尔返回值通常用于表示是否执行成功
    bool hasContentLength = false;
    // 循环处理：逐项遍历并构造结果或执行操作
    for (const auto& kv : headers) {
        // 条件判断：根据运行时状态决定后续流程
        if (kv.first == "Content-Length") {
            hasContentLength = true;
        }
        out << kv.first << ": " << kv.second << "\r\n";
    }

    // 条件判断：根据运行时状态决定后续流程
    if (!hasContentLength) {
        out << "Content-Length: " << body.size() << "\r\n";
    }
    out << "Connection: close\r\n";
    out << "\r\n";
    out << body;
    // 返回当前阶段的处理结果或默认兜底值
    return out.str();
}

const char* HttpResponse::statusText(int code) {
    switch (code) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 400: return "Bad Request";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 500: return "Internal Server Error";
        default: return "OK";
    }
}

}  // namespace oj
