#ifndef JUDGE_ENGINE_H
#define JUDGE_ENGINE_H

#include "compiler.h"
#include "sandbox.h"
#include "result_judger.h"
#include "language_config.h"
#include <vector>
#include <string>
#include <memory>

// 输入用例：标准输入、期望输出与分值
struct TestCase {
    int id;
    std::string input;
    std::string expected_output;
    int score;
};

// 评测引擎：编排编译、沙箱运行与结果判定
class JudgeEngine {
public:
    JudgeEngine();
    
    ~JudgeEngine();
    
    JudgeResult judge(const std::string& code, 
                   const std::string& language, 
                   const std::vector<TestCase>& test_cases, 
                   int time_limit_ms, 
                   int memory_limit_mb);
    
private:
    std::unique_ptr<LanguageConfig> language_config_;
    std::unique_ptr<ResultJudger> result_judger_;
    
    Compiler* getCompiler(const std::string& language);
    
    SandboxConfig createSandboxConfig(const std::string& language, 
                                   int time_limit_ms, 
                                   int memory_limit_mb);
};

#endif // JUDGE_ENGINE_H
