-- ============================================
-- 数据插入与查询测试（用户、题目、讨论区、排行榜）
-- ============================================

USE `myOJ`;
SET NAMES utf8mb4;

-- ----------------------------
-- 1. 插入基础用户数据（id 自增，将生成 1~8）
-- ----------------------------
INSERT INTO Users (username, password) VALUES
('Fooly_Cooly','pwd'), ('Fire_Starter','pwd'), ('Marquis_de_Carabas','pwd'), ('Full_Swing','pwd'),
('Brittle_Bullet','pwd'), ('FLCLimax','pwd'), ('Eri_Ninamori','pwd'), ('Der_ya','pwd');

-- 为 Eri_Ninamori 添加个人资料
INSERT INTO `Profiles` (username, nickname, signature)
VALUES ('Eri_Ninamori', '蜷守惠理', '我在海上耕耘，风中播种🌧');

-- ----------------------------
-- 2. 题目系统基础数据
-- ----------------------------
INSERT INTO problem_users (username, role)
VALUES ('admin', 'admin');

INSERT INTO problems (
    title,
    description,
    difficulty,
    time_limit,
    memory_limit,
    input_description,
    output_description,
    sample_input,
    sample_output,
    created_by,
    is_public,
    submission_count,
    accepted_count
) VALUES (
    'Hello Problem',
    'This is a test problem for the problem management module.',
    'easy',
    1000,
    128,
    'Input a string.',
    'Output the same string.',
    'Hello',
    'Hello',
    1,
    TRUE,
    10,
    8
);

INSERT INTO test_cases (problem_id, input, output, is_sample, score)
VALUES
    (1, 'Hello\n', 'Hello\n', TRUE, 20),
    (1, 'World\n', 'World\n', FALSE, 80);

INSERT INTO tags (name, color, problem_count)
VALUES ('入门', '#1890ff', 1);

INSERT INTO problem_tags (problem_id, tag_id)
VALUES (1, 1);

INSERT INTO problem_user_status (
    user_id,
    problem_id,
    accepted,
    best_score,
    last_score,
    max_score,
    last_submitted_at,
    accepted_at
) VALUES (
    1,
    1,
    TRUE,
    100,
    100,
    100,
    CURRENT_TIMESTAMP,
    CURRENT_TIMESTAMP
);

-- ----------------------------
-- 3. 排行榜演示数据（依赖 Users 表中存在 id 2~5）
-- ----------------------------
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

-- ============================================
-- 查询验证（覆盖所有模块）
-- ============================================
SELECT * FROM `myOJ`.`Users`;
SELECT * FROM `myOJ`.`Profiles`;
SELECT * FROM `myOJ`.`problem_users`;
SELECT * FROM `myOJ`.`problems`;
SELECT * FROM `myOJ`.`test_cases`;
SELECT * FROM `myOJ`.`tags`;
SELECT * FROM `myOJ`.`problem_tags`;
SELECT * FROM `myOJ`.`problem_user_status`;
SELECT * FROM `myOJ`.`discussion_topics`;
SELECT * FROM `myOJ`.`discussion_comments`;
SELECT * FROM `myOJ`.`leaderboard_global`;
SELECT * FROM `myOJ`.`user_problem_stats`;
SELECT * FROM `myOJ`.`leaderboard_contest`;