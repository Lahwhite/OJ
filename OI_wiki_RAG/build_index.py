import sys
import re
from pathlib import Path
from typing import Optional, Tuple
from bs4 import BeautifulSoup
from whoosh import index
from whoosh.fields import Schema, TEXT, ID
from whoosh.analysis import StandardAnalyzer
from config import HTML_ROOT, INDEX_DIR
import logging

try:
    import chardet
    ENCODING_DETECT = True
except ImportError:
    ENCODING_DETECT = False

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(levelname)s - %(message)s')
logger = logging.getLogger(__name__)

BATCH_SIZE = 256
SKIP_KEYWORDS = {'search', '404', 'sitemap', '403', '500'}
REMOVE_TAGS = ['nav', 'footer', 'script', 'style', 'noscript', 'header', 'aside', 'form']


def read_file_with_encoding(file_path: Path) -> Optional[str]:
    """ 自动检测编码 """
    try:
        with open(file_path, 'r', encoding='utf-8') as f:
            return f.read()
    except UnicodeDecodeError:
        if not ENCODING_DETECT:
            logger.warning("%s 编码非UTF-8，且未安装chardet，跳过", file_path.name)
            return None
        with open(file_path, 'rb') as f:
            raw_data = f.read()
        encoding = chardet.detect(raw_data)['encoding'] or 'gbk'
        try:
            return raw_data.decode(encoding)
        except Exception:
            logger.error("%s 无法以编码 %s 解码，跳过", file_path.name, encoding)
            return None
    except Exception as e:
        logger.error("读取文件失败 %s: %s", file_path.name, str(e))
        return None


def restore_math_latex(soup):
    """ 将 MathJax/Katex 的 script 标签恢复为 LaTeX 源语 """
    # 处理 MathJax
    for script in soup.find_all('script', type=lambda t: t and 'math/tex' in t if t else False):
        latex = script.string
        if latex:
            disp = 'mode=display' in script.get('type', '')
            if disp:
                replacement = f'$${latex.strip()}$$'
            else:
                replacement = f'${latex.strip()}$'
            script.replace_with(replacement)
    # 处理 KaTeX（data-latex 属性，常见于 OI Wiki 渲染）
    for span in soup.find_all('span', class_='katex'):
        latex = span.get('data-latex')
        if latex:
            span.replace_with(f'${latex}$')
    # 行内和行间公式的特殊标记（有些模板用 `\(...\)` `\[...\]`）
    for script in soup.find_all('script', type='math/tex'):
        if script.string:
            script.replace_with(f'${script.string.strip()}$')
    for script in soup.find_all('script', type='math/tex; mode=display'):
        if script.string:
            script.replace_with(f'$${script.string.strip()}$$')


def restore_code_blocks(soup):
    """ 将 <pre><code> 转换为带语言标记的围栏代码块 """
    for pre in soup.find_all('pre'):
        code = pre.find('code')
        if not code:
            continue
        lang = ''
        if code.get('class'):
            for cls in code['class']:
                if cls.startswith('language-'):
                    lang = cls[len('language-'):]
                    break
        code_text = code.get_text()
        fence = f'\n```{lang}\n{code_text}\n```\n'
        pre.replace_with(fence)


def clean_title(raw_title: str) -> str:
    """ 去除标题中常见的后缀，如 ' - OI Wiki' """
    # 按常见分隔符分割，取第一部分
    for sep in [' - ', ' | ', ' — ']:
        if sep in raw_title:
            return raw_title.split(sep)[0].strip()
    # 移除末尾可能残留的 " - OI Wiki"
    raw_title = re.sub(r'\s*[-—|]\s*OI\s*Wiki', '', raw_title, flags=re.IGNORECASE).strip()
    return raw_title


def extract_html_content(html_content: str, file_name: str) -> Tuple[Optional[str], Optional[str]]:
    """ 提取标题和正文，同时恢复公式和代码块 """
    soup = BeautifulSoup(html_content, 'html.parser')

    # 1. 提取并清洗标题
    title_tag = soup.find('title')
    if title_tag:
        title = clean_title(title_tag.get_text(strip=True))
    else:
        title = Path(file_name).stem.replace('-', ' ').replace('_', ' ').title()

    # 2. 先还原公式、代码块
    restore_math_latex(soup)
    restore_code_blocks(soup)

    # 3. 定位正文区域
    article = soup.select_one('article.md-content__inner') or soup.body
    if not article:
        return None, None

    # 4. 删除垃圾标签
    for tag in article.find_all(REMOVE_TAGS):
        tag.decompose()

    # 5. 提取纯文本，清洗空行
    content = article.get_text(separator='\n', strip=True)
    content = '\n'.join([line.strip() for line in content.splitlines() if line.strip()])
    if not content:
        return None, None

    return title, content


