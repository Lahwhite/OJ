/**
 * @file judge_engine.h
 * @brief 评测引擎
 * @author OJ Team
 * @date 2024-01-01
 */
#ifndef JUDGE_ENGINE_H
#define JUDGE_ENGINE_H

#include "compiler.h"
#include "sandbox.h"
#include "result_judger.h"
#include "language_config.h"
#include <vector>
#include <string>
#include <memory>

/**
 * @struct TestCase
 * @brief 测试用例结构体
 */
struct TestCase {
    int id;                         ///< 测试用例ID
    std::string input;              ///< 输入数据
    std::string expected_output;    ///< 预期输出
    int score;                      ///< 分值
};

/**
 * @class JudgeEngine
 * @brief 评测引擎类
 */
class JudgeEngine {
public:
    /**
     * @brief 构造函数
     */
    JudgeEngine();
    
    /**
     * @brief 析构函数
     */
    ~JudgeEngine();
    
    /**
     * @brief 评测代码
     * @param code 源代码
     * @param language 语言
     * @param test_cases 测试用例
     * @param time_limit_ms 时间限制（毫秒）
     * @param memory_limit_mb 内存限制（MB）
     * @return 评测结果
     */
    JudgeResult judge(const std::string& code, 
                   const std::string& language, 
                   const std::vector<TestCase>& test_cases, 
                   int time_limit_ms, 
                   int memory_limit_mb);
    
private:
    std::unique_ptr<LanguageConfig> language_config_; ///< 语言配置
    std::unique_ptr<ResultJudger> result_judger_;     ///< 结果判定器
    
    /**
     * @brief 获取编译器
     * @param language 语言
     * @return 编译器指针
     */
    Compiler* getCompiler(const std::string& language);
    
    /**
     * @brief 创建沙箱配置
     * @param language 语言
     * @param time_limit_ms 时间限制（毫秒）
     * @param memory_limit_mb 内存限制（MB）
     * @return 沙箱配置
     */
    SandboxConfig createSandboxConfig(const std::string& language, 
                                   int time_limit_ms, 
                                   int memory_limit_mb);
};

#endif // JUDGE_ENGINE_H