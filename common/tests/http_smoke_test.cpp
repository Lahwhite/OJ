// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/http_response.h"
#include "oj/router.h"

#include <iostream>

namespace {

// 布尔返回值通常用于表示是否执行成功
bool expect(bool cond, const std::string& msg) {
    // 条件判断：根据运行时状态决定后续流程
    if (!cond) {
        std::cerr << "[FAIL] " << msg << std::endl;
        // 返回当前阶段的处理结果或默认兜底值
        return false;
    }
    std::cout << "[PASS] " << msg << std::endl;
    // 返回当前阶段的处理结果或默认兜底值
    return true;
}

// 布尔返回值通常用于表示是否执行成功
bool testRouterDispatch() {
    oj::Router router;
    router.get("/health", [](const oj::HttpRequest&) {
        // 返回当前阶段的处理结果或默认兜底值
        return oj::HttpResponse::json(200, "{\"status\":\"ok\"}");
    });

    oj::HttpRequest req;
    req.method = "GET";
    req.path = "/health";

    oj::HttpResponse res = router.dispatch(req);
    // 返回当前阶段的处理结果或默认兜底值
    return expect(res.status == 200, "router dispatches registered GET handler") &&
           expect(res.body == "{\"status\":\"ok\"}", "router returns handler body");
}

// 布尔返回值通常用于表示是否执行成功
bool testRouterNotFound() {
    oj::Router router;

    oj::HttpRequest req;
    req.method = "POST";
    req.path = "/missing";

    oj::HttpResponse res = router.dispatch(req);
    // 返回当前阶段的处理结果或默认兜底值
    return expect(res.status == 404, "router returns 404 for missing route") &&
           expect(res.body.find("NOT_FOUND") != std::string::npos,
                  "router returns standard not found json");
}

// 布尔返回值通常用于表示是否执行成功
bool testHttpWireFormat() {
    oj::HttpResponse res = oj::HttpResponse::json(201, "{\"id\":1}");
    std::string wire = res.toHttpString();

    // 返回当前阶段的处理结果或默认兜底值
    return expect(wire.find("HTTP/1.1 201 Created") != std::string::npos,
                  "http response has status line") &&
           expect(wire.find("Content-Type: application/json; charset=utf-8") != std::string::npos,
                  "http response has json content type") &&
           expect(wire.find("Content-Length: 8") != std::string::npos,
                  "http response has content length") &&
           expect(wire.find("{\"id\":1}") != std::string::npos,
                  "http response includes body");
}

}  // namespace

int main() {
    // 布尔返回值通常用于表示是否执行成功
    bool ok = true;
    ok = testRouterDispatch() && ok;
    ok = testRouterNotFound() && ok;
    ok = testHttpWireFormat() && ok;

    // 条件判断：根据运行时状态决定后续流程
    if (!ok) {
        std::cerr << "Common HTTP smoke tests failed." << std::endl;
        // 返回当前阶段的处理结果或默认兜底值
        return 1;
    }
    std::cout << "Common HTTP smoke tests passed." << std::endl;
    // 返回当前阶段的处理结果或默认兜底值
    return 0;
}
