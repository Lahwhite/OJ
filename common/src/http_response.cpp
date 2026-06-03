#include "oj/http_response.h"

#include <sstream>

namespace oj {

HttpResponse HttpResponse::text(int statusCode, const std::string& bodyText,
                                const std::string& contentType) {
    HttpResponse res;
    res.status = statusCode;
    res.body = bodyText;
    res.headers["Content-Type"] = contentType;
    return res;
}

HttpResponse HttpResponse::json(int statusCode, const std::string& bodyText) {
    return text(statusCode, bodyText, "application/json; charset=utf-8");
}

std::string HttpResponse::toHttpString() const {
    std::ostringstream out;
    out << "HTTP/1.1 " << status << " " << statusText(status) << "\r\n";

    bool hasContentLength = false;
    for (const auto& kv : headers) {
        if (kv.first == "Content-Length") {
            hasContentLength = true;
        }
        out << kv.first << ": " << kv.second << "\r\n";
    }

    if (!hasContentLength) {
        out << "Content-Length: " << body.size() << "\r\n";
    }
    out << "Connection: close\r\n";
    out << "\r\n";
    out << body;
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
