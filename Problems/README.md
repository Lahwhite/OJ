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
http://localhost:8080/problems/
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
$env:OJ_DB_URL="jdbc:mysql://127.0.0.1:3306/myOJ?useSSL=false&allowPublicKeyRetrieval=true&serverTimezone=Asia/Shanghai&characterEncoding=utf8"
$env:OJ_DB_USERNAME="<your_mysql_username>"
$env:OJ_DB_PASSWORD="<your_mysql_password>"
```

默认账号适合部署环境统一创建授权；本地联调时建议只用环境变量覆盖，不要直接提交个人数据库账号或密码到配置文件。

常用环境变量：

- `OJ_PROBLEMS_ADDRESS`
- `OJ_PROBLEMS_PORT`
- `OJ_PROBLEMS_CONTEXT_PATH`
- `OJ_DB_URL`
- `OJ_DB_USERNAME`
- `OJ_DB_PASSWORD`
- `OJ_JWT_SECRET`
- `OJ_PROBLEM_STATUS_TOKEN`
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
http://localhost:8080/problems/
```

如果需要展示某个用户的 AC 状态，在页面 URL 中带上用户 ID：

```text
http://localhost:8080/problems/?user_id=1
```

前端会自动读取 `GET /problems/v1/problem-status/users/{userId}`，并在题目列表左侧显示 `✓` 或 `-`，详情页显示是否通过、历史最高分、最近得分和最近提交时间。

## 主要接口

公开接口：

- `GET /problems/v1/problems`
- `GET /problems/v1/problems/{id}`
- `GET /problems/v1/problems/{id}/test-cases`

管理员接口，需要请求头：

```text
Authorization: Bearer <JWT_TOKEN>
```

- `POST /problems/v1/problems`
- `PUT /problems/v1/problems/{id}`
- `DELETE /problems/v1/problems/{id}`

AC 状态接口：

- `GET /problems/v1/problem-status/users/{userId}`
- `GET /problems/v1/problem-status/users/{userId}/problems/{problemId}`

其他模块更新状态时调用：

```http
PUT /problems/v1/problem-status
X-Service-Token: <OJ_PROBLEM_STATUS_TOKEN>
Content-Type: application/json
```

```json
{
  "userId": 1,
  "problemId": 1001,
  "accepted": true,
  "score": 100,
  "maxScore": 100,
  "submittedAt": "2026-06-04T20:30:00"
}
```

字段说明：

- `userId`：用户 ID
- `problemId`：题目 ID
- `accepted`：本次提交是否通过
- `score`：本次得分
- `maxScore`：该题满分
- `submittedAt`：本次提交时间，ISO-8601 格式

更新规则：同一 `userId + problemId` 只保留一条状态；`accepted` 一旦为 true 不会被后续失败提交覆盖；`bestScore` 取历史最高分；`lastScore`、`maxScore` 和 `lastSubmittedAt` 记录最近一次提交。

其他模块只需要在评测或计分完成后调用该接口即可，Problems 模块只负责存储和展示 AC 状态，不参与代码评测。

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

- `problem_users`
- `problems`
- `problem_user_status`
- `test_cases`
- `tags`
- `problem_tags`

## 测试

测试环境使用 H2 内存数据库，不依赖本地 MySQL。

运行测试：

```powershell
mvn clean test
```

验证覆盖率：

```powershell
mvn test jacoco:check@check
```

当前 JaCoCo 覆盖率门槛：

- 行覆盖率不低于 90%
- 分支覆盖率不低于 60%

JaCoCo 报告生成在：

```text
target/site/jacoco/index.html
```

当前验证结果：

```text
Tests run: 42, Failures: 0, Errors: 0, Skipped: 0
LINE: 324 covered / 335 total = 96.72%
BRANCH: 59 covered / 84 total = 70.24%
All coverage checks have been met.
BUILD SUCCESS
```
