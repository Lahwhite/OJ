#include "storage/mysql_repository.hpp"

#include "oj/log.h"
#include "storage/mysql_c_wrapper.hpp"

#include <cstdlib>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <string>

#ifdef OJ_WITH_MYSQL
#include <mysql/mysql.h>
#endif

namespace oj {

#ifdef OJ_WITH_MYSQL

namespace {

std::int64_t col_int64(MYSQL_ROW row, int idx) {
    return row[idx] ? static_cast<std::int64_t>(atoll(row[idx])) : 0;
}

std::int32_t col_int(MYSQL_ROW row, int idx) {
    return row[idx] ? static_cast<std::int32_t>(atoi(row[idx])) : 0;
}

std::string col_str(MYSQL_ROW row, int idx, const std::string& fallback = "") {
    return row[idx] ? std::string(row[idx]) : fallback;
}

std::string nullable_username(MYSQL_ROW row, int idx) {
    return col_str(row, idx, "user_unknown");
}

char* safe_str(const std::string& s) {
    return const_cast<char*>(s.c_str());
}

void sql_exec(MYSQL* conn, const std::string& sql) {
    if (mysql_query(conn, sql.c_str()) != 0) {
        throw std::runtime_error(std::string("sql error: ") + mysql_error(conn) + " | sql: " + sql);
    }
}

int sql_exec_result_affected(MYSQL* conn, const std::string& sql) {
    sql_exec(conn, sql);
    return static_cast<int>(mysql_affected_rows(conn));
}

struct ResultGuard {
    MYSQL_RES* res;
    explicit ResultGuard(MYSQL_RES* r) : res(r) {}
    ~ResultGuard() { if (res) mysql_free_result(res); }
    ResultGuard(const ResultGuard&) = delete;
    ResultGuard& operator=(const ResultGuard&) = delete;
};

struct CConnGuard {
    MysqlCConfig cfg;
    MysqlCConnection conn;
    CConnGuard() : cfg(load_mysql_config()), conn(cfg) {
        if (!conn.connected()) {
            throw std::runtime_error("mysql c connection is not available");
        }
    }
};

}  // namespace

bool MysqlLeaderboardRepository::database_ready() {
    try {
        CConnGuard guard;
        sql_exec(guard.conn.get(),
                 "SELECT 1 FROM information_schema.tables "
                 "WHERE table_schema = DATABASE() AND table_name = 'leaderboard_global' LIMIT 1");
        ResultGuard rg(mysql_store_result(guard.conn.get()));
        return rg.res && mysql_num_rows(rg.res) > 0;
    } catch (const std::exception& e) {
        OJ_LOG_WARN(std::string("rank mysql schema check failed: ") + e.what());
        return false;
    }
}

std::vector<LeaderboardRow> MysqlLeaderboardRepository::get_global_rows() {
    CConnGuard guard;
    std::vector<LeaderboardRow> rows;
    sql_exec(guard.conn.get(),
             "SELECT g.user_id, COALESCE(u.username, CONCAT('user_', g.user_id)) AS username, "
             "g.solved_count, g.total_submissions, g.penalty_seconds, g.score, g.last_accepted_at "
             "FROM leaderboard_global g "
             "LEFT JOIN `Users` u ON u.id = g.user_id");
    ResultGuard rg(mysql_store_result(guard.conn.get()));
    if (!rg.res) return rows;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(rg.res))) {
        LeaderboardRow r;
        r.user_id = col_int64(row, 0);
        r.username = nullable_username(row, 1);
        r.solved_count = col_int(row, 2);
        r.total_submissions = col_int(row, 3);
        r.penalty_seconds = col_int64(row, 4);
        r.score = col_int64(row, 5);
        r.last_accepted_at = col_int64(row, 6);
        rows.push_back(r);
    }
    return rows;
}

std::optional<ProblemCompletionStats> MysqlLeaderboardRepository::get_problem_stats(std::int64_t user_id) {
    CConnGuard guard;
    std::ostringstream sql;
    sql << "SELECT user_id, solved_total, solved_easy, solved_medium, solved_hard "
        << "FROM user_problem_stats WHERE user_id = " << user_id;
    sql_exec(guard.conn.get(), sql.str());
    ResultGuard rg(mysql_store_result(guard.conn.get()));
    if (!rg.res || mysql_num_rows(rg.res) == 0) return std::nullopt;

    MYSQL_ROW row = mysql_fetch_row(rg.res);
    ProblemCompletionStats stats;
    stats.user_id = col_int64(row, 0);
    stats.solved_total = col_int(row, 1);
    stats.solved_easy = col_int(row, 2);
    stats.solved_medium = col_int(row, 3);
    stats.solved_hard = col_int(row, 4);
    return stats;
}

std::vector<ContestLeaderboardRow> MysqlLeaderboardRepository::get_contest_rows(std::int64_t contest_id) {
    CConnGuard guard;
    std::vector<ContestLeaderboardRow> rows;
    std::ostringstream sql;
    sql << "SELECT c.contest_id, c.user_id, COALESCE(u.username, CONCAT('user_', c.user_id)) AS username, "
        << "c.solved_count, c.penalty_seconds, c.score, c.last_accepted_at "
        << "FROM leaderboard_contest c "
        << "LEFT JOIN `Users` u ON u.id = c.user_id "
        << "WHERE c.contest_id = " << contest_id;
    sql_exec(guard.conn.get(), sql.str());
    ResultGuard rg(mysql_store_result(guard.conn.get()));
    if (!rg.res) return rows;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(rg.res))) {
        ContestLeaderboardRow r;
        r.contest_id = col_int64(row, 0);
        r.user_id = col_int64(row, 1);
        r.username = nullable_username(row, 2);
        r.solved_count = col_int(row, 3);
        r.penalty_seconds = col_int64(row, 4);
        r.score = col_int64(row, 5);
        r.last_accepted_at = col_int64(row, 6);
        rows.push_back(r);
    }
    return rows;
}

