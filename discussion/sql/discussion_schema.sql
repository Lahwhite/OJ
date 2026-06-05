-- 讨论区复用主项目数据库，脚本可独立执行，也可由运行时建表逻辑兜底。
CREATE DATABASE IF NOT EXISTS `myOJ`
  DEFAULT CHARACTER SET utf8mb4
  DEFAULT COLLATE utf8mb4_unicode_ci;

USE `myOJ`;

-- 主题表只保存讨论区自己的业务字段，用户展示信息通过 Users/Profiles 动态关联。
CREATE TABLE IF NOT EXISTS `discussion_topics` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    -- problem_id 对应题目模块的题目编号，用于题目详情页过滤讨论。
    problem_id BIGINT NOT NULL,
    -- user_id 指向 Users 表，删除权限也按这个作者字段判断。
    user_id BIGINT NOT NULL,
    -- 标题长度与前端 maxlength 和服务层校验保持一致。
    title VARCHAR(200) NOT NULL,
    -- 正文使用 TEXT，具体长度上限由服务层控制为 5000 字节。
    content TEXT NOT NULL,
    -- 冗余评论数用于列表页快速展示，评论增删时由服务层事务维护。
    comment_count INT NOT NULL DEFAULT 0,
    -- updated_at 会在主题被评论时主动刷新，使活跃主题靠前。
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    -- 按题目过滤后再按更新时间展示，是讨论区最常见的查询路径。
    INDEX idx_discussion_topics_problem_updated(problem_id, updated_at),
    -- 作者索引用于后续扩展“我的讨论”等用户维度查询。
    INDEX idx_discussion_topics_user(user_id),
    CONSTRAINT fk_discussion_topics_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

-- 评论表同时承载顶层评论和回复，parent_comment_id 为空表示顶层评论。
CREATE TABLE IF NOT EXISTS `discussion_comments` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    -- 评论必须属于一个主题，删除主题时评论自动清理。
    topic_id BIGINT NOT NULL,
    -- 评论作者同样引用 Users 表，删除权限按作者判断。
    user_id BIGINT NOT NULL,
    -- 自引用父评论用于展示回复关系，服务层会校验父评论属于同一主题。
    parent_comment_id BIGINT NULL,
    -- 评论内容长度由服务层限制为 3000 字节，数据库层保留更宽松类型。
    content TEXT NOT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    -- 按主题取评论并按 id 升序渲染，是评论列表的主要查询路径。
    INDEX idx_discussion_comments_topic_id(topic_id, id),
    -- 用户索引用于权限检查和后续个人评论查询。
    INDEX idx_discussion_comments_user(user_id),
    CONSTRAINT fk_discussion_comments_topic
        FOREIGN KEY (topic_id) REFERENCES `discussion_topics`(id)
        ON DELETE CASCADE,
    CONSTRAINT fk_discussion_comments_user
        FOREIGN KEY (user_id) REFERENCES `Users`(id),
    -- 删除父评论时级联删除子回复，服务层会同步扣减 comment_count。
    CONSTRAINT fk_discussion_comments_parent
        FOREIGN KEY (parent_comment_id) REFERENCES `discussion_comments`(id)
        ON DELETE CASCADE
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
