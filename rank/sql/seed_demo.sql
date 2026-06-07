-- 在 Users 表已有用户（id 1-8）的前提下导入演示排行榜数据。
-- 若你的 Users 表用户名不同，请按实际 id 调整。

USE `myOJ`;

INSERT INTO leaderboard_global
    (user_id, solved_count, total_submissions, penalty_seconds, score, last_accepted_at)
VALUES
    (2, 128, 420, 1800, 28600, 1712000010),
    (3, 125, 398, 1650, 28100, 1712000000),
    (4, 118, 350, 1420, 26400, 1712000100),
    (5, 110, 310, 1300, 24200, 1712000200)
AS new_data
ON DUPLICATE KEY UPDATE
    solved_count      = new_data.solved_count,
    total_submissions = new_data.total_submissions,
    penalty_seconds   = new_data.penalty_seconds,
    score             = new_data.score,
    last_accepted_at  = new_data.last_accepted_at;

INSERT INTO user_problem_stats
    (user_id, solved_total, solved_easy, solved_medium, solved_hard)
VALUES
    (2, 128, 42, 52, 34),
    (3, 125, 45, 48, 32),
    (4, 118, 40, 46, 32),
    (5, 110, 38, 42, 30)
AS new_data
ON DUPLICATE KEY UPDATE
    solved_total  = new_data.solved_total,
    solved_easy   = new_data.solved_easy,
    solved_medium = new_data.solved_medium,
    solved_hard   = new_data.solved_hard;

INSERT INTO leaderboard_contest
    (contest_id, user_id, solved_count, penalty_seconds, score, last_accepted_at)
VALUES
    (1001, 2, 8, 520, 1800, 1712000010),
    (1001, 3, 8, 500, 1800, 1712000000),
    (1001, 4, 7, 480, 1750, 1712000100),
    (1001, 5, 6, 450, 1680, 1712000200)
AS new
ON DUPLICATE KEY UPDATE
    solved_count     = new.solved_count,
    penalty_seconds  = new.penalty_seconds,
    score            = new.score,
    last_accepted_at = new.last_accepted_at;

select * from `myOJ`.`leaderboard_global`;
select * from `myOJ`.`user_problem_stats`;
select * from `myOJ`.`leaderboard_contest`;