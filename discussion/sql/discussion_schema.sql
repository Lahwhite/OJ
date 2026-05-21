CREATE DATABASE IF NOT EXISTS `myOJ`
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE `myOJ`;

CREATE TABLE IF NOT EXISTS `discussion_topics` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    problem_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    title VARCHAR(200) NOT NULL,
    content TEXT NOT NULL,
    comment_count INT NOT NULL DEFAULT 0,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    INDEX idx_discussion_topics_problem_updated(problem_id, updated_at),
    INDEX idx_discussion_topics_user(user_id),
    CONSTRAINT fk_discussion_topics_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `discussion_comments` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    topic_id BIGINT NOT NULL,
    user_id BIGINT NOT NULL,
    parent_comment_id BIGINT NULL,
    content TEXT NOT NULL,
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
