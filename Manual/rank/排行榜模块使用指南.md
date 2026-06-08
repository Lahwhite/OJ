# OJ 排行榜模块（rank）

排行榜模块位于 `OJ/rank`，提供全球排名、竞赛排名、用户题目完成统计，以及 LeetCode 风格可视化前端。

## 1. 能力概览

1. 总排行榜（分页、多关键字排序）
2. 比赛排行榜（分页）
3. 用户题目完成统计（Easy / Medium / Hard）
4. 全局统计摘要、用户百分位洞察、CSV 导出
5. 排行榜 JSON / HTTP 接口
6. Windows 联调运行包（含完整 DLL，pull 即用）

技术栈：

- 后端：C++17 + `common`（`oj::HttpServer`、`oj::Router`、`OJ_LOG_*`）
- 前端：`web/`（HTML + Tailwind CDN + ECharts）
- 存储：内存 demo 或 MySQL（libmariadbclient C API，`OJ_RANK_ENABLE_MYSQL=ON`）

对外入口：

- 前端页面：`GET /rank`
- REST API：`/api/leaderboard/...`

## 2. 目录结构

```text
rank/
├── include/          # 头文件
├── src/              # 服务、仓储、HTTP 处理
├── web/              # 前端页面
├── sql/              # 数据库脚本（唯一位置，运行包不再重复一份）
│   ├── setup_local_mysql.sql
│   ├── seed_users_for_rank.sql
│   ├── leaderboard_schema.sql
│   └── seed_demo.sql
├── tests/
├── runtime_package/  # Windows 运行包（exe + DLL + web + 启动脚本）
├── CMakeLists.txt
└── README.md         # 本文档
```

## 3. 快速启动

### 3.1 Windows 运行包（推荐，无需编译）

```powershell
git pull
cd OJ\rank\runtime_package
powershell -ExecutionPolicy Bypass -File .\start_rank.ps1
```

- 页面：http://127.0.0.1:8092/rank
- 健康检查：http://127.0.0.1:8092/health
- **窗口需保持打开**；成功时应持续输出 `http server started on port 8092`

可选 MySQL（不配则用内存 demo）：

```powershell
$env:OJ_MYSQL_HOST="127.0.0.1"
$env:OJ_MYSQL_PORT="3306"
$env:OJ_MYSQL_USER="User"
$env:OJ_MYSQL_PASSWORD="Management"
$env:OJ_MYSQL_DATABASE="myOJ"
powershell -ExecutionPolicy Bypass -File .\start_rank.ps1
```

### 3.2 开发编译

**默认（内存仓储）：**

```bash
cd OJ/rank
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -S . -B build-msys2
cmake --build build-msys2 --target leaderboard_server -j
./build-msys2/leaderboard_server.exe
```

**MySQL 持久化（libmariadbclient C API，纯 g++）：**

```bash
pacman -S --noconfirm mingw-w64-ucrt-x86_64-libmariadbclient

cd OJ/rank
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ \
  -DOJ_RANK_ENABLE_MYSQL=ON \
  -S . -B build-msys2
cmake --build build-msys2 --target leaderboard_server -j
```

服务启动后自动检测：MySQL 可用且 `leaderboard_global` 表存在 → `MysqlLeaderboardRepository`；否则回退内存 demo。

## 4. 数据库脚本（`rank/sql/`）

所有 SQL **只维护这一处**；`runtime_package` 不再附带副本。

| 文件 | 用途 | 执行顺序 |
|------|------|----------|
| `setup_local_mysql.sql` | 创建 `myOJ` 库与应用账号 `User`/`Management` | 1（root 执行） |
| `seed_users_for_rank.sql` | 插入演示用户 id 1–8（需已有 `Users` 表） | 2（可选） |
| `leaderboard_schema.sql` | 排行榜相关表结构 | 3 |
| `seed_demo.sql` | 演示排行榜数据（依赖 Users id 1–4） | 4（可选） |

示例（与 discussion / Users 共用 `myOJ` 库）：

