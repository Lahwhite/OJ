-- ============================================
-- 数据库与所有表结构定义（用户、题目、讨论区、排行榜）
-- ============================================
CREATE DATABASE IF NOT EXISTS `myOJ`
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE `myOJ`;

-- ----------------------------
-- 原有用户表
-- ----------------------------
CREATE TABLE IF NOT EXISTS `Users` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(32) NOT NULL UNIQUE,
    password VARCHAR(60) NOT NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `Profiles` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(32) NOT NULL UNIQUE,
    nickname VARCHAR(64) CHARACTER SET utf8mb4,
    signature VARCHAR(255) CHARACTER SET utf8mb4,
    avatar VARCHAR(255)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- 题目系统用户表
-- ----------------------------
CREATE TABLE IF NOT EXISTS problem_users (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '用户ID',
    username VARCHAR(50) NOT NULL COMMENT '用户名',
    role ENUM('user', 'admin') DEFAULT 'user' COMMENT '角色',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    UNIQUE KEY uk_username (username)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户表';

-- ----------------------------
-- 题目表
-- ----------------------------
CREATE TABLE IF NOT EXISTS problems (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '题目ID',
    title VARCHAR(255) NOT NULL COMMENT '题目标题',
    description TEXT NOT NULL COMMENT '题目描述',
    difficulty ENUM('easy', 'medium', 'hard') NOT NULL COMMENT '难度',
    time_limit INT DEFAULT 1000 COMMENT '时间限制(ms)',
    memory_limit INT DEFAULT 128 COMMENT '内存限制(MB)',
    input_description TEXT NOT NULL COMMENT '输入说明',
    output_description TEXT NOT NULL COMMENT '输出说明',
    sample_input TEXT NOT NULL COMMENT '样例输入',
    sample_output TEXT NOT NULL COMMENT '样例输出',
    created_by BIGINT UNSIGNED DEFAULT NULL COMMENT '创建者ID',
    is_public BOOLEAN DEFAULT TRUE COMMENT '是否公开',
    submission_count INT DEFAULT 0 COMMENT '提交次数',
    accepted_count INT DEFAULT 0 COMMENT '通过次数',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    INDEX idx_difficulty (difficulty),
    INDEX idx_created_by (created_by),
    INDEX idx_is_public (is_public),
    INDEX idx_created_at (created_at),
    FULLTEXT INDEX ft_title (title),
    FULLTEXT INDEX ft_description (description),
    CONSTRAINT fk_problem_creator FOREIGN KEY (created_by) REFERENCES problem_users(id) ON DELETE SET NULL
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='题目表';

-- ----------------------------
-- 用户题目状态表
-- ----------------------------
CREATE TABLE IF NOT EXISTS problem_user_status (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '用户题目状态ID',
    user_id BIGINT NOT NULL COMMENT '外部用户ID',
    problem_id BIGINT UNSIGNED NOT NULL COMMENT '题目ID',
    accepted BOOLEAN NOT NULL DEFAULT FALSE COMMENT '是否AC',
    best_score INT NOT NULL DEFAULT 0 COMMENT '历史最高分',
    last_score INT DEFAULT NULL COMMENT '最近一次得分',
    max_score INT DEFAULT NULL COMMENT '最近评测理论最高分',
    last_submitted_at TIMESTAMP NULL DEFAULT NULL COMMENT '最近提交时间',
    accepted_at TIMESTAMP NULL DEFAULT NULL COMMENT '首次AC时间',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    UNIQUE KEY uk_problem_user_status (user_id, problem_id),
    INDEX idx_status_user_id (user_id),
    INDEX idx_status_problem_id (problem_id),
    INDEX idx_status_accepted (accepted),
    CONSTRAINT fk_problem_user_status_problem FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='用户题目AC状态表';

-- ----------------------------
-- 测试用例表
-- ----------------------------
CREATE TABLE IF NOT EXISTS test_cases (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '测试用例ID',
    problem_id BIGINT UNSIGNED NOT NULL COMMENT '题目ID',
    input TEXT NOT NULL COMMENT '输入数据',
    output TEXT NOT NULL COMMENT '期望输出',
    is_sample BOOLEAN DEFAULT FALSE COMMENT '是否为样例',
    score INT DEFAULT 0 COMMENT '分值',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    INDEX idx_problem_id (problem_id),
    INDEX idx_is_sample (is_sample),
    CONSTRAINT fk_test_case_problem FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='测试用例表';

-- ----------------------------
-- 标签表
-- ----------------------------
CREATE TABLE IF NOT EXISTS tags (
    id BIGINT UNSIGNED PRIMARY KEY AUTO_INCREMENT COMMENT '标签ID',
    name VARCHAR(50) NOT NULL COMMENT '标签名称',
    color VARCHAR(7) DEFAULT '#1890ff' COMMENT '标签颜色',
    problem_count INT DEFAULT 0 COMMENT '关联题目数量',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    UNIQUE KEY uk_name (name),
    INDEX idx_problem_count (problem_count)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='标签表';

-- ----------------------------
-- 题目标签关联表
-- ----------------------------
CREATE TABLE IF NOT EXISTS problem_tags (
    problem_id BIGINT UNSIGNED NOT NULL COMMENT '题目ID',
    tag_id BIGINT UNSIGNED NOT NULL COMMENT '标签ID',
    PRIMARY KEY (problem_id, tag_id),
    CONSTRAINT fk_problem_tags_problem FOREIGN KEY (problem_id) REFERENCES problems(id) ON DELETE CASCADE,
    CONSTRAINT fk_problem_tags_tag FOREIGN KEY (tag_id) REFERENCES tags(id) ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COMMENT='题目标签关联表';

-- ----------------------------
-- 讨论区：主题表
-- ----------------------------
CREATE TABLE IF NOT EXISTS `discussion_topics` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    problem_id BIGINT NOT NULL COMMENT '关联题目ID',
    user_id BIGINT NOT NULL COMMENT '作者ID（关联Users表）',
    title VARCHAR(200) NOT NULL COMMENT '标题',
    content TEXT NOT NULL COMMENT '正文',
    comment_count INT NOT NULL DEFAULT 0 COMMENT '评论数',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_discussion_topics_problem_updated(problem_id, updated_at),
    INDEX idx_discussion_topics_user(user_id),
    CONSTRAINT fk_discussion_topics_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- 讨论区：评论表
-- ----------------------------
CREATE TABLE IF NOT EXISTS `discussion_comments` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    topic_id BIGINT NOT NULL COMMENT '所属主题ID',
    user_id BIGINT NOT NULL COMMENT '评论者ID',
    parent_comment_id BIGINT NULL COMMENT '父评论ID（支持嵌套）',
    content TEXT NOT NULL COMMENT '评论内容',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_discussion_comments_topic_id(topic_id, id),
    INDEX idx_discussion_comments_user(user_id),
    CONSTRAINT fk_discussion_comments_topic
        FOREIGN KEY (topic_id) REFERENCES `discussion_topics`(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_discussion_comments_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id),
    CONSTRAINT fk_discussion_comments_parent
        FOREIGN KEY (parent_comment_id) REFERENCES `discussion_comments`(id)
        ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- 排行榜：提交记录表
-- ----------------------------
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

-- ----------------------------
-- 排行榜：全局排行榜物化视图
-- ----------------------------
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

-- ----------------------------
-- 排行榜：用户解题难度统计表
-- ----------------------------
CREATE TABLE IF NOT EXISTS `user_problem_stats` (
    user_id BIGINT PRIMARY KEY,
    solved_total INT NOT NULL DEFAULT 0,
    solved_easy INT NOT NULL DEFAULT 0,
    solved_medium INT NOT NULL DEFAULT 0,
    solved_hard INT NOT NULL DEFAULT 0,
    CONSTRAINT fk_user_problem_stats_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- 排行榜：比赛排行榜物化视图
-- ----------------------------
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

-- ----------------------------
-- 排行榜：用户已解决问题记录表
-- ----------------------------
CREATE TABLE IF NOT EXISTS `rank_solved_problems` (
    user_id BIGINT NOT NULL,
    problem_id BIGINT NOT NULL,
    difficulty TINYINT NOT NULL,
    first_accepted_at BIGINT NOT NULL,
    PRIMARY KEY (user_id, problem_id),
    CONSTRAINT fk_rank_solved_problems_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- ----------------------------
-- 排行榜：比赛期间解决的问题记录表
-- ----------------------------
CREATE TABLE IF NOT EXISTS `rank_contest_solved_problems` (
    contest_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    problem_id BIGINT NOT NULL,
    first_accepted_at BIGINT NOT NULL,
    PRIMARY KEY (contest_id, user_id, problem_id),
    CONSTRAINT fk_rank_contest_solved_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;