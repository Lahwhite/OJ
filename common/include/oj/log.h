// common 模块头文件：声明公共组件对外暴露的接口与数据结构
#pragma once

#include <chrono>
#include <fstream>
#include <mutex>
#include <string>

namespace oj {

enum class LogLevel { Debug, Info, Warning, Error };

// 这里没做很复杂的日志框架，主要图一个简单好用
class Logger {
public:
    static Logger& instance();

    // 过程型函数：主要通过副作用完成状态更新
    void init(LogLevel level, const std::string& log_file_path = "");
    // 过程型函数：主要通过副作用完成状态更新
    void setLevel(LogLevel level);

    // 过程型函数：主要通过副作用完成状态更新
    void debug(const std::string& msg);
    // 过程型函数：主要通过副作用完成状态更新
    void info(const std::string& msg);
    // 过程型函数：主要通过副作用完成状态更新
    void warning(const std::string& msg);
    // 过程型函数：主要通过副作用完成状态更新
    void error(const std::string& msg);

private:
    Logger() = default;
    // 过程型函数：主要通过副作用完成状态更新
    void write(LogLevel lvl, const std::string& msg);
    static const char* levelName(LogLevel lvl);
    // 布尔返回值通常用于表示是否执行成功
    bool shouldLog(LogLevel lvl) const;

    std::mutex mutex_;
    LogLevel level_{LogLevel::Info};
    std::ofstream file_;
};

}  // namespace oj

#define OJ_LOG_DEBUG(msg) ::oj::Logger::instance().debug(msg)
#define OJ_LOG_INFO(msg) ::oj::Logger::instance().info(msg)
#define OJ_LOG_WARN(msg) ::oj::Logger::instance().warning(msg)
#define OJ_LOG_ERROR(msg) ::oj::Logger::instance().error(msg)
