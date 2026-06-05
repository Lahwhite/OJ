#ifndef RESULT_JUDGER_H
#define RESULT_JUDGER_H

#include <string>
#include <vector>
#include "sandbox.h"

// 评测判定状态，与 JSON overall_status 整数值一一对应
enum class JudgeStatus {
    PENDING,
    JUDGING,
    ACCEPTED,
    WRONG_ANSWER,
    TIME_LIMIT_EXCEEDED,
    MEMORY_LIMIT_EXCEEDED,
    RUNTIME_ERROR,
    COMPILE_ERROR
};

// 单个测试用例的判定详情
struct TestCaseResult {
    int case_id;
    JudgeStatus status;
    int runtime_ms;
    int memory_kb;
    std::string output;
    std::string expected_output;
    bool passed;
    int score_awarded = 0;
};

// 整体评测结果，含各用例结果列表与得分汇总
struct JudgeResult {
    JudgeStatus overall_status;
    int runtime_ms;
    int memory_kb;
    std::string error_message;
    std::vector<TestCaseResult> test_case_results;

    int total_score = 0;  // 实际获得总分
    int max_score = 0;    // 满分（各用例 score 之和）
};

// 输出比对与状态判定逻辑
class ResultJudger {
public:
    JudgeResult judge(const std::string& actual_output, 
                   const std::string& expected_output, 
                   const SandboxResult& sandbox_result);
    
private:
    bool compareOutput(const std::string& actual, 
                    const std::string& expected);
    
    JudgeStatus determineStatus(const SandboxResult& result, 
                          bool output_matches);
    
    std::string trim(const std::string& str);
};

#endif // RESULT_JUDGER_H
