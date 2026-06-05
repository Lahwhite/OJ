# 动态规划接口测试：爬楼梯

## 题意

给定 `n`（1 ≤ n ≤ 45），每次可爬 1 或 2 阶，求到达第 `n` 阶的不同方法数（经典 DP：`f(n)=f(n-1)+f(n-2)`）。

## 文件

| 语言 | 源码 | 期望结果 |
|------|------|----------|
| C | `testcode/dp_stairs.c` | `expect/dp_stairs.json` |
| C++ | `testcode/dp_stairs.cpp` | 同上 |
| Java | `testcode/dp_stairs.java` | 同上 |
| Python | `testcode/dp_stairs.py` | 同上 |

共 7 组用例，满分 80 分（最后一组 n=30 权重 20）。

## 运行示例

在项目根目录 `judge` 下执行（避免找不到 `config/languages.json`）：

```powershell
$exe = ".\build\judge_engine.exe"
$expect = "tests\interface\dp\expect\dp_stairs.json"

.\build\judge_engine.exe --program_language=c     --src_file=tests\interface\dp\testcode\dp_stairs.c   --expect_result=$expect
.\build\judge_engine.exe --program_language=cpp   --src_file=tests\interface\dp\testcode\dp_stairs.cpp --expect_result=$expect
.\build\judge_engine.exe --program_language=java  --src_file=tests\interface\dp\testcode\dp_stairs.java --expect_result=$expect
.\build\judge_engine.exe --program_language=python --src_file=tests\interface\dp\testcode\dp_stairs.py --expect_result=$expect
```

期望 `verdict` 均为 `AC`，`total_score` 为 `80`。
