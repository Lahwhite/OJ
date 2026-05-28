/**
 * @file discussion_service.h
 * @brief Discussion module business service.
 */
#ifndef DISCUSSION_SERVICE_H
#define DISCUSSION_SERVICE_H

#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

struct DiscussionTopic {
    int64_t id = 0;
    int64_t problem_id = 0;
    int64_t user_id = 0;
    std::string username;
    std::string nickname;
    std::string avatar;
    std::string title;
    std::string content;
    std::string created_at;
    std::string updated_at;
    int comment_count = 0;
};

struct DiscussionComment {
    int64_t id = 0;
    int64_t topic_id = 0;
    int64_t user_id = 0;
    std::string username;
    std::string nickname;
    std::string avatar;
    int64_t parent_comment_id = 0;
    std::string content;
    std::string created_at;
};

class DiscussionService {
public:
    bool databaseReady() const;

    int64_t createTopic(int64_t problem_id,
                        int64_t user_id,
                        const std::string& title,
                        const std::string& content);

    int64_t createTopicByUsername(int64_t problem_id,
                                  const std::string& username,
                                  const std::string& title,
                                  const std::string& content);

    std::vector<DiscussionTopic> listTopics(std::optional<int64_t> problem_id,
                                            size_t limit,
                                            size_t offset) const;

    std::optional<DiscussionTopic> getTopic(int64_t topic_id) const;

    int64_t createComment(int64_t topic_id,
                          int64_t user_id,
                          const std::string& content,
                          std::optional<int64_t> parent_comment_id);

    int64_t createCommentByUsername(int64_t topic_id,
                                    const std::string& username,
                                    const std::string& content,
                                    std::optional<int64_t> parent_comment_id);

    std::vector<DiscussionComment> listComments(int64_t topic_id) const;

private:
    void ensureSchema() const;

    mutable std::mutex schema_mutex_;
    mutable bool schema_initialized_ = false;
};

#endif  // DISCUSSION_SERVICE_H
