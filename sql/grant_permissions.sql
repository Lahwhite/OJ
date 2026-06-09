-- 创建用户 User（本地与远程）
CREATE USER IF NOT EXISTS 'User'@'localhost' IDENTIFIED BY 'Management';
CREATE USER IF NOT EXISTS 'User'@'%' IDENTIFIED BY 'Management';

-- 创建用户 Problems（本地与远程）
CREATE USER IF NOT EXISTS 'Problems'@'localhost' IDENTIFIED BY 'Management';
CREATE USER IF NOT EXISTS 'Problems'@'%' IDENTIFIED BY 'Management';

-- 授予 User 全部权限
GRANT ALL ON myOJ.* TO 'User'@'localhost';
GRANT ALL ON myOJ.* TO 'User'@'%';

-- 授予 Problems 全部权限
GRANT ALL ON myOJ.* TO 'Problems'@'localhost';
GRANT ALL ON myOJ.* TO 'Problems'@'%';

FLUSH PRIVILEGES;