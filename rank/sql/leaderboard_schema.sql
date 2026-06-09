CREATE DATABASE IF NOT EXISTS `myOJ`
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE `myOJ`;

CREATE TABLE IF NOT EXISTS `rank_submissions` (
    id BIGINT PRIMARY KEY,
    user_id BIGINT NOT NULL,
    problem_id BIGINT NOT NULL,
    contest_id BIGINT NULL,
    verdict VARCHAR(32) NOT NULL,
    difficulty TINYINT NOT NULL,
    submit_at BIGINT NOT NULL,
    penalty_seconds BIGINT NOT NULL DEFAULT 0,
    INDEX idx_rank_submissions_user_submit(user_id, submit_at),
    INDEX idx_rank_submissions_contest_user(contest_id, user_id),
    CONSTRAINT fk_rank_submissions_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `leaderboard_global` (
    user_id BIGINT PRIMARY KEY,
    solved_count INT NOT NULL DEFAULT 0,
    total_submissions INT NOT NULL DEFAULT 0,
    penalty_seconds BIGINT NOT NULL DEFAULT 0,
    score BIGINT NOT NULL DEFAULT 0,
    last_accepted_at BIGINT NOT NULL DEFAULT 0,
    INDEX idx_rank_global(score DESC, solved_count DESC, penalty_seconds ASC, last_accepted_at ASC),
    CONSTRAINT fk_leaderboard_global_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `user_problem_stats` (
    user_id BIGINT PRIMARY KEY,
    solved_total INT NOT NULL DEFAULT 0,
    solved_easy INT NOT NULL DEFAULT 0,
    solved_medium INT NOT NULL DEFAULT 0,
    solved_hard INT NOT NULL DEFAULT 0,
    CONSTRAINT fk_user_problem_stats_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `leaderboard_contest` (
    contest_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    solved_count INT NOT NULL DEFAULT 0,
    penalty_seconds BIGINT NOT NULL DEFAULT 0,
    score BIGINT NOT NULL DEFAULT 0,
    last_accepted_at BIGINT NOT NULL DEFAULT 0,
    PRIMARY KEY (contest_id, user_id),
    INDEX idx_rank_contest(contest_id, score DESC, solved_count DESC, penalty_seconds ASC, last_accepted_at ASC),
    CONSTRAINT fk_leaderboard_contest_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `rank_solved_problems` (
    user_id BIGINT NOT NULL,
    problem_id BIGINT NOT NULL,
    difficulty TINYINT NOT NULL,
    first_accepted_at BIGINT NOT NULL,
    PRIMARY KEY (user_id, problem_id),
    CONSTRAINT fk_rank_solved_problems_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `rank_contest_solved_problems` (
    contest_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    problem_id BIGINT NOT NULL,
    first_accepted_at BIGINT NOT NULL,
    PRIMARY KEY (contest_id, user_id, problem_id),
    CONSTRAINT fk_rank_contest_solved_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