void MysqlLeaderboardRepository::apply_submission_event(const SubmissionEvent& event, std::int64_t score_delta) {
    CConnGuard guard;
    MYSQL* conn = guard.conn.get();
    mysql_autocommit(conn, 0);

    const int diff = static_cast<int>(event.difficulty);
    const char* verdict = event.is_accepted ? "AC" : "FAIL";
    const char* contest_val = event.contest_id > 0 ? std::to_string(event.contest_id).c_str() : "NULL";
    const char* contest_null = event.contest_id > 0 ? "" : "NULL";

    try {
        std::ostringstream sub_sql;
        sub_sql << "INSERT INTO rank_submissions "
                << "(id, user_id, problem_id, " << (event.contest_id > 0 ? "contest_id, " : "")
                << "verdict, difficulty, submit_at, penalty_seconds) "
                << "VALUES (" << event.submission_id << ", "
                << event.user_id << ", "
                << event.problem_id << ", "
                << (event.contest_id > 0 ? contest_val : std::string("NULL")) << ", "
                << "'" << verdict << "', "
                << diff << ", "
                << event.submit_at << ", "
                << event.penalty_seconds << ")";
        sql_exec(conn, sub_sql.str());

        {
            std::ostringstream s;
            s << "INSERT INTO leaderboard_global "
              << "(user_id, solved_count, total_submissions, penalty_seconds, score, last_accepted_at) "
              << "VALUES (" << event.user_id << ", 0, 1, " << event.penalty_seconds << ", 0, 0) "
              << "ON DUPLICATE KEY UPDATE "
              << "total_submissions = total_submissions + 1, "
              << "penalty_seconds = penalty_seconds + " << event.penalty_seconds;
            sql_exec(conn, s.str());
        }

        {
            std::ostringstream s;
            s << "INSERT INTO user_problem_stats "
              << "(user_id, solved_total, solved_easy, solved_medium, solved_hard) "
              << "VALUES (" << event.user_id << ", 0, 0, 0, 0) "
              << "ON DUPLICATE KEY UPDATE user_id = user_id";
            sql_exec(conn, s.str());
        }

        if (event.is_accepted) {
            std::ostringstream ss;
            ss << "INSERT IGNORE INTO rank_solved_problems "
               << "(user_id, problem_id, difficulty, first_accepted_at) VALUES ("
               << event.user_id << ", "
               << event.problem_id << ", "
               << diff << ", "
               << event.submit_at << ")";
            const int solved_inserted = sql_exec_result_affected(conn, ss.str());

            {
                std::ostringstream s;
                s << "UPDATE leaderboard_global SET "
                  << "score = score + " << score_delta << ", "
                  << "last_accepted_at = " << event.submit_at << ", "
                  << "solved_count = solved_count + " << (solved_inserted > 0 ? 1 : 0) << " "
                  << "WHERE user_id = " << event.user_id;
                sql_exec(conn, s.str());
            }

            if (solved_inserted > 0) {
                const int e = event.difficulty == ProblemDifficulty::Easy ? 1 : 0;
                const int m = event.difficulty == ProblemDifficulty::Medium ? 1 : 0;
                const int h = event.difficulty == ProblemDifficulty::Hard ? 1 : 0;
                std::ostringstream s;
                s << "UPDATE user_problem_stats SET "
                  << "solved_total = solved_total + 1, "
                  << "solved_easy = solved_easy + " << e << ", "
                  << "solved_medium = solved_medium + " << m << ", "
                  << "solved_hard = solved_hard + " << h << " "
                  << "WHERE user_id = " << event.user_id;
                sql_exec(conn, s.str());
            }
        }

        if (event.contest_id > 0) {
            {
                std::ostringstream s;
                s << "INSERT INTO leaderboard_contest "
                  << "(contest_id, user_id, solved_count, penalty_seconds, score, last_accepted_at) "
                  << "VALUES (" << event.contest_id << ", " << event.user_id << ", 0, "
                  << event.penalty_seconds << ", 0, 0) "
                  << "ON DUPLICATE KEY UPDATE "
                  << "penalty_seconds = penalty_seconds + " << event.penalty_seconds;
                sql_exec(conn, s.str());
            }

            if (event.is_accepted) {
                std::ostringstream ss;
                ss << "INSERT IGNORE INTO rank_contest_solved_problems "
                   << "(contest_id, user_id, problem_id, first_accepted_at) VALUES ("
                   << event.contest_id << ", " << event.user_id << ", "
                   << event.problem_id << ", " << event.submit_at << ")";
                const int csolved = sql_exec_result_affected(conn, ss.str());

                std::ostringstream s;
                s << "UPDATE leaderboard_contest SET "
                  << "score = score + " << score_delta << ", "
                  << "last_accepted_at = " << event.submit_at << ", "
                  << "solved_count = solved_count + " << (csolved > 0 ? 1 : 0) << " "
                  << "WHERE contest_id = " << event.contest_id
                  << " AND user_id = " << event.user_id;
                sql_exec(conn, s.str());
            }
        }

        mysql_commit(conn);
        mysql_autocommit(conn, 1);
        OJ_LOG_INFO("rank submission persisted, user_id=" + std::to_string(event.user_id) +
                    " submission_id=" + std::to_string(event.submission_id));
    } catch (...) {
        mysql_rollback(conn);
        mysql_autocommit(conn, 1);
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