```powershell
# root 初始化库与账号
mysql -u root -p < sql/setup_local_mysql.sql

# 应用账号建表与灌数
mysql -u User -pManagement myOJ < sql/leaderboard_schema.sql
mysql -u User -pManagement myOJ < sql/seed_demo.sql
```

若 `Users` 表为空，先执行 `seed_users_for_rank.sql`（或沿用 Users 模块已有数据）。

## 5. 同步运行包

修改源码或 `web/` 后，重新构建并同步：

```powershell
powershell -ExecutionPolicy Bypass -File .\rank\runtime_package\sync_runtime.ps1
```

脚本会复制 `leaderboard_server.exe`、`web/` 及 21 个运行时 DLL（含 libmariadb 传递依赖）。SQL 始终在 `../sql/`，无需同步。

运行包 DLL 列表：`libgcc_s_seh-1.dll`、`libstdc++-6.dll`、`libwinpthread-1.dll`、`libmariadb.dll`、`libcurl-4.dll`、`libssl-3-x64.dll`、`libcrypto-3-x64.dll` 等共 21 个。

## 6. 环境变量

| 变量 | 说明 | 默认 |
|------|------|------|
| `OJ_RANK_PORT` | HTTP 端口 | `8092` |
| `OJ_LOG_LEVEL` | 日志级别 | `info` |
| `OJ_LOG_FILE` | 日志文件（可选） | 空 |
| `OJ_MYSQL_HOST` | MySQL 主机 | `127.0.0.1` |
| `OJ_MYSQL_PORT` | MySQL 端口 | `3306` |
| `OJ_MYSQL_USER` | MySQL 用户 | 空 |
| `OJ_MYSQL_PASSWORD` | MySQL 密码 | 空 |
| `OJ_MYSQL_DATABASE` | 数据库名 | `myOJ` |

## 7. HTTP 接口

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/rank` | 排行榜页面 |
| GET | `/api/leaderboard/global` | 总榜 JSON（`limit`、`offset`） |
| GET | `/api/leaderboard/global/summary` | 全球统计摘要 |
| GET | `/api/leaderboard/global/export.csv` | 导出 CSV |
| GET | `/api/leaderboard/users/{id}/insight` | 用户百分位洞察 |
| GET | `/api/leaderboard/contest/{id}` | 比赛榜 JSON |
| GET | `/api/leaderboard/users/{id}/stats` | 用户题目完成统计 |
| GET | `/health` | 健康检查（含 `mysql_ready`、`rank_database_ready`） |

## 8. 排名规则

总榜与比赛榜排序：

1. `score` 降序
2. `solved_count` 降序
3. `penalty_seconds` 升序
4. `last_accepted_at` 升序

AC 得分：Easy +100、Medium +220、Hard +400。同一用户同一题目在总榜只计一次。

## 9. 与 Users 模块集成

Users 页面增加链接即可，无需编译 rank：

```html
<a th:href="@{http://127.0.0.1:8092/rank(return_url='http://127.0.0.1:8081/home')}">
    排行榜
</a>
```

## 10. 前端页面

访问 `/rank`：全球/竞赛 Tab、Top3 领奖台、ECharts 图表、CSV 导出。

可选 URL 参数：`return_url`、`contest_id`、`user_id`。

## 11. 构建与测试

```bash
cmake --build build-msys2 --target leaderboard_tests leaderboard_extended_tests leaderboard_mysql_tests -j
./build-msys2/leaderboard_tests.exe
./build-msys2/leaderboard_mysql_tests.exe
```

## 12. 常见问题

**页面 404**：确认服务在跑，访问 `http://127.0.0.1:8092/rank`。

**启动后窗口秒退**：多为缺 DLL 或端口占用。重新 `sync_runtime.ps1`；或设置 `OJ_RANK_PORT=8093`。

**MySQL 未生效**：检查 `/health` 中 `mysql_ready` 与 `rank_database_ready`；确认已执行 `sql/leaderboard_schema.sql` 且环境变量正确。

**运行包缺 exe**：先关闭正在运行的 `leaderboard_server.exe`，再执行 `sync_runtime.ps1`。
