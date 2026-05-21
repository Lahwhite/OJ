#include "discussion_service.h"

#include "oj/bootstrap.h"
#include "oj/config.h"
#include "oj/mysql_pool.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

bool envEnabled(const char* name) {
#if defined(_WIN32)
    char* value = nullptr;
    size_t len = 0;
    if (_dupenv_s(&value, &len, name) != 0 || !value) {
        return false;
    }
    const std::string text(value);
    std::free(value);
    return text == "1" || text == "true" || text == "TRUE" || text == "on" || text == "ON";
#else
    const char* value = std::getenv(name);
    if (!value) {
        return false;
    }
    const std::string text(value);
    return text == "1" || text == "true" || text == "TRUE" || text == "on" || text == "ON";
#endif
}

}  // namespace

int main() {
    DiscussionService service;

    if (!envEnabled("OJ_DISCUSSION_RUN_DB_TESTS")) {
        try {
            (void)service.listTopics(std::nullopt, 20, 0);
        } catch (const std::exception& e) {
            assert(std::string(e.what()).find("MySQL") != std::string::npos ||
                   std::string(e.what()).find("mysql") != std::string::npos);
        }
        std::cout << "Skipping discussion DB integration test; set OJ_DISCUSSION_RUN_DB_TESTS=1 to run it.\n";
        return 0;
    }

    oj::AppConfig cfg = oj::loadConfigFromEnv();
    oj::initInfrastructure(cfg);

    if (!oj::MysqlConnectionPool::instance().available()) {
        std::cerr << "MySQL is not available; cannot run discussion DB integration test.\n";
        oj::shutdownInfrastructure();
        return 1;
    }

    const int64_t topic_id = service.createTopicByUsername(
        1001,
        "haha",
        "Two Sum question",
        "Can someone explain O(n) solution?");
    assert(topic_id > 0);

    const auto topics = service.listTopics(std::nullopt, 20, 0);
    assert(!topics.empty());

    const int64_t c1 = service.createCommentByUsername(topic_id, "haha", "Use hash map.", std::nullopt);
    const int64_t c2 = service.createCommentByUsername(topic_id, "haha", "Agree with above.", c1);
    assert(c2 > c1);

    const auto comments = service.listComments(topic_id);
    assert(comments.size() >= 2);

    const auto topic = service.getTopic(topic_id);
    assert(topic.has_value());
    assert(topic->comment_count >= 2);
    assert(topic->username == "haha");

    oj::shutdownInfrastructure();
    return 0;
}
