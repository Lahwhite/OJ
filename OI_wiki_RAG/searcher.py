from typing import Optional
from whoosh import index
from whoosh.qparser import MultifieldParser, OrGroup
from whoosh.highlight import Highlighter, SentenceFragmenter
from config import INDEX_DIR, TOP_K


def search_wiki(query_str: str) -> str:
    """在 OI Wiki 索引中执行关键词搜索，返回格式化文本"""
    query_str = query_str.strip()
    if not query_str:
        return "搜索关键词不能为空。"

    try:
        ix = index.open_dir(INDEX_DIR)
    except Exception as e:
        return f"搜索失败：索引未找到或损坏。请先运行 build_index.py。错误：{e}"

    try:
        with ix.searcher() as searcher:
            parser = MultifieldParser(["title", "content"], schema=ix.schema, group=OrGroup)
            q = parser.parse(query_str)
            results = searcher.search(q, limit=TOP_K)
            if not results:
                return f"未找到与「{query_str}」相关的内容。"

            snippets = []
            for hit in results:
                # 直接获取原始正文（无任何第三方API调用）
                content = hit["content"]
                # 第一步：先截取前500个字符
                summary = content[:500]

                # 第二步：智能截断（保证句子完整，不出现半截话）
                if len(content) > 500:
                    # 找最后一个 中文句号/换行/英文句号 作为截断点
                    last_break = max(summary.rfind("。"), summary.rfind("\n"), summary.rfind(". "))
                    # 只有截断点大于100字符时才截断（避免摘要太短）
                    if last_break > 100:
                        summary = summary[:last_break + 1]
                    # 追加省略号，表示内容未展示完
                    summary += "..."

                # 格式化单条结果
                snippet = (
                    f"【{hit['title']}】\n"
                    f"路径：{hit['path']}\n"
                    f"摘要：{summary}"
                )
                snippets.append(snippet)

            return "\n\n---\n\n".join(snippets)

    except Exception as e:
        return f"搜索过程异常：{e}"

if __name__ == "__main__":
    keyword = input("输入搜索关键词: ").strip()
    print(search_wiki(keyword))