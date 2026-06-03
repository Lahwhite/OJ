# 检索增强生成模块使用手册

## 1. 本模块旨在实现

检索增强生成功能

## 2. 使用前环境需求

### 需要先本地部署离线版 OI wiki

```
git clone https://gitee.com/OI-wiki/OI-wiki.git -b gh-pages
```

进入 OI-wiki 目录下，启动HTTP服务器
```
python3 -m http.server 8000
```

可通过如下方式访问 OI Wiki 页面
```
http://localhost:8000/
```

build_index.py 用于构建索引
searcher.py 用于搜索相关页面
agent.py 用于调用本地大模型回答问题