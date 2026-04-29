/**
 * @file result_judger.h
 * @brief 评测结果判定
 * @author OJ Team
 * @date 2024-01-01
 */
#ifndef RESULT_JUDGER_H
#define RESULT_JUDGER_H

#include <string>
#include <vector>
#include "sandbox.h"

/**
 * @enum JudgeStatus
 * @brief 评测状态枚举
 */
enum class JudgeStatus {
    PENDING,                ///< 等待评测
    JUDGING,                ///< 正在评测
    ACCEPTED,               ///< 正确
    WRONG_ANSWER,           ///< 错误答案
    TIME_LIMIT_EXCEEDED,    ///< 超时
    MEMORY_LIMIT_EXCEEDED,  ///< 内存超限
    RUNTIME_ERROR,          ///< 运行时错误
    COMPILE_ERROR           ///< 编译错误
};

/**
 * @struct TestCaseResult
 * @brief 测试用例结果结构体
 */
struct TestCaseResult {
    int case_id;            ///< 测试用例ID
    JudgeStatus status;     ///< 测试用例状态
    int runtime_ms;         ///< 运行时间（毫秒）
    int memory_kb;          ///< 内存使用（KB）
    std::string output;     ///< 实际输出
    std::string expected_output; ///< 预期输出
    bool passed;            ///< 是否通过
    int score_awarded = 0; ///< 通过获得的分数
};

/**
 * @struct JudgeResult
 * @brief 评测结果结构体
 */
struct JudgeResult {
    JudgeStatus overall_status;     ///< 整体状态
    int runtime_ms;                 ///< 总运行时间（毫秒）
    int memory_kb;                  ///< 最大内存使用（KB）
    std::string error_message;      ///< 错误信息
    std::vector<TestCaseResult> test_case_results; ///< 测试用例结果

    // 评分相关：允许部分分。
    int total_score = 0;           ///< 本次评测获得的总分
    int max_score = 0;             ///< 本次评测理论最大总分
};

/**
 * @class ResultJudger
 * @brief 结果判定器类
 */
class ResultJudger {
public:
    /**
     * @brief 判定评测结果
     * @param actual_output 实际输出
     * @param expected_output 预期输出
     * @param sandbox_result 沙箱执行结果
     * @return 评测结果
     */
    JudgeResult judge(const std::string& actual_output, 
                   const std::string& expected_output, 
                   const SandboxResult& sandbox_result);
    
private:
    /**
     * @brief 比较输出
     * @param actual 实际输出
     * @param expected 预期输出
     * @return 是否匹配
     */
    bool compareOutput(const std::string& actual, 
                    const std::string& expected);
    
    /**
     * @brief 确定状态
     * @param result 沙箱执行结果
     * @param output_matches 输出是否匹配
     * @return 评测状态
     */
    JudgeStatus determineStatus(const SandboxResult& result, 
                          bool output_matches);
    
    /**
     * @brief 修剪字符串
     * @param str 输入字符串
     * @return 修剪后的字符串
     */
    std::string trim(const std::string& str);
};

#endif // RESULT_JUDGER_H