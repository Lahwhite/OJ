#include "result_judger.h"

// 去除输出首尾空白字符，避免换行/空格导致 WA
std::string ResultJudger::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) return str;
    
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// 精确比对 trim 后的实际输出与期望输出
bool ResultJudger::compareOutput(const std::string& actual, const std::string& expected) {
    std::string actual_trimmed = trim(actual);
    std::string expected_trimmed = trim(expected);
    
    return actual_trimmed == expected_trimmed;
}

// 按优先级判定：TLE > MLE > RE > WA > AC
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

// 单用例判定：综合沙箱运行结果与输出比对
JudgeResult ResultJudger::judge(const std::string& actual_output, 
                              const std::string& expected_output, 
                              const SandboxResult& sandbox_result) {
    JudgeResult result;
    
    bool output_matches = compareOutput(actual_output, expected_output);
    
    result.overall_status = determineStatus(sandbox_result, output_matches);
    result.runtime_ms = sandbox_result.runtime_ms;
    result.memory_kb = sandbox_result.memory_kb;
    
    // 沙箱 stderr 作为错误信息传递给上层
    if (!sandbox_result.error.empty()) {
        result.error_message = sandbox_result.error;
    }
    
    // 封装单用例结果（case_id 由上层 JudgeEngine 填充）
    TestCaseResult test_case_result;
    test_case_result.case_id = 0;
    test_case_result.status = result.overall_status;
    test_case_result.runtime_ms = sandbox_result.runtime_ms;
    test_case_result.memory_kb = sandbox_result.memory_kb;
    test_case_result.output = actual_output;
    test_case_result.expected_output = expected_output;
    // 仅当状态为 AC 时标记 passed
    test_case_result.passed = (result.overall_status == JudgeStatus::ACCEPTED);
    test_case_result.score_awarded = 0;
    
    result.test_case_results.push_back(test_case_result);
    
    return result;
}
