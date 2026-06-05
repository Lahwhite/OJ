#include "judge_engine.h"
#include "default_language_config.h"
#include <iostream>
#include <sstream>
#include <filesystem>
#include <cctype>

namespace fs = std::filesystem;

namespace {

// 从源码中解析 Java public class 名，用于 javac/java 命令
std::string guessJavaPublicClassName(const std::string& source_code) {
    const std::string key = "public class";
    const auto pos = source_code.find(key);
    if (pos == std::string::npos) {
        return "Main";
    }

    size_t i = pos + key.size();
    while (i < source_code.size() && std::isspace(static_cast<unsigned char>(source_code[i]))) {
        ++i;
    }

    std::string name;
    while (i < source_code.size()) {
        const unsigned char c = static_cast<unsigned char>(source_code[i]);
        if (std::isalnum(c) || c == '_' || c == '$') {
            name.push_back(static_cast<char>(c));
            ++i;
            continue;
        }
        break;
    }

    return name.empty() ? "Main" : name;  // 未找到时回退为 Main
}
} // namespace

JudgeEngine::JudgeEngine() {
    language_config_ = std::make_unique<LanguageConfig>();
    result_judger_ = std::make_unique<ResultJudger>();

    // 优先加载外部配置，失败则回退到内置默认语言表
    const std::string config_path = "config/languages.json";
    if (!language_config_->load(config_path)) {
        if (!language_config_->loadFromJsonString(kDefaultLanguageConfigJson)) {
            std::cerr << "Failed to load language config from both file and default string" << std::endl;
        } else {
            std::cerr << "Language config file not found, loaded built-in default config" << std::endl;
        }
    }
}

JudgeEngine::~JudgeEngine() {
}

// 按语言标识创建对应编译器实例（调用方负责释放）
// 按语言标识创建对应编译器实例（调用方负责释放）
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

// 构建沙箱运行参数，并按语言配置应用时限/内存倍率
SandboxConfig JudgeEngine::createSandboxConfig(const std::string& language, 
                                             int time_limit_ms, 
                                             int memory_limit_mb) {
    SandboxConfig config;
    config.time_limit_ms = time_limit_ms;
    config.memory_limit_mb = memory_limit_mb;
    config.output_limit_mb = 10;
    config.enable_network = false;      // 禁止网络访问
    config.enable_file_system = false;  // 禁止额外文件系统操作
    
    const LanguageInfo* lang_info = language_config_->getLanguage(language);
    if (lang_info) {
        config.time_limit_ms = static_cast<int>(time_limit_ms * lang_info->time_limit_multiplier);
        config.memory_limit_mb = static_cast<int>(memory_limit_mb * lang_info->memory_limit_multiplier);
    }
    
    return config;
}

// 评测入口：编译一次，逐用例在沙箱中运行并汇总结果
// 评测入口：编译一次，逐用例在沙箱中运行并汇总结果
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
    
    // 步骤 1：选择编译器
    // 步骤 1：选择编译器
    std::unique_ptr<Compiler> compiler(getCompiler(language));
    if (!compiler) {
        overall_result.overall_status = JudgeStatus::COMPILE_ERROR;
        overall_result.error_message = "Unsupported language";
        return overall_result;
    }
    
    // 步骤 2：确定编译产物路径（各语言产物形态不同）
    // 步骤 2：确定编译产物路径（各语言产物形态不同）
    std::string output_path = "temp_output";
    if (language == "java") {
        output_path = "temp_java_out";
    } else if (language != "python") {
#ifdef _WIN32
        output_path += ".exe";
#endif
    }
    CompileResult compile_result = compiler->compile(code, output_path);
    
    // 编译失败则直接返回 CE，不进入运行阶段
    if (!compile_result.success) {
        overall_result.overall_status = JudgeStatus::COMPILE_ERROR;
        overall_result.error_message = compile_result.error;
        return overall_result;
    }
    
    // 步骤 3：创建沙箱，按用例循环执行
    SandboxConfig sandbox_config = createSandboxConfig(language, time_limit_ms, memory_limit_mb);
    Sandbox sandbox(sandbox_config);
    
    for (const auto& test_case : test_cases) {
        // 按语言拼接运行命令
        std::string execute_command;
        if (language == "python") {
            execute_command = "python3 " + output_path;
        } else if (language == "java") {
            // Java 需指定 classpath 与主类名
            const std::string class_name = guessJavaPublicClassName(code);
            execute_command = "java -cp \"" + output_path + "\" " + class_name;
        } else {
#ifdef _WIN32
            execute_command = ".\\" + output_path;
#else
            execute_command = "./" + output_path;
#endif
        }
        
        // 在沙箱中运行并注入标准输入
        SandboxResult sandbox_result = sandbox.execute(execute_command, test_case.input);
        
        // 比对输出并判定 WA/TLE/RE 等状态
        JudgeResult test_case_result = result_judger_->judge(
            sandbox_result.output, 
            test_case.expected_output, 
            sandbox_result
        );
        
        // 判定器未返回结果时视为失败
        if (test_case_result.test_case_results.empty()) {
            all_accepted = false;
            if (first_failure_status == JudgeStatus::ACCEPTED) {
                first_failure_status = JudgeStatus::WRONG_ANSWER;
            }
            continue;
        }
        
        auto tc = test_case_result.test_case_results.front();
        tc.case_id = test_case.id;
        // 通过才计分，否则该用例得分为 0
        tc.score_awarded = tc.passed ? test_case.score : 0;
        overall_result.test_case_results.push_back(tc);
        
        // 累加总用时，取各用例峰值内存
        overall_result.runtime_ms += tc.runtime_ms;
        if (tc.memory_kb > overall_result.memory_kb) {
            overall_result.memory_kb = tc.memory_kb;
        }
        
        overall_result.total_score += tc.score_awarded;
        overall_result.max_score += test_case.score;
        
        // 记录首个失败用例的判定，作为整体 verdict
        if (tc.passed == false) {
            all_accepted = false;
            if (first_failure_status == JudgeStatus::ACCEPTED) {
                first_failure_status = tc.status;
            }
        }
        
        if (tc.status != JudgeStatus::ACCEPTED && !test_case_result.error_message.empty() && overall_result.error_message.empty()) {
            overall_result.error_message = test_case_result.error_message;
        }
    }
    
    // 步骤 4：汇总整体判定
    if (all_accepted) {
        overall_result.overall_status = JudgeStatus::ACCEPTED;
    } else {
        overall_result.overall_status = first_failure_status;
    }
    
    // 清理临时编译产物
    try {
        if (language == "java") {
            fs::remove_all(output_path);
        } else {
            fs::remove(output_path);
        }
    } catch (...) {
    }
    
    return overall_result;
}
