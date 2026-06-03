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
- Gemini 模型：`gemini-2.5-flash`

AI 总结功能需要先设置 Gemini API Key；没有设置时讨论区仍可正常使用，但“AI 总结”接口会返回 `gemini_not_configured`。

如果要改配置，可以先设置环境变量再启动：

```powershell
$env:OJ_MYSQL_HOST="127.0.0.1"
$env:OJ_MYSQL_PORT="3306"
$env:OJ_MYSQL_USER="User"
$env:OJ_MYSQL_PASSWORD="Management"
$env:OJ_MYSQL_DATABASE="myOJ"
$env:OJ_DISCUSSION_PORT="8079"
$env:OJ_GEMINI_API_KEY="你的 Gemini API Key"
$env:OJ_GEMINI_MODEL="gemini-2.5-flash"
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

## AI 总结测试

启动服务并发布至少一个主题后，可以在页面右侧主题详情中点击“生成总结”，也可以用接口测试：

```powershell
Invoke-RestMethod -Method Post `
  -Uri "http://127.0.0.1:8079/api/discussions/topics/1/summary" `
  -ContentType "application/json" `
  -Body "{}"
```

如果更新了源码或前端文件，需要重新构建 `discussion_server.exe` 并同步 `web/` 到本运行包；直接使用旧运行包不会包含新接口。

## 删除评论测试

页面中只有当前 `username` 与评论作者一致时才显示“删除”按钮。接口测试示例：

```powershell
Invoke-RestMethod -Method Delete `
  -Uri "http://127.0.0.1:8079/api/discussions/topics/1/comments/2" `
  -ContentType "application/json" `
  -Body (@{ username = "haha" } | ConvertTo-Json)
```

如果删除的是父评论，它下面的回复会一起删除，并同步更新主题评论数。

## 删除主题测试

页面中只有当前 `username` 与主题作者一致时才显示“删除主题”按钮。接口测试示例：

```powershell
Invoke-RestMethod -Method Delete `
  -Uri "http://127.0.0.1:8079/api/discussions/topics/1" `
  -ContentType "application/json" `
  -Body (@{ username = "haha" } | ConvertTo-Json)
```

删除主题后，该主题下的评论和回复会一起删除。如果你用 `?username=test` 打开作者为 `haha` 的主题，按钮不会显示，直接调接口也会返回权限错误。

