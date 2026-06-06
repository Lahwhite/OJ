// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include <string>
#include <unordered_map>

namespace oj {

// 结构定义：用于组织一组紧密相关的数据
struct HttpRequest {
    std::string method;
    std::string path;
    std::string query;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    std::string header(const std::string& key) const;
};

}  // namespace oj
