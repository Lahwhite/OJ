# Problems Module

题目管理模块，负责题目列表、题目详情、测试用例、标签和管理员维护题库等功能。

## 功能

- 分页查询题目，支持关键字、难度、标签筛选
- 查看题目详情和测试用例
- 管理员创建、编辑、删除题目
- 维护题目标签和测试用例
- 记录提交次数、通过次数和通过率
- JWT 管理员鉴权
- 统一响应和异常处理
- 提供静态前端页面

## 技术栈

- Java 8+
- Spring Boot 2.7.18
- Spring Web
- Spring Data JPA
- MySQL
- H2, JUnit 5, Mockito, MockMvc
- JaCoCo

## 配置

默认服务地址：

```text
http://localhost:8080/api/
```

默认数据库配置在 `src/main/resources/application.yml`：

```yaml
spring:
  datasource:
    url: ${OJ_DB_URL:jdbc:mysql://localhost:3306/myOJ?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8}
    username: ${OJ_DB_USERNAME:Problems}
    password: ${OJ_DB_PASSWORD:Management}
```

可以通过环境变量覆盖默认值，例如 PowerShell：

```powershell
$env:OJ_DB_USERNAME="root"
$env:OJ_DB_PASSWORD="123456"
```

常用环境变量：

- `OJ_PROBLEMS_ADDRESS`
- `OJ_PROBLEMS_PORT`
- `OJ_PROBLEMS_CONTEXT_PATH`
- `OJ_DB_URL`
- `OJ_DB_USERNAME`
- `OJ_DB_PASSWORD`
- `OJ_JWT_SECRET`
- `OJ_JPA_DDL_AUTO`
- `OJ_SHOW_SQL`

## 启动

进入模块目录：

```powershell
cd Problems
mvn clean spring-boot:run
```

启动后访问：

```text
http://localhost:8080/api/
```

## 主要接口

公开接口：

- `GET /api/v1/problems`
- `GET /api/v1/problems/{id}`
- `GET /api/v1/problems/{id}/test-cases`

管理员接口，需要请求头：

```text
Authorization: Bearer <JWT_TOKEN>
```

- `POST /api/v1/problems`
- `PUT /api/v1/problems/{id}`
- `DELETE /api/v1/problems/{id}`

## 数据库初始化

建表脚本：

```text
src/main/resources/schema.sql
```

初始化数据：

```text
src/main/resources/init-data.sql
```

默认核心表：

- `users`
- `problems`
- `test_cases`
- `tags`
- `problem_tags`

## 测试

测试环境使用 H2 内存数据库，不依赖本地 MySQL。

运行测试：

```powershell
mvn clean test
```

查看覆盖率：

```powershell
mvn clean verify
```

JaCoCo 报告生成在：

```text
target/site/jacoco/index.html
```

当前验证结果：

```text
Tests run: 35, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS
```
