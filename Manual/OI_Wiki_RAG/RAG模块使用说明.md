# 检索增强生成模块使用手册

## 1. 本模块旨在实现检索增强生成功能

## 2. 使用前环境准备

### 需要先本地部署离线版 OI Wiki

```
git clone https://gitee.com/OI-wiki/OI-wiki.git -b gh-pages
```

进入 OI-wiki 目录，启动 HTTP 服务器
```
python3 -m http.server 8000
```

可通过以下地址访问 OI Wiki 页面
```
http://localhost:8000/
```

build_index.py 用于构建索引
searcher.py 用于搜索相关页面
agent.py 用于调用本地大模型回答问题

先使用 requirements.txt 配置运行环境
```
python -m venv venv
venv\Scripts\activate
pip install -r requirements.txt
python .\rag_api.py
```