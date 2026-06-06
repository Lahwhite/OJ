# OJ 排行榜模块（rank）

完整使用说明见：**[排行榜模块使用文档](../模块使用文档/排行榜模块使用文档.md)**

## 快速启动

**开发编译（默认内存仓储）：**

```bash
cd OJ/rank
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ -S . -B build-msys2
cmake --build build-msys2 --target leaderboard_server -j
./build-msys2/leaderboard_server.exe
```

**MySQL 持久化编译（使用 libmariadbclient C API，纯 g++，无需 Connector/C++）：**

```bash
# 先安装依赖（只需一次）
pacman -S --noconfirm mingw-w64-ucrt-x86_64-libmariadbclient

# 编译
cd OJ/rank
cmake -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++ \
  -DOJ_RANK_ENABLE_MYSQL=ON \
  -S . -B build-msys2
cmake --build build-msys2 --target leaderboard_server -j
```

启动前配置（与 discussion / Users 共用 `myOJ` 库）：

```powershell
$env:OJ_MYSQL_HOST="127.0.0.1"
$env:OJ_MYSQL_PORT="3306"
$env:OJ_MYSQL_USER="User"
$env:OJ_MYSQL_PASSWORD="Management"
$env:OJ_MYSQL_DATABASE="myOJ"
```

初始化表结构：

```text
mysql -u User -p myOJ < sql/leaderboard_schema.sql
```

可选导入演示数据（需 `Users` 表已有 id 1-8）：

```text
mysql -u User -p myOJ < sql/seed_demo.sql
```

服务启动后会自动检测：MySQL 可用且表存在 → 使用 `MysqlLeaderboardRepository`；否则回退内存 demo。

**Windows 运行包：**

```powershell
cd OJ\rank\runtime_package
powershell -ExecutionPolicy Bypass -File .\start_rank.ps1
```

浏览器访问：`http://127.0.0.1:8092/rank`
