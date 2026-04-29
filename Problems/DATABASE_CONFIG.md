# Problems 数据库配置

## 服务监听

- 地址：`0.0.0.0`
- 端口：`8080`
- 访问前缀：`/api`

局域网访问示例：

```text
http://<服务器局域网IP>:8080/api/v1/problems
```

## MySQL 默认配置

- Host：`localhost`
- Port：`3306`
- Database：`myOJ`
- Username：`Problems`
- Password：`Management`

默认 JDBC URL：

```text
jdbc:mysql://localhost:3306/myOJ?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8
```

## 环境变量覆盖

运行时可以用环境变量覆盖默认值：

```powershell
$env:OJ_PROBLEMS_ADDRESS='0.0.0.0'
$env:OJ_PROBLEMS_PORT='8080'
$env:OJ_DB_URL='jdbc:mysql://localhost:3306/myOJ?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8'
$env:OJ_DB_USERNAME='Problems'
$env:OJ_DB_PASSWORD='Management'
```

兼容旧的 `OJ_MANAGEMENT_ADDRESS`、`OJ_MANAGEMENT_PORT`、`OJ_MANAGEMENT_CONTEXT_PATH` 环境变量，但新配置建议使用 `OJ_PROBLEMS_*`。

启动：

```powershell
mvn spring-boot:run
```
