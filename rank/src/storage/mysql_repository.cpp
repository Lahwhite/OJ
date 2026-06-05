#include "storage/mysql_repository.hpp"

#include "oj/log.h"
#include "oj/mysql_pool.h"

#include <stdexcept>
#include <string>

#ifdef OJ_WITH_MYSQL
#include <cppconn/connection.h>
#include <cppconn/exception.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#endif

namespace oj {

#ifdef OJ_WITH_MYSQL

namespace {

class MysqlConnectionGuard {
public:
    MysqlConnectionGuard() : conn_(static_cast<sql::Connection*>(MysqlConnectionPool::instance().acquire())) {
        if (!conn_) {
            throw std::runtime_error("mysql connection is not available");
        }
    }

    ~MysqlConnectionGuard() {
        MysqlConnectionPool::instance().release(conn_);
    }

    sql::Connection* get() const {
        return conn_;
    }

private:
    sql::Connection* conn_;
};

std::string nullable_username(sql::ResultSet& rs) {
    const std::string value = rs.getString("username");
    return rs.wasNull() ? std::string("user_unknown") : value;
}

std::int32_t difficulty_value(ProblemDifficulty difficulty) {
    return static_cast<std::int32_t>(difficulty);
}

}  // namespace

bool MysqlLeaderboardRepository::database_ready() {
    if (!MysqlConnectionPool::instance().available()) {
        return false;
    }
    try {
        MysqlConnectionGuard guard;
        std::unique_ptr<sql::Statement> stmt(guard.get()->createStatement());
        std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery(
            "SELECT 1 FROM information_schema.tables "
            "WHERE table_schema = DATABASE() AND table_name = 'leaderboard_global' LIMIT 1"));
        return rs->next();
    } catch (const std::exception& e) {
        OJ_LOG_WARN(std::string("rank mysql schema check failed: ") + e.what());
        return false;
    }
}

std::vector<LeaderboardRow> MysqlLeaderboardRepository::get_global_rows() {
    MysqlConnectionGuard guard;
    std::vector<LeaderboardRow> rows;
    std::unique_ptr<sql::Statement> stmt(guard.get()->createStatement());
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery(
        "SELECT g.user_id, COALESCE(u.username, CONCAT('user_', g.user_id)) AS username, "
        "g.solved_count, g.total_submissions, g.penalty_seconds, g.score, g.last_accepted_at "
        "FROM leaderboard_global g "
        "LEFT JOIN `Users` u ON u.id = g.user_id"));

    while (rs->next()) {
        LeaderboardRow row;
        row.user_id = rs->getInt64("user_id");
        row.username = nullable_username(*rs);
        row.solved_count = rs->getInt("solved_count");
        row.total_submissions = rs->getInt("total_submissions");
        row.penalty_seconds = rs->getInt64("penalty_seconds");
        row.score = rs->getInt64("score");
        row.last_accepted_at = rs->getInt64("last_accepted_at");
        rows.push_back(row);
    }
    return rows;
}

std::optional<ProblemCompletionStats> MysqlLeaderboardRepository::get_problem_stats(std::int64_t user_id) {
    MysqlConnectionGuard guard;
    std::unique_ptr<sql::PreparedStatement> stmt(guard.get()->prepareStatement(
        "SELECT user_id, solved_total, solved_easy, solved_medium, solved_hard "
        "FROM user_problem_stats WHERE user_id = ?"));
    stmt->setInt64(1, user_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());
    if (!rs->next()) {
        return std::nullopt;
    }

    ProblemCompletionStats stats;
    stats.user_id = rs->getInt64("user_id");
    stats.solved_total = rs->getInt("solved_total");
    stats.solved_easy = rs->getInt("solved_easy");
    stats.solved_medium = rs->getInt("solved_medium");
    stats.solved_hard = rs->getInt("solved_hard");
    return stats;
}

