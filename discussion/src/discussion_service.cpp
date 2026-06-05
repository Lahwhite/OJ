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

// 讨论区的长度限制集中放在服务层，保证绕过前端时仍然能被拦住。
// 这些值要和 web/index.html 中的 maxlength 保持一致，避免用户端和服务端提示不一致。
constexpr int kMaxTitleLength = 200;
constexpr int kMaxTopicContentLength = 5000;
constexpr int kMaxCommentContentLength = 3000;

void validateTopicInput(int64_t problem_id,
                        int64_t user_id,
                        const std::string& title,
                        const std::string& content) {
    // 题目、用户都来自其他模块，先做正数校验可以避免无意义的数据库查询。
    if (problem_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("problem_id and user_id must be positive");
    }
    // 标题和正文必须同时存在，空字符串不进入持久化层。
    if (title.empty() || content.empty()) {
        throw std::invalid_argument("title/content cannot be empty");
    }
    // 这里按字节数限制，与 MySQL VARCHAR/TEXT 实际存储上限相比更保守。
    if (title.size() > kMaxTitleLength) {
        throw std::invalid_argument("title too long, max length is 200");
    }
    // 内容过长会影响列表接口响应体大小，所以在入库前直接拒绝。
    if (content.size() > kMaxTopicContentLength) {
        throw std::invalid_argument("content too long, max length is 5000");
    }
}

void validateCommentInput(int64_t topic_id, int64_t user_id, const std::string& content) {
    // 评论必须绑定到已有主题和用户，负数或零都属于调用方参数错误。
    if (topic_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("topic_id and user_id must be positive");
    }
    // 不允许空评论，避免页面出现没有正文的回复卡片。
    if (content.empty()) {
        throw std::invalid_argument("content cannot be empty");
    }
    // 评论区通常比主题正文更短，单独设置上限便于前端提示和后端保护。
    if (content.size() > kMaxCommentContentLength) {
        throw std::invalid_argument("content too long, max length is 3000");
    }
}

#ifdef OJ_WITH_MYSQL

// MySQL 连接池只暴露 acquire/release，这里用 RAII 包一层，确保异常路径也归还连接。
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

// Profiles 表中的昵称和头像允许为空，统一转换成空字符串简化 JSON 序列化。
std::string nullableString(sql::ResultSet& rs, const std::string& column) {
    const std::string value = rs.getString(column);
    return rs.wasNull() ? std::string() : value;
}

// 将主题查询结果映射成领域对象，字段名需要和 SELECT 中的别名保持一致。
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

// 评论查询同样在这里做空值归一化，parent_comment_id 为 0 表示顶层评论。
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

// Connector/C++ 这里没有直接返回自增 ID，所以用同一连接读取 LAST_INSERT_ID()。
int64_t lastInsertId(sql::Connection& conn) {
    std::unique_ptr<sql::Statement> stmt(conn.createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery("SELECT LAST_INSERT_ID() AS id"));
    if (!rs->next()) {
        throw std::runtime_error("failed to read inserted id");
    }
    return rs->getInt64("id");
}

// 前端主要传 username，服务层在这里转换成 Users.id，避免各接口重复写查询逻辑。
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

// 使用 user_id 的接口也要确认用户存在，否则外键异常会变成不友好的 500。
void ensureUserExists(sql::Connection& conn, int64_t user_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT id FROM `Users` WHERE id = ?"));
    stmt->setInt64(1, user_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        throw std::out_of_range("user not found");
    }
}

