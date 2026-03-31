/**
 * @file submission_handler.h
 * @brief 提交处理器
 * @author OJ Team
 * @date 2024-01-01
 */
#ifndef SUBMISSION_HANDLER_H
#define SUBMISSION_HANDLER_H

#include "crow_all.h"
#include <memory>
#include "judge_engine.h"

/**
 * @class SubmissionHandler
 * @brief 提交处理器类
 */
class SubmissionHandler {
public:
    /**
     * @brief 构造函数
     */
    SubmissionHandler();
    
    /**
     * @brief 处理代码提交
     * @param req HTTP请求
     * @return HTTP响应
     */
    crow::response handleSubmit(const crow::request& req);
    
    /**
     * @brief 获取提交列表
     * @param req HTTP请求
     * @return HTTP响应
     */
    crow::response handleGetSubmissions(const crow::request& req);
    
    /**
     * @brief 获取提交详情
     * @param req HTTP请求
     * @param id 提交ID
     * @return HTTP响应
     */
    crow::response handleGetSubmission(const crow::request& req, int64_t id);
    
    /**
     * @brief 获取评测状态（SSE）
     * @param req HTTP请求
     * @param id 提交ID
     * @return HTTP响应
     */
    crow::response handleSubmissionStatus(const crow::request& req, int64_t id);
    
    /**
     * @brief 启动服务器
     * @param port 端口号
     */
    void startServer(uint16_t port);
    
private:
    std::unique_ptr<JudgeEngine> judge_engine_; ///< 评测引擎
    std::unique_ptr<crow::SimpleApp> app_; ///< Crow应用
    
    /**
     * @brief 验证提交
     * @param req HTTP请求
     * @return 是否验证通过
     */
    bool validateSubmission(const crow::request& req);
};

#endif // SUBMISSION_HANDLER_H