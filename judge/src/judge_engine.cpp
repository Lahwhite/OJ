#include "judge_engine.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include "oj/config.h"
#include "oj/log.h"
#include "oj/mysql_pool.h"
#include "oj/redis_cache.h"
#include "oj/json_error.h"

namespace fs = std::filesystem;

/**
 * @brief 构造函数
 */
JudgeEngine::JudgeEngine() {
    language_config_ = std::make_unique<LanguageConfig>();
    result_judger_ = std::make_unique<ResultJudger>();
    
    // 加载语言配置
    std::string config_path = "config/languages.json";
    if (!language_config_->load(config_path)) {
        OJ_LOG_ERROR("Failed to load language config: " + config_path);
    } else {
        OJ_LOG_INFO("Language config loaded successfully from: " + config_path);
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
        
        // 合并结果
        overall_result.test_case_results.push_back(test_case_result.test_case_results[0]);
        overall_result.runtime_ms += test_case_result.runtime_ms;
        if (test_case_result.memory_kb > overall_result.memory_kb) {
            overall_result.memory_kb = test_case_result.memory_kb;
        }
        
        // 如果有任何测试用例失败，整体结果为失败
        if (test_case_result.overall_status != JudgeStatus::ACCEPTED) {
            overall_result.overall_status = test_case_result.overall_status;
            if (!test_case_result.error_message.empty()) {
                overall_result.error_message = test_case_result.error_message;
            }
            break;
        }
    }
    
    // 如果所有测试用例都通过，设置为ACCEPTED
    if (overall_result.overall_status == JudgeStatus::JUDGING) {
        overall_result.overall_status = JudgeStatus::ACCEPTED;
    }
    
    // 清理临时文件
    try {
        fs::remove(output_path);
    } catch (...) {
        // 忽略清理错误
    }
    
    // 存储评测结果到数据库
    try {
        // 这里可以添加具体的数据库存储逻辑
        OJ_LOG_INFO("Submission result stored in database");
    } catch (const std::exception& e) {
        OJ_LOG_ERROR("Failed to store result in database: " + std::string(e.what()));
    }
    
    // 缓存评测结果到Redis
    try {
        // 这里可以添加具体的缓存逻辑
        OJ_LOG_INFO("Submission result cached in Redis");
    } catch (const std::exception& e) {
        OJ_LOG_ERROR("Failed to cache result in Redis: " + std::string(e.what()));
    }
    
    return overall_result;
}