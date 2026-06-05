-- 在 Users 表已有用户（id 1-8）的前提下导入演示排行榜数据。
-- 若你的 Users 表用户名不同，请按实际 id 调整。

USE `myOJ`;

INSERT INTO leaderboard_global
    (user_id, solved_count, total_submissions, penalty_seconds, score, last_accepted_at)
VALUES
    (1, 128, 420, 1800, 28600, 1712000010),
    (2, 125, 398, 1650, 28100, 1712000000),
    (3, 118, 350, 1420, 26400, 1712000100),
    (4, 110, 310, 1300, 24200, 1712000200)
ON DUPLICATE KEY UPDATE
    solved_count = VALUES(solved_count),
    total_submissions = VALUES(total_submissions),
    penalty_seconds = VALUES(penalty_seconds),
    score = VALUES(score),
    last_accepted_at = VALUES(last_accepted_at);

INSERT INTO user_problem_stats
    (user_id, solved_total, solved_easy, solved_medium, solved_hard)
VALUES
    (1, 128, 42, 52, 34),
    (2, 125, 45, 48, 32),
    (3, 118, 40, 46, 32),
    (4, 110, 38, 42, 30)
ON DUPLICATE KEY UPDATE
    solved_total = VALUES(solved_total),
    solved_easy = VALUES(solved_easy),
    solved_medium = VALUES(solved_medium),
    solved_hard = VALUES(solved_hard);

INSERT INTO leaderboard_contest
    (contest_id, user_id, solved_count, penalty_seconds, score, last_accepted_at)
VALUES
    (1001, 1, 8, 520, 1800, 1712000010),
    (1001, 2, 8, 500, 1800, 1712000000),
    (1001, 3, 7, 480, 1750, 1712000100),
    (1001, 4, 6, 450, 1680, 1712000200)
ON DUPLICATE KEY UPDATE
    solved_count = VALUES(solved_count),
    penalty_seconds = VALUES(penalty_seconds),
    score = VALUES(score),
    last_accepted_at = VALUES(last_accepted_at);