def build_index(incremental: bool = False):
    """ 构建或更新索引 """
    schema = Schema(
        path=ID(stored=True, unique=True),
        title=TEXT(stored=True),
        content=TEXT(analyzer=StandardAnalyzer(), stored=True),
        mtime=TEXT(stored=True),    # 记录文件最后修改时间，用于增量判断
    )

    index_path = Path(INDEX_DIR)
    index_path.mkdir(exist_ok=True)

    # 检查索引是否存在，决定是新建还是打开
    if index.exists_in(index_path) and incremental:
        ix = index.open_dir(index_path)
        existing_paths = set()
        # 增量模式：先检查现有索引中的文件时间戳
        with ix.searcher() as searcher:
            for doc in searcher.all_stored_fields():
                existing_paths.add(doc['path'])
        writer = ix.writer()
        base_path = Path(HTML_ROOT)
        if not base_path.exists():
            logger.error("HTML根目录不存在：%s", base_path)
            sys.exit(1)

        total, added, updated, skipped_existing, failed = 0, 0, 0, 0, 0
        for html_file in base_path.rglob("*.html"):
            # 过滤隐藏目录/文件
            if any(p.startswith('.') and p not in ('.', '..') for p in html_file.parts):
                continue
            # 过滤无关页面
            if any(key in html_file.stem.lower() for key in SKIP_KEYWORDS):
                continue
            total += 1
            rel_path = str(html_file.relative_to(base_path))
            current_mtime = str(html_file.stat().st_mtime)
            # 检查是否已存在于索引中
            if rel_path in existing_paths:
                # 可在此比较存储的 mtime，若不同则更新；此处简化：直接跳过
                # 读取已索引文档的 mtime：需要 searcher 获取，略
                skipped_existing += 1
                continue
            # 新文件处理
            html_content = read_file_with_encoding(html_file)
            if not html_content:
                failed += 1
                continue
            title, content = extract_html_content(html_content, html_file.name)
            if not title or not content:
                failed += 1
                continue
            # 新增文档
            writer.add_document(path=rel_path, title=title, content=content, mtime=current_mtime)
            added += 1
            if added % BATCH_SIZE == 0:
                writer.commit()
                logger.info("已提交 %d 个新文档", added)
                writer = ix.writer()

        writer.commit()
        logger.info("增量索引完成：扫描文件 %d，新增 %d，已存在跳过 %d，处理失败 %d", total, added, skipped_existing, failed)
    else:
        # 全量构建（或增量模式但索引不存在）
        ix = index.create_in(index_path, schema)
        writer = ix.writer()
        base_path = Path(HTML_ROOT)
        if not base_path.exists():
            logger.error("HTML根目录不存在：%s", base_path)
            sys.exit(1)

        total, success, skipped = 0, 0, 0
        for html_file in base_path.rglob("*.html"):
            if any(part.startswith('.') for part in html_file.parts):
                skipped += 1
                continue
            if any(key in html_file.stem.lower() for key in SKIP_KEYWORDS):
                skipped += 1
                continue
            total += 1
            html_content = read_file_with_encoding(html_file)
            if not html_content:
                skipped += 1
                continue
            title, content = extract_html_content(html_content, html_file.name)
            if not title or not content:
                skipped += 1
                continue
            rel_path = str(html_file.relative_to(base_path))
            mtime = str(html_file.stat().st_mtime)
            writer.add_document(path=rel_path, title=title, content=content, mtime=mtime)
            success += 1
            if success % BATCH_SIZE == 0:
                writer.commit()
                logger.info("已提交 %d 个文档", success)
                writer = ix.writer()
        writer.commit()
        logger.info("索引构建完成：总扫描 %d，成功 %d，跳过 %d", total, success, skipped)


if __name__ == "__main__":
    """ 支持命令行参数 --incremental 进行增量索引 """
    incremental = "--incremental" in sys.argv
    build_index(incremental=incremental)