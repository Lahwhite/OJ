USE `myOJ`;

INSERT INTO Users (id, username, password) VALUES
(1,'linzhe','pwd'),(2,'chenxi','pwd'),(3,'hanmiao','pwd'),(4,'zeyuan','pwd'),
(5,'yufei','pwd'),(6,'qingyang','pwd'),(7,'muxin','pwd'),(8,'tingyu','pwd')
ON DUPLICATE KEY UPDATE username = VALUES(username);
