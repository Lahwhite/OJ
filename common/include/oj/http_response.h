// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include <string>
#include <unordered_map>

namespace oj {

// 类定义：把相关状态与行为封装在一起
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
