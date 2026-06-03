#include "discussion_service.h"

#include "oj/mysql_pool.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#ifdef OJ_WITH_MYSQL
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#endif

namespace {

constexpr int kMaxTitleLength = 200;
constexpr int kMaxTopicContentLength = 5000;
constexpr int kMaxCommentContentLength = 3000;

void validateTopicInput(int64_t problem_id,
                        int64_t user_id,
                        const std::string& title,
                        const std::string& content) {
    if (problem_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("problem_id and user_id must be positive");
    }
    if (title.empty() || content.empty()) {
        throw std::invalid_argument("title/content cannot be empty");
    }
    if (title.size() > kMaxTitleLength) {
        throw std::invalid_argument("title too long, max length is 200");
    }
    if (content.size() > kMaxTopicContentLength) {
        throw std::invalid_argument("content too long, max length is 5000");
    }
}

void validateCommentInput(int64_t topic_id, int64_t user_id, const std::string& content) {
    if (topic_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("topic_id and user_id must be positive");
    }
    if (content.empty()) {
        throw std::invalid_argument("content cannot be empty");
    }
    if (content.size() > kMaxCommentContentLength) {
        throw std::invalid_argument("content too long, max length is 3000");
    }
}

#ifdef OJ_WITH_MYSQL

class MysqlConnectionGuard {
public:
    MysqlConnectionGuard() : conn_(static_cast<sql::Connection*>(oj::MysqlConnectionPool::instance().acquire())) {
        if (!conn_) {
            throw std::runtime_error("mysql connection is not available");
        }
    }

    ~MysqlConnectionGuard() {
        oj::MysqlConnectionPool::instance().release(conn_);
    }

