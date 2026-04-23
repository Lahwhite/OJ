# 独立排行榜模块（不侵入主工程）

这个目录是单独实现的排行榜模块，不依赖你当前仓库已有的排行榜代码，适合团队协作里“自己模块自己交付”。

## 功能

- 总排行榜（支持分页）
- 比赛排行榜（支持分页）
- 用户题目完成统计
- 提交事件增量更新
- 排名规则：`score desc` -> `solved_count desc` -> `penalty asc` -> `last_accepted_at asc`

## 规则细节

- 只有 `accepted=true` 才增加分数
- 分数权重：
  - Easy `+100`
  - Medium `+220`
  - Hard `+400`
- 同一用户同一题在总榜只算一次 solved
- 同一用户同一比赛同一题在比赛榜只算一次 solved

## 构建与测试

在 `leaderboard_isolated` 目录执行：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

## 对接建议（最小侵入）

- 你们主工程只需要把这个目录当成独立静态库引入
- 在 Controller 里调用：
  - `get_global_json(limit, offset)`
  - `get_contest_json(contest_id, limit, offset)`
  - `get_problem_stats_json(user_id)`
- 提交判题结果时调用 `apply_submission(event)` 完成增量更新