std::vector<ContestLeaderboardRow> MysqlLeaderboardRepository::get_contest_rows(std::int64_t contest_id) {
    MysqlConnectionGuard guard;
    std::vector<ContestLeaderboardRow> rows;
    std::unique_ptr<sql::PreparedStatement> stmt(guard.get()->prepareStatement(
        "SELECT c.contest_id, c.user_id, COALESCE(u.username, CONCAT('user_', c.user_id)) AS username, "
        "c.solved_count, c.penalty_seconds, c.score, c.last_accepted_at "
        "FROM leaderboard_contest c "
        "LEFT JOIN `Users` u ON u.id = c.user_id "
        "WHERE c.contest_id = ?"));
    stmt->setInt64(1, contest_id);
    std::unique_ptr<sql::ResultSet> rs(stmt->executeQuery());

    while (rs->next()) {
        ContestLeaderboardRow row;
        row.contest_id = rs->getInt64("contest_id");
        row.user_id = rs->getInt64("user_id");
        row.username = nullable_username(*rs);
        row.solved_count = rs->getInt("solved_count");
        row.penalty_seconds = rs->getInt64("penalty_seconds");
        row.score = rs->getInt64("score");
        row.last_accepted_at = rs->getInt64("last_accepted_at");
        rows.push_back(row);
    }
    return rows;
}