    sql::Connection* get() const {
        return conn_;
    }

private:
    sql::Connection* conn_;
};

std::string nullableString(sql::ResultSet& rs, const std::string& column) {
    const std::string value = rs.getString(column);
    return rs.wasNull() ? std::string() : value;
}

DiscussionTopic topicFromResult(sql::ResultSet& rs) {
    DiscussionTopic topic;
    topic.id = rs.getInt64("id");
    topic.problem_id = rs.getInt64("problem_id");
    topic.user_id = rs.getInt64("user_id");
    topic.username = nullableString(rs, "username");
    topic.nickname = nullableString(rs, "nickname");
    topic.avatar = nullableString(rs, "avatar");
    topic.title = rs.getString("title");
    topic.content = rs.getString("content");
    topic.created_at = rs.getString("created_at");
    topic.updated_at = rs.getString("updated_at");
    topic.comment_count = rs.getInt("comment_count");
    return topic;
}

DiscussionComment commentFromResult(sql::ResultSet& rs) {
    DiscussionComment comment;
    comment.id = rs.getInt64("id");
    comment.topic_id = rs.getInt64("topic_id");
    comment.user_id = rs.getInt64("user_id");
    comment.username = nullableString(rs, "username");
    comment.nickname = nullableString(rs, "nickname");
    comment.avatar = nullableString(rs, "avatar");
    comment.parent_comment_id = rs.getInt64("parent_comment_id");
    if (rs.wasNull()) {
        comment.parent_comment_id = 0;
    }
    comment.content = rs.getString("content");
    comment.created_at = rs.getString("created_at");
    return comment;
}

int64_t lastInsertId(sql::Connection& conn) {
    std::unique_ptr<sql::Statement> stmt(conn.createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
    if (!rs->next()) {
        throw std::runtime_error("failed to read inserted id");
    }
    return rs->getInt64("id");
}

int64_t findUserIdByUsername(sql::Connection& conn, const std::string& username) {
    if (username.empty()) {
        throw std::invalid_argument("username cannot be empty");
    }

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT id FROM `Users` WHERE username = ?"));
    stmt->setString(1, username);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        throw std::out_of_range("user not found");
    }
    return rs->getInt64("id");
}

void ensureUserExists(sql::Connection& conn, int64_t user_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT id FROM `Users` WHERE id = ?"));
    stmt->setInt64(1, user_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        throw std::out_of_range("user not found");
    }
}

bool topicExists(sql::Connection& conn, int64_t topic_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT id FROM discussion_topics WHERE id = ?"));
    stmt->setInt64(1, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    return rs->next();
}

std::optional<int64_t> topicOwnerUserId(sql::Connection& conn, int64_t topic_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT user_id FROM discussion_topics WHERE id = ?"));
    stmt->setInt64(1, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        return std::nullopt;
    }
    return rs->getInt64("user_id");
}

bool parentCommentBelongsToTopic(sql::Connection& conn, int64_t topic_id, int64_t parent_comment_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT id FROM discussion_comments WHERE id = ? AND topic_id = ?"));
    stmt->setInt64(1, parent_comment_id);
    stmt->setInt64(2, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    return rs->next();
}

std::optional<int64_t> commentOwnerUserId(sql::Connection& conn, int64_t topic_id, int64_t comment_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT user_id FROM discussion_comments WHERE id = ? AND topic_id = ?"));
    stmt->setInt64(1, comment_id);
    stmt->setInt64(2, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        return std::nullopt;
    }
    return rs->getInt64("user_id");
}

int countCommentTree(sql::Connection& conn, int64_t topic_id, int64_t comment_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement(
            "WITH RECURSIVE comment_tree AS ("
            "SELECT id FROM discussion_comments WHERE id = ? AND topic_id = ? "
            "UNION ALL "
            "SELECT c.id FROM discussion_comments c "
            "JOIN comment_tree ct ON c.parent_comment_id = ct.id "
            "WHERE c.topic_id = ?"
            ") SELECT COUNT(*) AS deleted_count FROM comment_tree"));
    stmt->setInt64(1, comment_id);
    stmt->setInt64(2, topic_id);
    stmt->setInt64(3, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        return 0;
    }
    return rs->getInt("deleted_count");
}

int countTopicComments(sql::Connection& conn, int64_t topic_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT COUNT(*) AS comment_count FROM discussion_comments WHERE topic_id = ?"));
    stmt->setInt64(1, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        return 0;
    }
    return rs->getInt("comment_count");
}

void deleteTopicComments(sql::Connection& conn, int64_t topic_id) {
    while (countTopicComments(conn, topic_id) > 0) {
        std::unique_ptr<sql::PreparedStatement> stmt(
            conn.prepareStatement(
                "DELETE FROM discussion_comments "
                "WHERE topic_id = ? "
                "AND id NOT IN ("
                "SELECT parent_id FROM ("
                "SELECT DISTINCT parent_comment_id AS parent_id "
                "FROM discussion_comments "
                "WHERE topic_id = ? AND parent_comment_id IS NOT NULL"
                ") AS referenced_parents"
                ")"));
        stmt->setInt64(1, topic_id);
        stmt->setInt64(2, topic_id);
        const int deleted_count = stmt->executeUpdate();
        if (deleted_count <= 0) {
            throw std::runtime_error("failed to delete topic comments");
        }
    }
}

int deleteCommentWithUserId(sql::Connection& conn, int64_t topic_id, int64_t comment_id, int64_t user_id) {
    if (!topicExists(conn, topic_id)) {
        throw std::out_of_range("topic not found");
    }

    const bool old_autocommit = conn.getAutoCommit();
    conn.setAutoCommit(false);
    try {
        const auto owner_user_id = commentOwnerUserId(conn, topic_id, comment_id);
        if (!owner_user_id.has_value()) {
            throw std::out_of_range("comment not found");
        }
        if (*owner_user_id != user_id) {
            throw std::domain_error("permission denied");
        }

        const int deleted_count = countCommentTree(conn, topic_id, comment_id);
        if (deleted_count <= 0) {
            throw std::out_of_range("comment not found");
        }

        std::unique_ptr<sql::PreparedStatement> delete_stmt(
            conn.prepareStatement("DELETE FROM discussion_comments WHERE id = ? AND topic_id = ?"));
        delete_stmt->setInt64(1, comment_id);
        delete_stmt->setInt64(2, topic_id);
        delete_stmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> update_stmt(
            conn.prepareStatement(
                "UPDATE discussion_topics "
                "SET comment_count = GREATEST(comment_count - ?, 0), updated_at = CURRENT_TIMESTAMP "
                "WHERE id = ?"));
        update_stmt->setInt(1, deleted_count);
        update_stmt->setInt64(2, topic_id);
        update_stmt->executeUpdate();

        conn.commit();
        conn.setAutoCommit(old_autocommit);
        return deleted_count;
    } catch (...) {
        conn.rollback();
        conn.setAutoCommit(old_autocommit);
        throw;
    }
}

int deleteTopicWithUserId(sql::Connection& conn, int64_t topic_id, int64_t user_id) {
    const bool old_autocommit = conn.getAutoCommit();
    conn.setAutoCommit(false);
    try {
        const auto owner_user_id = topicOwnerUserId(conn, topic_id);
        if (!owner_user_id.has_value()) {
            throw std::out_of_range("topic not found");
        }
        if (*owner_user_id != user_id) {
            throw std::domain_error("permission denied");
        }

        deleteTopicComments(conn, topic_id);

        std::unique_ptr<sql::PreparedStatement> delete_stmt(
            conn.prepareStatement("DELETE FROM discussion_topics WHERE id = ?"));
        delete_stmt->setInt64(1, topic_id);
        const int deleted_count = delete_stmt->executeUpdate();
        if (deleted_count <= 0) {
            throw std::out_of_range("topic not found");
        }

        conn.commit();
        conn.setAutoCommit(old_autocommit);
        return deleted_count;
    } catch (...) {
        conn.rollback();
        conn.setAutoCommit(old_autocommit);
        throw;
    }
}

#endif

}  // namespace

bool DiscussionService::databaseReady() const {
    return oj::MysqlConnectionPool::instance().available();
}

void DiscussionService::ensureSchema() const {
#ifndef OJ_WITH_MYSQL
    throw std::runtime_error("discussion service requires MySQL; rebuild with OJ_ENABLE_MYSQL=ON");
#else
    {
        std::lock_guard<std::mutex> lock(schema_mutex_);
        if (schema_initialized_) {
            return;
        }
    }

    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();

    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    stmt->execute(
        "CREATE TABLE IF NOT EXISTS discussion_topics ("
        "id BIGINT PRIMARY KEY AUTO_INCREMENT,"
        "problem_id BIGINT NOT NULL,"
        "user_id BIGINT NOT NULL,"
        "title VARCHAR(200) NOT NULL,"
        "content TEXT NOT NULL,"
        "comment_count INT NOT NULL DEFAULT 0,"
        "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
        "INDEX idx_discussion_topics_problem_updated(problem_id, updated_at),"
        "INDEX idx_discussion_topics_user(user_id),"
        "CONSTRAINT fk_discussion_topics_user FOREIGN KEY (user_id) REFERENCES `Users`(id)"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci");

    stmt->execute(
        "CREATE TABLE IF NOT EXISTS discussion_comments ("
        "id BIGINT PRIMARY KEY AUTO_INCREMENT,"
        "topic_id BIGINT NOT NULL,"
        "user_id BIGINT NOT NULL,"
        "parent_comment_id BIGINT NULL,"
        "content TEXT NOT NULL,"
        "created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,"
        "INDEX idx_discussion_comments_topic_id(topic_id, id),"
        "INDEX idx_discussion_comments_user(user_id),"
        "CONSTRAINT fk_discussion_comments_topic FOREIGN KEY (topic_id) REFERENCES discussion_topics(id) ON DELETE CASCADE,"
        "CONSTRAINT fk_discussion_comments_user FOREIGN KEY (user_id) REFERENCES `Users`(id),"
        "CONSTRAINT fk_discussion_comments_parent FOREIGN KEY (parent_comment_id) REFERENCES discussion_comments(id) ON DELETE CASCADE"
        ") ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci");

    std::lock_guard<std::mutex> lock(schema_mutex_);
    schema_initialized_ = true;
#endif
}

int64_t DiscussionService::createTopic(int64_t problem_id,
                                       int64_t user_id,
                                       const std::string& title,
                                       const std::string& content) {
    validateTopicInput(problem_id, user_id, title, content);
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    ensureUserExists(*conn, user_id);

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "INSERT INTO discussion_topics (problem_id, user_id, title, content) "
            "VALUES (?, ?, ?, ?)"));
    stmt->setInt64(1, problem_id);
    stmt->setInt64(2, user_id);
    stmt->setString(3, title);
    stmt->setString(4, content);
    stmt->executeUpdate();
    return lastInsertId(*conn);
#endif
}

