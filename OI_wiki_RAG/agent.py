import re
import ollama
from config import LLM_MODEL, MAX_SEARCH_STEPS
from searcher import search_wiki

SYSTEM_PROMPT = f"""你是精通OI（信息学奥林匹克竞赛）的专业助手。你必须严格依赖本地 OI Wiki 搜索引擎来获取知识，禁止编造任何未搜索到的信息。

你的推理流程必须严格遵循以下格式（不可省略、不可修改）：

Thought: <分析问题，判断是否需要搜索>
Action: Search
Action Input: <精准的搜索关键词，中文或英文，多个词用空格分隔>
Observation: <系统会自动填入搜索结果>

当信息足够时，直接输出：
Final Answer: <你的详细解答>

核心规则：
- 每次只能执行一次搜索，我将立即返回结果。
- 最多可搜索 {MAX_SEARCH_STEPS} 次，请珍惜搜索次数。
- 搜索关键词应精准，可包含算法名称、数据结构、公式中的关键符号等。
- 如果搜索结果为空或不相关，请根据已搜到的信息诚实回答，或说明未找到资料。
- 绝对不要编造答案，也不要省略格式。
- 输出时不要增加任何多余的标点或修饰，只保留指定的格式。
"""

# 分步提取正则（简洁、高性能）
ACTION_LINE = re.compile(r"^\s*Action:\s*Search\s*$", re.IGNORECASE | re.MULTILINE)
ACTION_INPUT = re.compile(r"^\s*Action Input:\s*(.+)$", re.IGNORECASE | re.MULTILINE)
FINAL_ANSWER = re.compile(r"^\s*Final Answer:\s*(.+)", re.IGNORECASE | re.MULTILINE | re.DOTALL)

def query_llm(messages):
    try:
        response = ollama.chat(
            model=LLM_MODEL,
            messages=messages,
            options={"temperature": 0.1}
        )
        return response['message']['content'].strip()
    except Exception as e:
        return f"[LLM_ERROR] {e}"

def parse_llm_output(text: str):
    """返回 (keyword_or_answer, is_final)"""
    final_match = FINAL_ANSWER.search(text)
    if final_match:
        return final_match.group(1).strip(), True

    if ACTION_LINE.search(text) and ACTION_INPUT.search(text):
        keyword = ACTION_INPUT.search(text).group(1).strip()
        if keyword:
            return keyword, False
    return None, False

def run_agent(question: str) -> str:
    messages = [
        {"role": "system", "content": SYSTEM_PROMPT},
        {"role": "user", "content": f"用户问题：{question}"}
    ]

    step = 0
    format_errors = 0
    last_keywords = set()   # 轻量去重：记录用过的关键词，防止重复搜索浪费步数

    print(f"📝 用户问题：{question}\n")

    while step < MAX_SEARCH_STEPS:
        step += 1
        print(f"===== 推理步骤 {step}/{MAX_SEARCH_STEPS} =====")

        llm_reply = query_llm(messages)
        if llm_reply.startswith("[LLM_ERROR]"):
            print(f"❌ 模型调用失败：{llm_reply}")
            return "抱歉，本地大模型服务出现问题，请检查 Ollama 是否正常运行。"

        print(f"🤖 模型输出：\n{llm_reply}\n")
        messages.append({"role": "assistant", "content": llm_reply})

        keyword_or_answer, is_final = parse_llm_output(llm_reply)

        if is_final:
            print("✅ 已获得最终答案！")
            return keyword_or_answer

        if keyword_or_answer:
            # 去重检查：如果关键词已经搜索过，提醒模型换词，但不消耗步数（此处策略：仍消耗步数但给出明确反馈）
            if keyword_or_answer in last_keywords:
                print(f"⚠️  关键词「{keyword_or_answer}」已搜索过，提醒模型更换...")
                messages.append({
                    "role": "user",
                    "content": f"「{keyword_or_answer}」刚刚搜索过，请换一个不同的关键词再搜索。"
                })
                # 注意：这里我们依然消耗了一步，因为模型已经输出了错误的行为，需要让它学会避免重复
                # 但更友好的做法是不计步，不过会增加循环复杂度，权衡后保留计步，通过提醒纠正
                continue

            # 记录关键词
            last_keywords.add(keyword_or_answer)
            format_errors = 0

            print(f"🔍 执行搜索：{keyword_or_answer}")
            search_res = search_wiki(keyword_or_answer)

            # 如果返回结果明显是“未找到”，在末尾附加简短提示（不消耗性能）
            if "未找到" in search_res or "无结果" in search_res:
                search_res += "\n【提示：本关键词无结果，请拆分或更换关键词重试】"

            preview = search_res[:200] + "..." if len(search_res) > 200 else search_res
            print(f"📄 搜索结果预览：{preview}\n")

            messages.append({
                "role": "user",
                "content": f"Observation: {search_res}"
            })
        else:
            format_errors += 1
            print(f"⚠️  输出格式不正确 (连续错误 {format_errors}/2)")

            if format_errors >= 2:
                print("⏰ 格式错误次数过多，强制生成答案...")
                messages.append({
                    "role": "user",
                    "content": "请忽略之前的格式要求，直接输出 Final Answer: 你的答案"
                })
                final_reply = query_llm(messages)
                ans, _ = parse_llm_output(final_reply)
                return ans if ans else final_reply
            else:
                messages.append({
                    "role": "user",
                    "content": "请严格按照指定格式输出：\nThought: ...\nAction: Search\nAction Input: 关键词\n或者直接输出 Final Answer: ..."
                })

    print(f"\n⏰ 已达最大搜索步数 ({MAX_SEARCH_STEPS})，要求模型总结...")
    messages.append({
        "role": "user",
        "content": "请根据以上所有搜索结果，直接输出 Final Answer: 你的详细回答"
    })
    final_reply = query_llm(messages)
    ans, _ = parse_llm_output(final_reply)
    return ans if ans else final_reply

if __name__ == "__main__":
    user_question = input("请输入你的 OI 相关问题：")
    answer = run_agent(user_question)
    print("\n" + "=" * 50)
    print("📘 最终答案：")
    print(answer)
    print("=" * 50)