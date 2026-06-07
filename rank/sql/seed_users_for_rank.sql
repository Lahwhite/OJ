USE `myOJ`;

INSERT INTO Users (id, username, password) VALUES
(1,'alice','pwd'),(2,'bob','pwd'),(3,'carol','pwd'),(4,'david','pwd'),
(5,'eve','pwd'),(6,'frank','pwd'),(7,'grace','pwd'),(8,'henry','pwd')
ON DUPLICATE KEY UPDATE username = VALUES(username);