int64_t DiscussionService::createTopicByUsername(int64_t problem_id,
                                                 const std::string& username,
                                                 const std::string& title,
                                                 const std::string& content) {
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    (void)problem_id;
    (void)username;
    (void)title;
    (void)content;
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    const int64_t user_id = findUserIdByUsername(*conn, username);

    validateTopicInput(problem_id, user_id, title, content);
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "INSERT INTO discussion_topics (problem_id, user_id, title, content) "
            "VALUES (?, ?, ?, ?)"));
    stmt->setInt64(1, problem_id);
    stmt->setInt64(2, user_id);
    stmt->setString(3, title);
    stmt->setString(4, content);
    stmt->executeUpdate();
    return lastInsertId(*conn);
#endif
}

std::vector<DiscussionTopic> DiscussionService::listTopics(std::optional<int64_t> problem_id,
                                                           size_t limit,
                                                           size_t offset) const {
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    (void)problem_id;
    (void)limit;
    (void)offset;
    return {};
#else
    if (limit == 0) {
        return {};
    }

    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    std::vector<DiscussionTopic> topics;

    std::unique_ptr<sql::PreparedStatement> stmt;
    if (problem_id.has_value()) {
        stmt.reset(conn->prepareStatement(
            "SELECT t.id, t.problem_id, t.user_id, u.username, p.nickname, p.avatar, "
            "t.title, t.content, t.comment_count, "
            "DATE_FORMAT(t.created_at, '%Y-%m-%d %H:%i:%s') AS created_at, "
            "DATE_FORMAT(t.updated_at, '%Y-%m-%d %H:%i:%s') AS updated_at "
            "FROM discussion_topics t "
            "JOIN `Users` u ON u.id = t.user_id "
            "LEFT JOIN `Profiles` p ON p.username = u.username "
            "WHERE t.problem_id = ? "
            "ORDER BY t.id DESC LIMIT ? OFFSET ?"));
        stmt->setInt64(1, *problem_id);
        stmt->setInt64(2, static_cast<int64_t>(limit));
        stmt->setInt64(3, static_cast<int64_t>(offset));
    } else {
        stmt.reset(conn->prepareStatement(
            "SELECT t.id, t.problem_id, t.user_id, u.username, p.nickname, p.avatar, "
            "t.title, t.content, t.comment_count, "
            "DATE_FORMAT(t.created_at, '%Y-%m-%d %H:%i:%s') AS created_at, "
            "DATE_FORMAT(t.updated_at, '%Y-%m-%d %H:%i:%s') AS updated_at "
            "FROM discussion_topics t "
            "JOIN `Users` u ON u.id = t.user_id "
            "LEFT JOIN `Profiles` p ON p.username = u.username "
            "ORDER BY t.id DESC LIMIT ? OFFSET ?"));
        stmt->setInt64(1, static_cast<int64_t>(limit));
        stmt->setInt64(2, static_cast<int64_t>(offset));
    }

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        topics.push_back(topicFromResult(*rs));
    }
    return topics;
