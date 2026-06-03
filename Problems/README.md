# 题目管理模块

## 功能概览

- 题目分页查询
- 题目详情查询
- 管理员创建题目
- 管理员更新题目
- 管理员删除题目
- 标签维护
- 测试用例维护
- JWT 鉴权
- 统一响应体
- 统一异常处理
- 单元测试与覆盖率配置

## 技术栈

- Java 8
- Spring Boot 2.7.18
- Spring Web
- Spring Validation
- Spring Data JPA
- MySQL
- H2（测试环境）
- JUnit 5
- Mockito
- MockMvc
- JaCoCo

## 项目结构

```text
management
├── pom.xml
├── README.md
├── src
│   ├── main
│   │   ├── java/com/oj/problem
│   │   └── resources
│   │       ├── application.yml
│   │       ├── schema.sql
│   │       └── init-data.sql
│   └── test
│       ├── java/com/oj/problem
│       └── resources/application-test.yml
└── target
```

## 主要接口

### 公开接口

- `GET /api/v1/problems`
- `GET /api/v1/problems/{id}`
- `GET /api/v1/problems/{id}/test-cases`

### 管理员接口

- `POST /api/v1/problems`
- `PUT /api/v1/problems/{id}`
- `DELETE /api/v1/problems/{id}`

## 环境要求

- JDK 8 及以上
- Maven 3.8 及以上
- MySQL 8.0（运行主程序时）
- IntelliJ IDEA（推荐）

## 配置说明

### 运行配置

运行配置文件：
[application.yml](E:/OJ/OJ/management/src/main/resources/application.yml)

当前已去除数据库和密钥相关硬编码，优先通过环境变量注入：

- `OJ_MANAGEMENT_PORT`
- `OJ_MANAGEMENT_CONTEXT_PATH`
- `OJ_DB_URL`
- `OJ_DB_USERNAME`
- `OJ_DB_PASSWORD`
- `OJ_JWT_SECRET`
- `OJ_JPA_DDL_AUTO`
- `OJ_SHOW_SQL`

示例：

```yaml
server:
  port: ${OJ_MANAGEMENT_PORT:8080}
  servlet:
    context-path: ${OJ_MANAGEMENT_CONTEXT_PATH:/api}

spring:
  datasource:
    url: ${OJ_DB_URL:jdbc:mysql://localhost:3306/oj?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8}
    username: ${OJ_DB_USERNAME:root}
    password: ${OJ_DB_PASSWORD:}
    driver-class-name: com.mysql.cj.jdbc.Driver

security:
  jwt:
    secret: ${OJ_JWT_SECRET:oj-management-dev-secret}
```

### 测试配置

测试配置文件：
[application-test.yml](E:/OJ/OJ/management/src/test/resources/application-test.yml)

测试默认使用 H2 内存数据库，不依赖真实 MySQL。

## 数据库初始化

建表脚本：
[schema.sql](E:/OJ/OJ/management/src/main/resources/schema.sql)

测试数据脚本：
[init-data.sql](E:/OJ/OJ/management/src/main/resources/init-data.sql)

当前核心表包括：

- `users`
- `problems`
- `test_cases`
- `tags`
- `problem_tags`

## 数据库使用方法

### 1. 创建数据库

先登录 MySQL：

```bash
mysql -u root -p
```

执行：

```sql
CREATE DATABASE IF NOT EXISTS oj CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
USE oj;
```

### 2. 导入表结构

在 `management` 目录下执行：

```bash
mysql -u root -p oj < src/main/resources/schema.sql
```

### 3. 导入测试数据

继续执行：

```bash
mysql -u root -p oj < src/main/resources/init-data.sql
```

这会导入一条公开题目 `Hello Problem`，可直接用于浏览器接口验证。

### 4. 配置数据库连接

如果你的 MySQL 密码不是空，启动前先设置环境变量，例如在 PowerShell 中：

