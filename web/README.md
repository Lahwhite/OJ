# OJ 评测结果页（源码）

编译 `judge_engine` 时嵌入到 exe，交付时无需单独携带本目录。

## 目录

```text
web/
├── index.html
├── css/style.css
├── js/app.js
└── config.example.json   # 配置示例（实际配置在 judge/config/judge_cli.json）
```

## 开发

修改页面后重新编译 judge：

```powershell
cd ../judge
cmake -S . -B build -G "MinGW Makefiles" -DCMAKE_CXX_COMPILER=g++
cmake --build build --target judge_engine
```

## 文档

- 交付使用：`../judge/交付使用手册.md`
- 改动说明：`../judge/改动文档.md`