#endif
}

std::optional<DiscussionTopic> DiscussionService::getTopic(int64_t topic_id) const {
    ensureSchema();
    if (topic_id <= 0) {
        return std::nullopt;
    }

#ifndef OJ_WITH_MYSQL
    return std::nullopt;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "SELECT t.id, t.problem_id, t.user_id, u.username, p.nickname, p.avatar, "
            "t.title, t.content, t.comment_count, "
            "DATE_FORMAT(t.created_at, '%Y-%m-%d %H:%i:%s') AS created_at, "
            "DATE_FORMAT(t.updated_at, '%Y-%m-%d %H:%i:%s') AS updated_at "
            "FROM discussion_topics t "
            "JOIN `Users` u ON u.id = t.user_id "
            "LEFT JOIN `Profiles` p ON p.username = u.username "
            "WHERE t.id = ?"));
    stmt->setInt64(1, topic_id);

    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        return std::nullopt;
    }
    return topicFromResult(*rs);
#endif
}

int DiscussionService::deleteTopic(int64_t topic_id, int64_t user_id) {
    if (topic_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("topic_id and user_id must be positive");
    }
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    ensureUserExists(*conn, user_id);
    return deleteTopicWithUserId(*conn, topic_id, user_id);
#endif
}

int DiscussionService::deleteTopicByUsername(int64_t topic_id, const std::string& username) {
    if (topic_id <= 0) {
        throw std::invalid_argument("topic_id must be positive");
    }
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    (void)topic_id;
    (void)username;
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    const int64_t user_id = findUserIdByUsername(*conn, username);
    return deleteTopicWithUserId(*conn, topic_id, user_id);
#endif
}