// 删除、回复和查询评论前都要确认主题存在，便于返回稳定的 not_found 错误。
bool topicExists(sql::Connection& conn, int64_t topic_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT id FROM discussion_topics WHERE id = ?"));
    stmt->setInt64(1, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    return rs->next();
}

// 删除主题需要校验作者，只返回 owner id，不提前暴露其他主题内容。
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

// 回复只能挂在同一个主题下的评论上，避免跨主题引用导致页面线程混乱。
bool parentCommentBelongsToTopic(sql::Connection& conn, int64_t topic_id, int64_t parent_comment_id) {
    std::unique_ptr<sql::PreparedStatement> stmt(
        conn.prepareStatement("SELECT id FROM discussion_comments WHERE id = ? AND topic_id = ?"));
    stmt->setInt64(1, parent_comment_id);
    stmt->setInt64(2, topic_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    return rs->next();
}

// 删除评论前先取作者，用于权限判断；找不到时交给上层转成 404。
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

// MySQL 递归 CTE 用来统计待删除评论及其所有子回复，保证 comment_count 扣减准确。
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

// 删除整个主题前需要知道还剩多少评论，用于循环删除叶子节点。
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

// 由于评论表有自引用外键，直接删父评论可能被子回复引用；这里从叶子层逐轮删除。
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
    // 先区分主题不存在和评论不存在，接口错误信息会更准确。
    if (!topicExists(conn, topic_id)) {
        throw std::out_of_range("topic not found");
    }

    // 删除评论和维护主题计数必须在同一个事务中完成，避免计数与实际评论数不一致。
    const bool old_autocommit = conn.getAutoCommit();
    conn.setAutoCommit(false);
    try {
        // 只有评论作者本人可以删除；管理员能力如果需要，应在这里单独扩展。
        const auto owner_user_id = commentOwnerUserId(conn, topic_id, comment_id);
        if (!owner_user_id.has_value()) {
            throw std::out_of_range("comment not found");
        }
        if (*owner_user_id != user_id) {
            throw std::domain_error("permission denied");
        }

        // comment_count 需要扣掉整棵回复树，而不是只扣当前这一条。
        const int deleted_count = countCommentTree(conn, topic_id, comment_id);
        if (deleted_count <= 0) {
            throw std::out_of_range("comment not found");
        }

        // 外键 ON DELETE CASCADE 会删除子回复，前面的统计负责给计数字段提供扣减值。
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

        // 恢复原 autocommit，避免连接回池后影响其他业务模块。
        conn.commit();
        conn.setAutoCommit(old_autocommit);
        return deleted_count;
    } catch (...) {
        // 任何异常都回滚，并把连接状态恢复到借出前。
        conn.rollback();
        conn.setAutoCommit(old_autocommit);
        throw;
    }
}

int deleteTopicWithUserId(sql::Connection& conn, int64_t topic_id, int64_t user_id) {
    // 主题删除会级联大量评论，必须和权限校验放进同一个事务。
    const bool old_autocommit = conn.getAutoCommit();
    conn.setAutoCommit(false);
    try {
        // 只允许作者删除自己的主题，避免前端篡改 topic_id 后误删他人讨论。
        const auto owner_user_id = topicOwnerUserId(conn, topic_id);
        if (!owner_user_id.has_value()) {
            throw std::out_of_range("topic not found");
        }
        if (*owner_user_id != user_id) {
            throw std::domain_error("permission denied");
        }

        // 先手动清理评论树，再删除主题，可以兼容不同 MySQL 版本的自引用级联行为。
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
        // 删除主题失败时不能留下半删状态。
        conn.rollback();
        conn.setAutoCommit(old_autocommit);
        throw;
    }
}

#endif

}  // namespace

bool DiscussionService::databaseReady() const {
    // 健康检查只看连接池是否可用，不在这里触发建表，避免 /health 造成写操作。
    return oj::MysqlConnectionPool::instance().available();
}

void DiscussionService::ensureSchema() const {
#ifndef OJ_WITH_MYSQL
    throw std::runtime_error("discussion service requires MySQL; rebuild with OJ_ENABLE_MYSQL=ON");
#else
    {
        std::lock_guard<std::mutex> lock(schema_mutex_);
        // 表结构只需要初始化一次，减少每个请求进入服务层时的数据库开销。
        if (schema_initialized_) {
            return;
        }
    }

    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();

    std::unique_ptr<sql::Statement> stmt(conn->createStatement());
    // 运行时建表用于本地部署兜底，正式环境仍建议执行 sql/discussion_schema.sql。
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

    // comment_count 冗余在主题表中，列表页可以不用每次 COUNT 评论表。
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
    // 只有两张表都创建成功后才标记完成，失败时下一次请求会继续尝试。
    schema_initialized_ = true;
#endif
}

