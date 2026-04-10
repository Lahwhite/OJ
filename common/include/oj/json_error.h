#pragma once

#include <stdexcept>
#include <string>

namespace oj {

struct HttpException : public std::runtime_error {
    int http_status;
    std::string error_code;

    HttpException(int status, std::string code, std::string message)
        : std::runtime_error(std::move(message)),
          http_status(status),
          error_code(std::move(code)) {}
};

std::string makeErrorJson(const std::string& error_code, const std::string& message);

}  // namespace oj