int64_t DiscussionService::createComment(int64_t topic_id,
                                         int64_t user_id,
                                         const std::string& content,
                                         std::optional<int64_t> parent_comment_id) {
    validateCommentInput(topic_id, user_id, content);
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    ensureUserExists(*conn, user_id);
    if (!topicExists(*conn, topic_id)) {
        throw std::out_of_range("topic not found");
    }
    if (parent_comment_id.has_value() &&
        !parentCommentBelongsToTopic(*conn, topic_id, *parent_comment_id)) {
        throw std::invalid_argument("parent_comment_id is invalid");
    }

    const bool old_autocommit = conn->getAutoCommit();
    conn->setAutoCommit(false);
    try {
        if (parent_comment_id.has_value()) {
            std::unique_ptr<sql::PreparedStatement> insert_stmt(
                conn->prepareStatement(
                    "INSERT INTO discussion_comments (topic_id, user_id, parent_comment_id, content) "
                    "VALUES (?, ?, ?, ?)"));
            insert_stmt->setInt64(1, topic_id);
            insert_stmt->setInt64(2, user_id);
            insert_stmt->setInt64(3, *parent_comment_id);
            insert_stmt->setString(4, content);
            insert_stmt->executeUpdate();
        } else {
            std::unique_ptr<sql::PreparedStatement> insert_stmt(
                conn->prepareStatement(
                    "INSERT INTO discussion_comments (topic_id, user_id, content) "
                    "VALUES (?, ?, ?)"));
            insert_stmt->setInt64(1, topic_id);
            insert_stmt->setInt64(2, user_id);
            insert_stmt->setString(3, content);
            insert_stmt->executeUpdate();
        }
        const int64_t comment_id = lastInsertId(*conn);

        std::unique_ptr<sql::PreparedStatement> update_stmt(
            conn->prepareStatement(
                "UPDATE discussion_topics "
                "SET comment_count = comment_count + 1, updated_at = CURRENT_TIMESTAMP "
                "WHERE id = ?"));
        update_stmt->setInt64(1, topic_id);
        update_stmt->executeUpdate();

        conn->commit();
        conn->setAutoCommit(old_autocommit);
        return comment_id;
    } catch (...) {
        conn->rollback();
        conn->setAutoCommit(old_autocommit);
        throw;
    }
#endif
}