void MysqlLeaderboardRepository::apply_submission_event(const SubmissionEvent& event, std::int64_t score_delta) {
    MysqlConnectionGuard guard;
    sql::Connection* conn = guard.get();
    conn->setAutoCommit(false);

    try {
        std::unique_ptr<sql::PreparedStatement> submission_stmt(conn->prepareStatement(
            "INSERT INTO rank_submissions "
            "(id, user_id, problem_id, contest_id, verdict, difficulty, submit_at, penalty_seconds) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?)"));
        submission_stmt->setInt64(1, event.submission_id);
        submission_stmt->setInt64(2, event.user_id);
        submission_stmt->setInt64(3, event.problem_id);
        if (event.contest_id > 0) {
            submission_stmt->setInt64(4, event.contest_id);
        } else {
            submission_stmt->setNull(4, sql::DataType::BIGINT);
        }
        submission_stmt->setString(5, event.is_accepted ? "AC" : "FAIL");
        submission_stmt->setInt(6, difficulty_value(event.difficulty));
        submission_stmt->setInt64(7, event.submit_at);
        submission_stmt->setInt64(8, event.penalty_seconds);
        submission_stmt->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> global_upsert(conn->prepareStatement(
            "INSERT INTO leaderboard_global "
            "(user_id, solved_count, total_submissions, penalty_seconds, score, last_accepted_at) "
            "VALUES (?, 0, 1, ?, 0, 0) "
            "ON DUPLICATE KEY UPDATE "
            "total_submissions = total_submissions + 1, "
            "penalty_seconds = penalty_seconds + VALUES(penalty_seconds)"));
        global_upsert->setInt64(1, event.user_id);
        global_upsert->setInt64(2, event.penalty_seconds);
        global_upsert->executeUpdate();

        std::unique_ptr<sql::PreparedStatement> stats_upsert(conn->prepareStatement(
            "INSERT INTO user_problem_stats "
            "(user_id, solved_total, solved_easy, solved_medium, solved_hard) "
            "VALUES (?, 0, 0, 0, 0) "
            "ON DUPLICATE KEY UPDATE user_id = user_id"));
        stats_upsert->setInt64(1, event.user_id);
        stats_upsert->executeUpdate();

        if (event.is_accepted) {
            std::unique_ptr<sql::PreparedStatement> solved_stmt(conn->prepareStatement(
                "INSERT IGNORE INTO rank_solved_problems "
                "(user_id, problem_id, difficulty, first_accepted_at) VALUES (?, ?, ?, ?)"));
            solved_stmt->setInt64(1, event.user_id);
            solved_stmt->setInt64(2, event.problem_id);
            solved_stmt->setInt(3, difficulty_value(event.difficulty));
            solved_stmt->setInt64(4, event.submit_at);
            const auto solved_inserted = solved_stmt->executeUpdate();

            std::unique_ptr<sql::PreparedStatement> global_ac_stmt(conn->prepareStatement(
                "UPDATE leaderboard_global SET "
                "score = score + ?, last_accepted_at = ?, solved_count = solved_count + ? "
                "WHERE user_id = ?"));
            global_ac_stmt->setInt64(1, score_delta);
            global_ac_stmt->setInt64(2, event.submit_at);
            global_ac_stmt->setInt(3, solved_inserted > 0 ? 1 : 0);
            global_ac_stmt->setInt64(4, event.user_id);
            global_ac_stmt->executeUpdate();

            if (solved_inserted > 0) {
                std::unique_ptr<sql::PreparedStatement> stats_ac_stmt(conn->prepareStatement(
                    "UPDATE user_problem_stats SET "
                    "solved_total = solved_total + 1, "
                    "solved_easy = solved_easy + ?, "
                    "solved_medium = solved_medium + ?, "
                    "solved_hard = solved_hard + ? "
                    "WHERE user_id = ?"));
                const auto easy_inc = event.difficulty == ProblemDifficulty::Easy ? 1 : 0;
                const auto medium_inc = event.difficulty == ProblemDifficulty::Medium ? 1 : 0;
                const auto hard_inc = event.difficulty == ProblemDifficulty::Hard ? 1 : 0;
                stats_ac_stmt->setInt(1, easy_inc);
                stats_ac_stmt->setInt(2, medium_inc);
                stats_ac_stmt->setInt(3, hard_inc);
                stats_ac_stmt->setInt64(4, event.user_id);
                stats_ac_stmt->executeUpdate();
            }
        }

        if (event.contest_id > 0) {
            std::unique_ptr<sql::PreparedStatement> contest_upsert(conn->prepareStatement(
                "INSERT INTO leaderboard_contest "
                "(contest_id, user_id, solved_count, penalty_seconds, score, last_accepted_at) "
                "VALUES (?, ?, 0, ?, 0, 0) "
                "ON DUPLICATE KEY UPDATE "
                "penalty_seconds = penalty_seconds + VALUES(penalty_seconds)"));
            contest_upsert->setInt64(1, event.contest_id);
            contest_upsert->setInt64(2, event.user_id);
            contest_upsert->setInt64(3, event.penalty_seconds);
            contest_upsert->executeUpdate();

            if (event.is_accepted) {
                std::unique_ptr<sql::PreparedStatement> contest_solved_stmt(conn->prepareStatement(
                    "INSERT IGNORE INTO rank_contest_solved_problems "
                    "(contest_id, user_id, problem_id, first_accepted_at) VALUES (?, ?, ?, ?)"));
                contest_solved_stmt->setInt64(1, event.contest_id);
                contest_solved_stmt->setInt64(2, event.user_id);
                contest_solved_stmt->setInt64(3, event.problem_id);
                contest_solved_stmt->setInt64(4, event.submit_at);
                const auto contest_solved_inserted = contest_solved_stmt->executeUpdate();

                std::unique_ptr<sql::PreparedStatement> contest_ac_stmt(conn->prepareStatement(
                    "UPDATE leaderboard_contest SET "
                    "score = score + ?, last_accepted_at = ?, solved_count = solved_count + ? "
                    "WHERE contest_id = ? AND user_id = ?"));
                contest_ac_stmt->setInt64(1, score_delta);
                contest_ac_stmt->setInt64(2, event.submit_at);
                contest_ac_stmt->setInt(3, contest_solved_inserted > 0 ? 1 : 0);
                contest_ac_stmt->setInt64(4, event.contest_id);
                contest_ac_stmt->setInt64(5, event.user_id);
                contest_ac_stmt->executeUpdate();
            }
        }

        conn->commit();
        conn->setAutoCommit(true);
        OJ_LOG_INFO("rank submission persisted, user_id=" + std::to_string(event.user_id) +
                    " submission_id=" + std::to_string(event.submission_id));
    } catch (...) {
        conn->rollback();
        conn->setAutoCommit(true);
        throw;
    }
}

#else

bool MysqlLeaderboardRepository::database_ready() {
    return false;
}

std::vector<LeaderboardRow> MysqlLeaderboardRepository::get_global_rows() {
    throw std::runtime_error("rank mysql repository requires OJ_ENABLE_MYSQL=ON");
}

std::optional<ProblemCompletionStats> MysqlLeaderboardRepository::get_problem_stats(std::int64_t) {
    throw std::runtime_error("rank mysql repository requires OJ_ENABLE_MYSQL=ON");
}

std::vector<ContestLeaderboardRow> MysqlLeaderboardRepository::get_contest_rows(std::int64_t) {
    throw std::runtime_error("rank mysql repository requires OJ_ENABLE_MYSQL=ON");
}

void MysqlLeaderboardRepository::apply_submission_event(const SubmissionEvent&, std::int64_t) {
    throw std::runtime_error("rank mysql repository requires OJ_ENABLE_MYSQL=ON");
}

#endif

}  // namespace oj
