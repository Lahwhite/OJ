# Rank Runtime Package（Windows）

本目录是排行榜模块的 **Windows 联调运行包**，用法与 `discussion/runtime_package` 一致：无需本机编译即可启动服务（当前为内存 demo 数据）。

## 启动

在本目录运行：

```powershell
powershell -ExecutionPolicy Bypass -File .\start_rank.ps1
```

默认配置：

- 服务地址：`http://127.0.0.1:8092/rank`
- 健康检查：`http://127.0.0.1:8092/health`

修改端口：

```powershell
$env:OJ_RANK_PORT="8093"
powershell -ExecutionPolicy Bypass -File .\start_rank.ps1
```

## Users 跳转链接

Users 模块只需加链接，不需要编译 rank：

```html
<a th:href="@{http://127.0.0.1:8092/rank(return_url='http://127.0.0.1:8081/home')}">
    排行榜
</a>
```

可选参数：

- `return_url`：返回主页地址
- `contest_id`：打开后默认进入竞赛榜并加载该竞赛
- `user_id`：打开后默认选中该用户

## 数据库（可选）

当前运行包内置 **内存演示数据**，不依赖 MySQL。若后续接入持久化，可先执行：

```text
sql/leaderboard_schema.sql
```

## 同步运行包

修改 `rank` 源码或 `rank/web` 后，需要重新构建并同步到本目录；旧运行包不会自动包含新接口或新页面。

在仓库根目录或 `rank` 目录执行：

```powershell
powershell -ExecutionPolicy Bypass -File .\rank\runtime_package\sync_runtime.ps1
```

或手动：

```powershell
cmake --build rank\build-msys2 --target leaderboard_server -j
Copy-Item rank\build-msys2\leaderboard_server.exe rank\runtime_package\ -Force
Copy-Item rank\web\* rank\runtime_package\web\ -Force
```

## 说明

- 该运行包为 **Windows + MSYS2 UCRT64** 构建，附带 `libgcc` / `libstdc++` / `libwinpthread` 等运行时 DLL。
- Linux/macOS 不能直接运行 `.exe`，需在对应平台重新 `cmake` 构建。
