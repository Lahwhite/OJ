CREATE TABLE IF NOT EXISTS users (
    id BIGINT PRIMARY KEY,
    username VARCHAR(64) NOT NULL UNIQUE
);

CREATE TABLE IF NOT EXISTS submissions (
    id BIGINT PRIMARY KEY,
    user_id BIGINT NOT NULL,
    problem_id BIGINT NOT NULL,
    contest_id BIGINT NULL,
    verdict VARCHAR(32) NOT NULL,
    difficulty TINYINT NOT NULL,
    submit_at BIGINT NOT NULL,
    penalty_seconds BIGINT NOT NULL DEFAULT 0,
    INDEX idx_user_submit(user_id, submit_at),
    INDEX idx_contest_user(contest_id, user_id)
);

CREATE TABLE IF NOT EXISTS leaderboard_global (
    user_id BIGINT PRIMARY KEY,
    solved_count INT NOT NULL DEFAULT 0,
    total_submissions INT NOT NULL DEFAULT 0,
    penalty_seconds BIGINT NOT NULL DEFAULT 0,
    score BIGINT NOT NULL DEFAULT 0,
    last_accepted_at BIGINT NOT NULL DEFAULT 0,
    INDEX idx_rank(score DESC, solved_count DESC, penalty_seconds ASC, last_accepted_at ASC)
);

CREATE TABLE IF NOT EXISTS user_problem_stats (
    user_id BIGINT PRIMARY KEY,
    solved_total INT NOT NULL DEFAULT 0,
    solved_easy INT NOT NULL DEFAULT 0,
    solved_medium INT NOT NULL DEFAULT 0,
    solved_hard INT NOT NULL DEFAULT 0
);

CREATE TABLE IF NOT EXISTS leaderboard_contest (
    contest_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    solved_count INT NOT NULL DEFAULT 0,
    penalty_seconds BIGINT NOT NULL DEFAULT 0,
    score BIGINT NOT NULL DEFAULT 0,
    last_accepted_at BIGINT NOT NULL DEFAULT 0,
    PRIMARY KEY (contest_id, user_id),
    INDEX idx_contest_rank(contest_id, score DESC, solved_count DESC, penalty_seconds ASC, last_accepted_at ASC)
);
