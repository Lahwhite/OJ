/**
 * @file result_judger.cpp
 * @brief 评测结果判定实现
 * @author OJ Team
 * @date 2024-01-01
 */
#include "result_judger.h"

/**
 * @brief 修剪字符串
 * @param str 输入字符串
 * @return 修剪后的字符串
 */
std::string ResultJudger::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return str;
    
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

/**
 * @brief 比较输出
 * @param actual 实际输出
 * @param expected 预期输出
 * @return 是否匹配
 */
bool ResultJudger::compareOutput(const std::string& actual, const std::string& expected) {
    std::string actual_trimmed = trim(actual);
    std::string expected_trimmed = trim(expected);
    
    return actual_trimmed == expected_trimmed;
}

/**
 * @brief 确定状态
 * @param result 沙箱执行结果
 * @param output_matches 输出是否匹配
 * @return 评测状态
 */
JudgeStatus ResultJudger::determineStatus(const SandboxResult& result, bool output_matches) {
    if (result.timeout) {
        return JudgeStatus::TIME_LIMIT_EXCEEDED;
    }
    
    if (result.memory_exceeded) {
        return JudgeStatus::MEMORY_LIMIT_EXCEEDED;
    }
    
    if (result.exit_code != 0) {
        return JudgeStatus::RUNTIME_ERROR;
    }
    
    if (!output_matches) {
        return JudgeStatus::WRONG_ANSWER;
    }
    
    return JudgeStatus::ACCEPTED;
}

/**
 * @brief 判定评测结果
 * @param actual_output 实际输出
 * @param expected_output 预期输出
 * @param sandbox_result 沙箱执行结果
 * @return 评测结果
 */
JudgeResult ResultJudger::judge(const std::string& actual_output, 
                              const std::string& expected_output, 
                              const SandboxResult& sandbox_result) {
    JudgeResult result;
    
    // 比较输出
    bool output_matches = compareOutput(actual_output, expected_output);
    
    // 确定状态
    result.overall_status = determineStatus(sandbox_result, output_matches);
    result.runtime_ms = sandbox_result.runtime_ms;
    result.memory_kb = sandbox_result.memory_kb;
    
    if (!sandbox_result.error.empty()) {
        result.error_message = sandbox_result.error;
    }
    
    // 创建测试用例结果
    TestCaseResult test_case_result;
    test_case_result.case_id = 0; // 默认由 JudgeEngine 覆盖为真实 case_id
    test_case_result.status = result.overall_status;
    test_case_result.runtime_ms = sandbox_result.runtime_ms;
    test_case_result.memory_kb = sandbox_result.memory_kb;
    test_case_result.output = actual_output;
    test_case_result.expected_output = expected_output;
    test_case_result.passed = (result.overall_status == JudgeStatus::ACCEPTED);
    test_case_result.score_awarded = 0; // 默认由 JudgeEngine 覆盖为本用例获得分
    
    result.test_case_results.push_back(test_case_result);
    
    return result;
}