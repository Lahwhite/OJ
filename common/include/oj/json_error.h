// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include <stdexcept>
#include <string>

namespace oj {

// 结构定义：用于组织一组紧密相关的数据
struct HttpException : public std::runtime_error {
    int http_status;
    std::string error_code;

    // 业务里直接抛这个，外层再统一转成 JSON 错误响应
    HttpException(int status, std::string code, std::string message)
        : std::runtime_error(std::move(message)),
          http_status(status),
          error_code(std::move(code)) {}
};

std::string makeErrorJson(const std::string& error_code, const std::string& message);

}  // namespace oj
