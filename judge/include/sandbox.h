#ifndef SANDBOX_H
#define SANDBOX_H

#include <string>

// 沙箱运行限制参数
struct SandboxConfig {
    int time_limit_ms;
    int memory_limit_mb;
    int output_limit_mb;
    bool enable_network;
    bool enable_file_system;
};

// 单次沙箱执行的结果（输出、错误、时限等）
struct SandboxResult {
    int exit_code;
    int runtime_ms;
    int memory_kb;
    std::string output;
    std::string error;
    bool timeout;
    bool memory_exceeded;
};

// 隔离执行用户程序并注入标准输入
class Sandbox {
public:
    Sandbox(const SandboxConfig& config);
    
    SandboxResult execute(const std::string& command, 
                       const std::string& input);
    
private:
    SandboxConfig config_;
    
    bool setupResourceLimits();
    
    bool createIsolatedEnvironment();
};

#endif // SANDBOX_H
