from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
import uvicorn
import logging

# 导入您的 RAG Agent 主函数
from agent import run_agent  # 请确保 agent.py 中有 run_agent(question) 函数

# 配置日志
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

app = FastAPI(title="RAG Agent Service")

# 允许跨域（如果后续前端直接调用，或 Spring Boot 转发时不需要，但加上无害）
app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],  # 生产环境请改为具体域名
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


class QueryRequest(BaseModel):
    question: str


class QueryResponse(BaseModel):
    answer: str


@app.post("/api/rag/query", response_model=QueryResponse)
async def rag_query(request: QueryRequest):
    question = request.question.strip()
    if not question:
        raise HTTPException(status_code=400, detail="问题不能为空")

    logger.info(f"收到问题: {question}")
    try:
        # 调用您的 RAG 核心函数
        answer = run_agent(question)
        logger.info(f"回答生成成功，长度: {len(answer)}")
        return QueryResponse(answer=answer)
    except Exception as e:
        logger.exception("RAG 处理失败")
        raise HTTPException(status_code=500, detail=f"内部错误: {str(e)}")


if __name__ == "__main__":
    uvicorn.run(app, host="127.0.0.1", port=8001)