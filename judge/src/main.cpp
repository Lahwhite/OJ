#include "submission_handler.h"
#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/log.h"

#include <cstdlib>
#include <iostream>

namespace {
void shutdownAtExit() {
    oj::shutdownInfrastructure();
}
}  // namespace

int main() {
    try {
        oj::AppConfig cfg = oj::loadConfigFromEnv();
        oj::initInfrastructure(cfg);
        std::atexit(shutdownAtExit);

        SubmissionHandler handler;
        std::cout << "Judge server starting on port 8080..." << std::endl;
        OJ_LOG_INFO("judge_engine process started");
        handler.startServer(8080);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
