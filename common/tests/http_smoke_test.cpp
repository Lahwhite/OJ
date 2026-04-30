#include "oj/http_response.h"
#include "oj/router.h"

#include <iostream>

namespace {

bool expect(bool cond, const std::string& msg) {
    if (!cond) {
        std::cerr << "[FAIL] " << msg << std::endl;
        return false;
    }
    std::cout << "[PASS] " << msg << std::endl;
    return true;
}

bool testRouterDispatch() {
    oj::Router router;
    router.get("/health", [](const oj::HttpRequest&) {
        return oj::HttpResponse::json(200, "{\"status\":\"ok\"}");
    });

    oj::HttpRequest req;
    req.method = "GET";
    req.path = "/health";

    oj::HttpResponse res = router.dispatch(req);
    return expect(res.status == 200, "router dispatches registered GET handler") &&
           expect(res.body == "{\"status\":\"ok\"}", "router returns handler body");
}

bool testRouterNotFound() {
    oj::Router router;

    oj::HttpRequest req;
    req.method = "POST";
    req.path = "/missing";

    oj::HttpResponse res = router.dispatch(req);
    return expect(res.status == 404, "router returns 404 for missing route") &&
           expect(res.body.find("NOT_FOUND") != std::string::npos,
                  "router returns standard not found json");
}

bool testHttpWireFormat() {
    oj::HttpResponse res = oj::HttpResponse::json(201, "{\"id\":1}");
    std::string wire = res.toHttpString();

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
    bool ok = true;
    ok = testRouterDispatch() && ok;
    ok = testRouterNotFound() && ok;
    ok = testHttpWireFormat() && ok;

    if (!ok) {
        std::cerr << "Common HTTP smoke tests failed." << std::endl;
        return 1;
    }
    std::cout << "Common HTTP smoke tests passed." << std::endl;
    return 0;
}
