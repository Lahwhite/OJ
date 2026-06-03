# OJ 排行榜模块（C++ / Redis / MySQL）

本模块实现以下需求：

1. 总排行榜实现  
2. 题目完成情况统计  
3. 比赛排行榜（可选，已实现）  
4. 排名算法优化（多关键字 + 难度加权）  
5. 排行榜数据接口（JSON）

## 目录

- `include/leaderboard/`：领域模型与服务接口
- `include/api/`：接口封装
- `include/storage/`：存储层实现
- `src/leaderboard/`：排行榜服务实现
- `src/api/`：JSON 数据接口实现
- `src/storage/`：缓存/仓储实现（内存版 + MySQL/Redis 占位）
- `sql/leaderboard_schema.sql`：MySQL 表结构
- `tests/`：核心逻辑测试

## 排名规则（优化后）

总榜与比赛榜都按以下顺序排序：

1. `score` 降序（主排序）  
2. `solved_count` 降序  
3. `penalty_seconds` 升序（罚时越低越好）  
4. `last_accepted_at` 升序（更早达到者在前）

得分增量（AC 时）：

- Easy: `+100`
- Medium: `+220`
- Hard: `+400`

## 接口说明（服务层）

- `get_global_leaderboard(limit, offset)`：获取总榜分页
- `get_problem_completion_stats(user_id)`：获取用户题目完成统计
- `get_contest_leaderboard(contest_id, limit, offset)`：获取比赛榜分页
- `on_submission(event)`：提交事件增量更新（含缓存失效）

## 数据接口（JSON）

- `get_global_leaderboard_json(limit, offset)`
- `get_problem_stats_json(user_id)`
- `get_contest_leaderboard_json(contest_id, limit, offset)`

这三个函数位于 `LeaderboardApi`，可直接被你的 HTTP Controller 调用。

## Redis / MySQL 接入建议

- Redis  
  - `global:zset`：总榜
  - `contest:{id}:zset`：比赛榜
  - `user:{id}:problem_stats`：题目统计
- MySQL  
  - 以 `submissions` 为事实表
  - `leaderboard_global` / `leaderboard_contest` / `user_problem_stats` 为聚合表
  - 通过提交事件异步或同步更新聚合表

## 前端页面（LeetCode 风格）

目录：`web/`

- 全球排名 / 竞赛排名切换
- 前三名领奖台卡片
- ECharts 高级柱状图：积分渐变横条、难度堆叠柱、通过数+罚时双轴组合图
- 点击表格行联动难度分布与用户统计

启动带前端的 HTTP 服务：

```bash
cmake -S . -B build
cmake --build build --target leaderboard_server
./build/leaderboard_server
```

### MSYS2（UCRT64）构建

在 **MSYS2 UCRT64** 终端中（或把 `C:\msys64\ucrt64\bin` 加入 PATH）：

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja

cd /f/软工实验（）新/OJ/rank   # 按你的实际路径调整
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -S . -B build-msys2
cmake --build build-msys2 --target leaderboard_server -j
./build-msys2/leaderboard_server.exe
```

浏览器访问：`http://127.0.0.1:8092/rank`（端口可通过环境变量 `OJ_RANK_PORT` 修改）

### Windows 运行包（与 discussion 一致）

目录：`runtime_package/`

团队同学本地联调可直接运行，无需自行编译：

```powershell
cd OJ\rank\runtime_package
powershell -ExecutionPolicy Bypass -File .\start_rank.ps1
```

更新源码或前端后同步运行包：

```powershell
powershell -ExecutionPolicy Bypass -File .\rank\runtime_package\sync_runtime.ps1
```

说明见 `runtime_package/README.md`。

### HTTP 接口

| 方法 | 路径 | 说明 |
|------|------|------|
| GET | `/rank` | 排行榜页面 |
| GET | `/api/leaderboard/global?limit=&offset=` | 总榜 JSON |
| GET | `/api/leaderboard/global/summary` | 全球统计摘要 |
| GET | `/api/leaderboard/global/export.csv` | 导出 CSV |
| GET | `/api/leaderboard/users/:id/insight` | 用户百分位洞察 |
| GET | `/api/leaderboard/contest/:id?limit=&offset=` | 比赛榜 JSON |
| GET | `/api/leaderboard/users/:id/stats` | 用户题目完成统计 |
| GET | `/health` | 健康检查 |

## 构建与测试

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```
