#include "discussion_service.h"
#include "gemini_client.h"

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
    // Windows 下测试同样使用安全环境变量读取方式。
    char* value = nullptr;
    size_t len = 0;
    if (_dupenv_s(&value, &len, name) != 0 || !value) {
        return false;
    }
    const std::string text(value);
    std::free(value);
    // 允许几种常见真值写法，方便命令行和 CI 配置。
    return text == "1" || text == "true" || text == "TRUE" || text == "on" || text == "ON";
#else
    const char* value = std::getenv(name);
    if (!value) {
        return false;
    }
    const std::string text(value);
    // 非 Windows 分支保持同样的开关语义。
    return text == "1" || text == "true" || text == "TRUE" || text == "on" || text == "ON";
#endif
}

}  // namespace

int main() {
    DiscussionService service;

    // 无需数据库也能测试 Gemini 提示词构造，保证 AI 摘要基础逻辑可快速验证。
    DiscussionTopic prompt_topic;
    prompt_topic.problem_id = 1001;
    prompt_topic.username = "haha";
    prompt_topic.title = "Two Sum question";
    prompt_topic.content = "Can someone explain O(n) solution?";
    DiscussionComment prompt_comment;
    prompt_comment.id = 1;
    prompt_comment.topic_id = 1;
    prompt_comment.username = "haha";
    prompt_comment.content = "Use hash map.";
    const std::string prompt = GeminiClient::buildSummaryPrompt(prompt_topic, {prompt_comment});
    // 提示词必须包含主题和评论内容，否则摘要会缺上下文。
    assert(prompt.find("Two Sum question") != std::string::npos);
    assert(prompt.find("Use hash map.") != std::string::npos);

    const std::string gemini_response =
        R"({"candidates":[{"content":{"parts":[{"text":"- Summary focuses on hash map."}]}}]})";
    // 响应解析只提取文本片段，忽略 Gemini 的外层结构。
    assert(GeminiClient::extractTextFromResponse(gemini_response).find("hash map") != std::string::npos);

    if (!envEnabled("OJ_DISCUSSION_RUN_DB_TESTS")) {
        // 默认跳过数据库集成测试，避免普通构建机器没有 MySQL 时失败。
        try {
            (void)service.listTopics(std::nullopt, 20, 0);
        } catch (const std::exception& e) {
            assert(std::string(e.what()).find("MySQL") != std::string::npos ||
                   std::string(e.what()).find("mysql") != std::string::npos);
        }
        std::cout << "Skipping discussion DB integration test; set OJ_DISCUSSION_RUN_DB_TESTS=1 to run it.\n";
        return 0;
    }

    // 开启集成测试后才初始化真实基础设施，避免无数据库环境产生副作用。
    oj::AppConfig cfg = oj::loadConfigFromEnv();
    oj::initInfrastructure(cfg);

    if (!oj::MysqlConnectionPool::instance().available()) {
        // 明确失败原因，避免误以为断言逻辑出错。
        std::cerr << "MySQL is not available; cannot run discussion DB integration test.\n";
        oj::shutdownInfrastructure();
        return 1;
    }

    // 创建主题走 username 入口，覆盖前端实际使用的调用路径。
    const int64_t topic_id = service.createTopicByUsername(
        1001,
        "haha",
        "Two Sum question",
        "Can someone explain O(n) solution?");
    assert(topic_id > 0);

    const auto topics = service.listTopics(std::nullopt, 20, 0);
    assert(!topics.empty());

    // 第二条评论回复第一条，用于验证回复关系和递归删除。
    const int64_t c1 = service.createCommentByUsername(topic_id, "haha", "Use hash map.", std::nullopt);
    const int64_t c2 = service.createCommentByUsername(topic_id, "haha", "Agree with above.", c1);
    assert(c2 > c1);

    const auto comments = service.listComments(topic_id);
    assert(comments.size() >= 2);

    const auto topic = service.getTopic(topic_id);
    assert(topic.has_value());
    // comment_count 是冗余字段，评论创建后必须同步增长。
    assert(topic->comment_count >= 2);
    assert(topic->username == "haha");

    const int deleted_count = service.deleteCommentByUsername(topic_id, c1, "haha");
    // 删除父评论时应同时删除子回复。
    assert(deleted_count >= 2);

    const auto comments_after_delete = service.listComments(topic_id);
    for (const auto& comment : comments_after_delete) {
        assert(comment.id != c1);
        assert(comment.id != c2);
    }

    const auto topic_after_delete = service.getTopic(topic_id);
    assert(topic_after_delete.has_value());
    // 删除回复树后，主题上的冗余计数也要同步扣减。
    assert(topic_after_delete->comment_count <= topic->comment_count - deleted_count);

    // 最后清理测试主题，避免重复跑集成测试时污染列表。
    const int deleted_topic_count = service.deleteTopicByUsername(topic_id, "haha");
    assert(deleted_topic_count == 1);
    assert(!service.getTopic(topic_id).has_value());

    oj::shutdownInfrastructure();
    return 0;
}
