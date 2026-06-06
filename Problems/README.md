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

## 与评测模块（judge_engine）对接

题目模块的数据结构已按 `judge_engine.exe` CLI 所需格式对齐。Problems 模块**不直接调用**评测引擎，由提交/评测服务负责拼命令、执行 exe、解析结果；本模块提供题目与测试用例数据，并在评测完成后接收 AC 状态回写。

更完整的对接说明见：[题目模块与评测模块对接说明](../模块使用文档/题目模块与评测模块对接说明.md)。

### 命令格式

```text
judge_engine.exe --program_language=<语言> --src_file=<源码路径> --expect_result=<期望结果JSON路径>
```

| CLI 参数 | 来源 | 说明 |
|---|---|---|
| `--program_language` | 用户选择的语言 | 支持 `c` / `cpp` / `java` / `python`，须与 `judge/config/languages.json` 的 `id` 一致 |
| `--src_file` | 用户提交的源码 | 写入临时文件后传入路径 |
| `--expect_result` | 由题目数据组装 | 见下方 JSON 格式 |

### 字段映射（Problems API → expect_result）

| expect_result 字段 | Problems API 字段 | 接口 |
|---|---|---|
| `time_limit_ms` | `timeLimit` | `GET /problems/v1/problems/{id}` |
| `memory_limit_mb` | `memoryLimit` | `GET /problems/v1/problems/{id}` |
| `test_cases[].id` | `testCases[].id` | `GET /problems/v1/problems/{id}/test-cases` |
| `test_cases[].input` | `testCases[].input` | 同上 |
| `test_cases[].expected_output` | `testCases[].output` | 同上（引擎也接受 `output` 字段名） |
| `test_cases[].score` | `testCases[].score` | 同上 |

### expect_result 示例

从接口数据组装后的 JSON 示例：

```json
{
  "time_limit_ms": 1000,
  "memory_limit_mb": 128,
  "test_cases": [
    {
      "id": 1,
      "input": "1 2\n",
      "expected_output": "3\n",
      "score": 50
    },
    {
      "id": 2,
      "input": "10 20\n",
      "expected_output": "30\n",
      "score": 50
    }
  ]
}
```

### 推荐接入流程

```text
用户提交代码
    ↓
GET /problems/v1/problems/{id}           → 取 timeLimit、memoryLimit
GET /problems/v1/problems/{id}/test-cases → 取完整测试用例
    ↓
组装 expect_result.json，写入用户源码为 src 文件
    ↓
执行 judge_engine.exe --program_language=... --src_file=... --expect_result=...
    ↓
读取 judge_engine 返回 JSON（verdict、total_score、max_score 等）
    ↓
PUT /problems/v1/problem-status           → 回写 AC 状态
```

### 注意事项

- 详情页语言选择器含 `javascript`，但 judge_engine 目前不支持，接入时需校验或过滤。
- 正式评测应使用 `/test-cases` 获取完整用例，不要依赖公开详情接口中的部分用例。
- judge_engine CLI 用法与返回 JSON 结构详见 `judge/使用文档.md`。
- 详情页「运行代码」「提交答案」按钮已预留 UI，评测逻辑由评测模块接入后调用上述流程。

提交评测接口：

- `POST /problems/v1/problems/{id}/submit`

```json
{
  "username": "admin",
  "language": "java",
  "code": "public class Main { ... }"
}
```

后端会在 `judge/` 目录组装 `cases.json` 和源码文件，并执行：

```text
judge_engine.exe --program_language=... --src_file=... --expect_result=... --username=...
```

配置项（`application.yml`）：

- `judge.work-dir`：默认 `../judge`
- `judge.executable`：默认 `judge_engine.exe`
- `judge.timeout-seconds`：默认 `120`

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