int64_t DiscussionService::createCommentByUsername(int64_t topic_id,
                                                   const std::string& username,
                                                   const std::string& content,
                                                   std::optional<int64_t> parent_comment_id) {
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    (void)topic_id;
    (void)username;
    (void)content;
    (void)parent_comment_id;
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    const int64_t user_id = findUserIdByUsername(*conn, username);
    validateCommentInput(topic_id, user_id, content);

    if (!topicExists(*conn, topic_id)) {
        throw std::out_of_range("topic not found");
    }
    if (parent_comment_id.has_value() &&
        !parentCommentBelongsToTopic(*conn, topic_id, *parent_comment_id)) {
        throw std::invalid_argument("parent_comment_id is invalid");
    }

    const bool old_autocommit = conn->getAutoCommit();
    conn->setAutoCommit(false);
    try {
        if (parent_comment_id.has_value()) {
            std::unique_ptr<sql::PreparedStatement> insert_stmt(
                conn->prepareStatement(
                    "INSERT INTO discussion_comments (topic_id, user_id, parent_comment_id, content) "
                    "VALUES (?, ?, ?, ?)"));
            insert_stmt->setInt64(1, topic_id);
            insert_stmt->setInt64(2, user_id);
            insert_stmt->setInt64(3, *parent_comment_id);
            insert_stmt->setString(4, content);
            insert_stmt->executeUpdate();
        } else {
            std::unique_ptr<sql::PreparedStatement> insert_stmt(
                conn->prepareStatement(
                    "INSERT INTO discussion_comments (topic_id, user_id, content) "
                    "VALUES (?, ?, ?)"));
            insert_stmt->setInt64(1, topic_id);
            insert_stmt->setInt64(2, user_id);
            insert_stmt->setString(3, content);
            insert_stmt->executeUpdate();
        }
        const int64_t comment_id = lastInsertId(*conn);

        std::unique_ptr<sql::PreparedStatement> update_stmt(
            conn->prepareStatement(
                "UPDATE discussion_topics "
                "SET comment_count = comment_count + 1, updated_at = CURRENT_TIMESTAMP "
                "WHERE id = ?"));
        update_stmt->setInt64(1, topic_id);
        update_stmt->executeUpdate();

        conn->commit();
        conn->setAutoCommit(old_autocommit);
        return comment_id;
    } catch (...) {
        conn->rollback();
        conn->setAutoCommit(old_autocommit);
        throw;
    }
#endif
}

int DiscussionService::deleteComment(int64_t topic_id, int64_t comment_id, int64_t user_id) {
    if (topic_id <= 0 || comment_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("topic_id, comment_id and user_id must be positive");
    }
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    ensureUserExists(*conn, user_id);
    return deleteCommentWithUserId(*conn, topic_id, comment_id, user_id);
#endif
}

int DiscussionService::deleteCommentByUsername(int64_t topic_id,
                                               int64_t comment_id,
                                               const std::string& username) {
    if (topic_id <= 0 || comment_id <= 0) {
        throw std::invalid_argument("topic_id and comment_id must be positive");
    }
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    (void)topic_id;
    (void)comment_id;
    (void)username;
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    const int64_t user_id = findUserIdByUsername(*conn, username);
    return deleteCommentWithUserId(*conn, topic_id, comment_id, user_id);
#endif
}

std::vector<DiscussionComment> DiscussionService::listComments(int64_t topic_id) const {
    ensureSchema();
    if (topic_id <= 0) {
        throw std::out_of_range("topic not found");
    }

#ifndef OJ_WITH_MYSQL
    return {};
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    if (!topicExists(*conn, topic_id)) {
        throw std::out_of_range("topic not found");
    }

    std::unique_ptr<sql::PreparedStatement> stmt(
        conn->prepareStatement(
            "SELECT c.id, c.topic_id, c.user_id, u.username, p.nickname, p.avatar, "
            "c.parent_comment_id, c.content, "
            "DATE_FORMAT(c.created_at, '%Y-%m-%d %H:%i:%s') AS created_at "
            "FROM discussion_comments c "
            "JOIN `Users` u ON u.id = c.user_id "
            "LEFT JOIN `Profiles` p ON p.username = u.username "
            "WHERE c.topic_id = ? "
            "ORDER BY c.id ASC"));
    stmt->setInt64(1, topic_id);

    std::vector<DiscussionComment> comments;
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    while (rs->next()) {
        comments.push_back(commentFromResult(*rs));
    }
    return comments;
#endif
}
