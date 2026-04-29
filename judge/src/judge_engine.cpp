/**
 * @file judge_engine.cpp
 * @brief 评测引擎实现
 * @author OJ Team
 * @date 2024-01-01
 */
#include "judge_engine.h"
#include "default_language_config.h"
#include <iostream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

/**
 * @brief 构造函数
 */
JudgeEngine::JudgeEngine() {
    language_config_ = std::make_unique<LanguageConfig>();
    result_judger_ = std::make_unique<ResultJudger>();

    // 优先加载外部配置文件，失败时回退到头文件中的默认 JSON 字符串。
    const std::string config_path = "config/languages.json";
    if (!language_config_->load(config_path)) {
        if (!language_config_->loadFromJsonString(kDefaultLanguageConfigJson)) {
            std::cerr << "Failed to load language config from both file and default string" << std::endl;
        } else {
            std::cerr << "Language config file not found, loaded built-in default config" << std::endl;
        }
    }
}

/**
 * @brief 析构函数
 */
JudgeEngine::~JudgeEngine() {
}

/**
 * @brief 获取编译器
 * @param language 语言
 * @return 编译器指针
 */
Compiler* JudgeEngine::getCompiler(const std::string& language) {
    if (language == "c") {
        return new CCompiler();
    } else if (language == "cpp") {
        return new CppCompiler();
    } else if (language == "java") {
        return new JavaCompiler();
    } else if (language == "python") {
        return new PythonCompiler();
    }
    return nullptr;
}

/**
 * @brief 创建沙箱配置
 * @param language 语言
 * @param time_limit_ms 时间限制（毫秒）
 * @param memory_limit_mb 内存限制（MB）
 * @return 沙箱配置
 */
SandboxConfig JudgeEngine::createSandboxConfig(const std::string& language, 
                                             int time_limit_ms, 
                                             int memory_limit_mb) {
    SandboxConfig config;
    config.time_limit_ms = time_limit_ms;
    config.memory_limit_mb = memory_limit_mb;
    config.output_limit_mb = 10;
    config.enable_network = false;
    config.enable_file_system = false;
    
    // 根据语言调整时间和内存限制
    const LanguageInfo* lang_info = language_config_->getLanguage(language);
    if (lang_info) {
        config.time_limit_ms = static_cast<int>(time_limit_ms * lang_info->time_limit_multiplier);
        config.memory_limit_mb = static_cast<int>(memory_limit_mb * lang_info->memory_limit_multiplier);
    }
    
    return config;
}

/**
 * @brief 评测代码
 * @param code 源代码
 * @param language 语言
 * @param test_cases 测试用例
 * @param time_limit_ms 时间限制（毫秒）
 * @param memory_limit_mb 内存限制（MB）
 * @return 评测结果
 */
JudgeResult JudgeEngine::judge(const std::string& code, 
                             const std::string& language, 
                             const std::vector<TestCase>& test_cases, 
                             int time_limit_ms, 
                             int memory_limit_mb) {
    JudgeResult overall_result;
    overall_result.overall_status = JudgeStatus::JUDGING;
    overall_result.runtime_ms = 0;
    overall_result.memory_kb = 0;
    overall_result.total_score = 0;
    overall_result.max_score = 0;
    
    bool all_accepted = true;
    JudgeStatus first_failure_status = JudgeStatus::ACCEPTED;
    
    // 获取编译器
    std::unique_ptr<Compiler> compiler(getCompiler(language));
    if (!compiler) {
        overall_result.overall_status = JudgeStatus::COMPILE_ERROR;
        overall_result.error_message = "Unsupported language";
        return overall_result;
    }
    
    // 编译代码
    std::string output_path = "temp_output";
    CompileResult compile_result = compiler->compile(code, output_path);
    
    if (!compile_result.success) {
        overall_result.overall_status = JudgeStatus::COMPILE_ERROR;
        overall_result.error_message = compile_result.error;
        return overall_result;
    }
    
    // 创建沙箱配置
    SandboxConfig sandbox_config = createSandboxConfig(language, time_limit_ms, memory_limit_mb);
    Sandbox sandbox(sandbox_config);
    
    // 执行每个测试用例
    for (const auto& test_case : test_cases) {
        // 构建执行命令
        std::string execute_command;
        if (language == "python") {
            execute_command = "python3 " + output_path;
        } else {
            execute_command = "./" + output_path;
        }
        
        // 执行程序
        SandboxResult sandbox_result = sandbox.execute(execute_command, test_case.input);
        
        // 判定结果
        JudgeResult test_case_result = result_judger_->judge(
            sandbox_result.output, 
            test_case.expected_output, 
            sandbox_result
        );
        
        if (test_case_result.test_case_results.empty()) {
            // 理论上不会发生；但为了健壮性避免越界
            all_accepted = false;
            if (first_failure_status == JudgeStatus::ACCEPTED) {
                first_failure_status = JudgeStatus::WRONG_ANSWER;
            }
            continue;
        }
        
        // 当前实现每次只生成一个 test_case_result（对应一个测试用例）
        auto tc = test_case_result.test_case_results.front();
        tc.case_id = test_case.id;
        tc.score_awarded = tc.passed ? test_case.score : 0;
        overall_result.test_case_results.push_back(tc);
        
        overall_result.runtime_ms += tc.runtime_ms;
        if (tc.memory_kb > overall_result.memory_kb) {
            overall_result.memory_kb = tc.memory_kb;
        }
        
        overall_result.total_score += tc.score_awarded;
        overall_result.max_score += test_case.score;
        
        if (tc.passed == false) {
            all_accepted = false;
            if (first_failure_status == JudgeStatus::ACCEPTED) {
                // 保留具体失败原因（可能是 TLE/MLE/RE/WA 等）
                first_failure_status = tc.status;
            }
        }
        
        // 继续评测后续用例以计算部分分
        if (tc.status != JudgeStatus::ACCEPTED && !test_case_result.error_message.empty() && overall_result.error_message.empty()) {
            overall_result.error_message = test_case_result.error_message;
        }
    }
    
    // 汇总整体结果
    if (all_accepted) {
        overall_result.overall_status = JudgeStatus::ACCEPTED;
    } else {
        overall_result.overall_status = first_failure_status;
    }
    
    // 清理临时文件
    try {
        fs::remove(output_path);
    } catch (...) {
        // 忽略清理错误
    }
    
    return overall_result;
}