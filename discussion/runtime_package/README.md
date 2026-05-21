# Discussion Runtime Package

## 启动

在本目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\start_discussion.ps1
```

默认配置：

- 服务地址：`http://127.0.0.1:8079/discussion`
- MySQL：`127.0.0.1:3306`
- 数据库：`myOJ`
- 用户名：`User`
- 密码：`Management`

如果要改配置，可以先设置环境变量再启动：

```powershell
$env:OJ_MYSQL_HOST="127.0.0.1"
$env:OJ_MYSQL_PORT="3306"
$env:OJ_MYSQL_USER="User"
$env:OJ_MYSQL_PASSWORD="Management"
$env:OJ_MYSQL_DATABASE="myOJ"
$env:OJ_DISCUSSION_PORT="8079"
powershell -ExecutionPolicy Bypass -File .\start_discussion.ps1
```

## Users 跳转链接

Users 模块只需要加链接，不需要编译 discussion：

```html
<a th:href="@{http://127.0.0.1:8079/discussion(username=${session.loginUser}, return_url='http://127.0.0.1:8081/home')}">
    讨论区
</a>
```

如果从题目详情页进入，可以额外传 `problem_id`：

```html
<a th:href="@{http://127.0.0.1:8079/discussion(username=${session.loginUser}, problem_id=${problem.id}, return_url='http://127.0.0.1:8081/home')}">
    进入题目讨论
</a>
```

## 数据库

如果数据库里还没有讨论区表，先执行

```text
sql/discussion_schema.sql
```

讨论区接口会使用 Users 模块已有的 `Users` 和 `Profiles` 表来读取用户名、昵称和头像。
