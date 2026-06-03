CREATE DATABASE IF NOT EXISTS `myOJ`;
USE`myOJ`;

CREATE TABLE IF NOT EXISTS `Users` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(32) NOT NULL UNIQUE,
    password VARCHAR(60) NOT NULL
);

CREATE TABLE IF NOT EXISTS `Profiles` (
    id BIGINT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(32) NOT NULL UNIQUE,
    nickname VARCHAR(64)  CHARACTER SET utf8mb4,
    signature VARCHAR(255)  CHARACTER SET utf8mb4,
    avatar VARCHAR(255)
);

INSERT INTO `Users` (username, password)
VALUES ('Eri Ninamori', 'Aomi Isara');
INSERT INTO `Profiles` (username, nickname, signature)
VALUES ('Eri Ninamori', '蜷守惠理', '我在海上耕耘，风中播种🌧');
INSERT INTO `Users` (username, password)
VALUES ('haha', 'zhu123456');

select * from `myOJ`.`Users`;
select * from `myOJ`.`Profiles`;