int64_t DiscussionService::createTopic(int64_t problem_id,
                                       int64_t user_id,
                                       const std::string& title,
                                       const std::string& content) {
    // user_id 入口主要给后端测试和可信调用方使用，仍然复用同一套校验。
    validateTopicInput(problem_id, user_id, title, content);
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    // 先查用户可以把外键错误转换成更明确的 user not found。
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
    // username 入口服务于前端，避免浏览器保存或暴露 user_id。
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
    // 用户名解析和主题写入使用同一连接，便于读取自增 ID。
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
    // 列表接口支持按题目过滤，也支持首页展示全部讨论。
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    (void)problem_id;
    (void)limit;
    (void)offset;
    return {};
#else
    if (limit == 0) {
        // limit 为 0 时直接返回空数组，避免向数据库发送无意义查询。
        return {};
    }

    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    std::vector<DiscussionTopic> topics;

    std::unique_ptr<sql::PreparedStatement> stmt;
    if (problem_id.has_value()) {
        // 题目详情页进入讨论区时会带 problem_id，只展示该题相关主题。
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
        // 首页列表按主题 id 倒序，保证新发布的主题最先显示。
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
        // 结果转换统一走 topicFromResult，避免 JSON 字段遗漏。
        topics.push_back(topicFromResult(*rs));
    }
    return topics;
#endif
}

std::optional<DiscussionTopic> DiscussionService::getTopic(int64_t topic_id) const {
    ensureSchema();
    // 非法 id 与不存在的主题都以 nullopt 呈现，由 HTTP 层转成 404。
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
    // 删除接口必须同时带主题和用户，不能只按主题 id 删除。
    if (topic_id <= 0 || user_id <= 0) {
        throw std::invalid_argument("topic_id and user_id must be positive");
    }
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    // user_id 也要确认存在，否则权限错误和用户不存在会被混在一起。
    ensureUserExists(*conn, user_id);
    return deleteTopicWithUserId(*conn, topic_id, user_id);
#endif
}

int DiscussionService::deleteTopicByUsername(int64_t topic_id, const std::string& username) {
    // 前端删除主题走 username，后端在服务层统一解析成 user_id 再做权限判断。
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
    // 评论创建既支持顶层评论，也支持回复某条评论。
    validateCommentInput(topic_id, user_id, content);
    ensureSchema();

#ifndef OJ_WITH_MYSQL
    return 0;
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    ensureUserExists(*conn, user_id);
    if (!topicExists(*conn, topic_id)) {
        // 外键能兜底，但提前检查可以返回更符合 API 语义的 404。
        throw std::out_of_range("topic not found");
    }
    if (parent_comment_id.has_value() &&
        !parentCommentBelongsToTopic(*conn, topic_id, *parent_comment_id)) {
        // 防止把当前主题的回复挂到其他主题评论下面。
        throw std::invalid_argument("parent_comment_id is invalid");
    }

    // 插入评论和递增主题 comment_count 必须原子完成。
    const bool old_autocommit = conn->getAutoCommit();
    conn->setAutoCommit(false);
    try {
        if (parent_comment_id.has_value()) {
            // 回复评论时保存父评论 id，前端据此展示“回复某人”的引用行。
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
            // 顶层评论不写 parent_comment_id，让数据库保持 NULL。
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

        // 更新 updated_at 让有新评论的主题重新浮到列表前方。
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
        // 评论插入失败或计数更新失败都不能提交半成品。
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
    // username 版本和 user_id 版本保持同样的事务语义，只是多一步用户解析。
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

    // 这里重复事务代码是为了保持接口直观，后续可提取成私有 helper。
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

        // comment_count 是列表页性能优化字段，每次写评论都同步维护。
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
    // 删除评论需要同时验证主题、评论和用户，避免只凭 comment_id 误删。
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
    // 浏览器端删除评论同样只传 username，权限最终仍然按 Users.id 判断。
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
    // 评论列表必须依附于一个合法主题，不能查询全站评论。
    if (topic_id <= 0) {
        throw std::out_of_range("topic not found");
    }

#ifndef OJ_WITH_MYSQL
    return {};
#else
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    if (!topicExists(*conn, topic_id)) {
        // 保持“主题不存在”与“主题暂无评论”的返回差异。
        throw std::out_of_range("topic not found");
    }

    // 按 id 升序返回，前端可以直接按时间顺序渲染评论和回复引用。
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
        // 每条评论携带用户展示信息，前端无需额外请求用户模块。
        comments.push_back(commentFromResult(*rs));
    }
    return comments;
#endif
}