```powershell
$env:OJ_DB_USERNAME='root'
$env:OJ_DB_PASSWORD='你的MySQL密码'
$env:OJ_DB_URL='jdbc:mysql://localhost:3306/oj?useSSL=false&serverTimezone=Asia/Shanghai&characterEncoding=utf8'
```

如需修改 JWT 密钥，也可以设置：

```powershell
$env:OJ_JWT_SECRET='your-own-secret'
```

## 如何导入 IDEA

1. 打开 IDEA
2. 选择 `Open`
3. 打开项目目录 `E:\OJ\OJ\management`
4. 如果没有自动识别为 Maven 项目，右键 [pom.xml](E:/OJ/OJ/management/pom.xml)
5. 选择 `Add as Maven Project`
6. 配置 `Project SDK` 为 JDK 8 或更高版本

## 如何运行项目

### 方式一：Maven

在项目根目录执行：

```bash
mvn spring-boot:run
```

### 方式二：IDEA

直接运行：
[ProblemManagementApplication.java](E:/OJ/OJ/management/src/main/java/com/oj/problem/ProblemManagementApplication.java)

启动后访问：

```text
http://localhost:8080/api/v1/problems
```

如果已经导入了 `init-data.sql`，还可以直接访问：

```text
http://localhost:8080/api/v1/problems/1
```

获取某个题目的全部测试用例：
```text
http://localhost:8080/api/v1/problems/1/test-cases
```

## 如何运行测试

### 在 IDEA 中运行

右键 `src/test/java` 目录，选择：

- `Run 'All Tests'`

### 使用 Maven 运行

```bash
mvn clean test
```

## 如何查看覆盖率

推荐使用 Maven + JaCoCo：

```bash
mvn clean verify
```

覆盖率报告生成位置：
[JaCoCo 报告](E:/OJ/OJ/management/target/site/jacoco/index.html)

说明：

- 项目已配置 JaCoCo
- 行覆盖率门槛为 `80%`
- 若未达到 `80%`，`verify` 会失败

## 已编写测试

主要测试文件：

- [ProblemServiceImplTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/service/impl/ProblemServiceImplTest.java)
- [ProblemControllerTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/controller/ProblemControllerTest.java)
- [JwtTokenServiceTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/security/JwtTokenServiceTest.java)
- [GlobalExceptionHandlerTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/exception/GlobalExceptionHandlerTest.java)
- [ApiResponseTest.java](E:/OJ/OJ/management/src/test/java/com/oj/problem/common/ApiResponseTest.java)

测试用例接口已覆盖：
- `ProblemControllerTest#getProblemTestCasesShouldReturnAllTestCases`
- `ProblemServiceImplTest#getProblemTestCasesShouldReturnPublicProblemTestCases`
- `ProblemServiceImplTest#getProblemTestCasesShouldRejectPrivateProblem`

当前已通过验证：
```bash
mvn test
```

验证结果：
```text
Tests run: 30, Failures: 0, Errors: 0, Skipped: 0
BUILD SUCCESS
```

## 认证说明

管理员接口需要在请求头中携带 JWT：

```text
Authorization: Bearer <JWT_TOKEN>
```

JWT payload 至少包含：

```json
{
  "user_id": 1,
  "role": "admin"
}
```

## 注意事项

1. 可通过 `GET /api/v1/problems/{id}/test-cases` 获取某个公开题目的全部测试用例
2. 运行主程序时需要真实 MySQL
3. 跑测试时不需要真实 MySQL
4. 如果 IDEA 自带 Coverage 有问题，优先使用 Maven 的 JaCoCo 报告
5. 如果 `mvn clean verify` 报 JRE/JDK 错误，需要检查 `JAVA_HOME` 和 Maven 使用的 JDK

## 当前实现边界

本模块当前只实现题目管理功能，适合作为课程实验或 OJ 后端子模块示例。

后续如果需要继续扩展，可以在此基础上补充：

- 用户模块
- 提交评测模块
- 排行榜模块
- 讨论区模块
- Swagger / OpenAPI 文档
- 集成测试
