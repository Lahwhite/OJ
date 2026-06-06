// common 模块实现文件：负责公共能力的具体实现与底层细节
#include "oj/log.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace oj {

Logger& Logger::instance() {
    static Logger inst;
    // 返回当前阶段的处理结果或默认兜底值
    return inst;
}

// 过程型函数：主要通过副作用完成状态更新
void Logger::init(LogLevel level, const std::string& log_file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
    // 每次 init 都把旧文件句柄关掉，防止重复打开
    file_.close();
    if (!log_file_path.empty()) {
        file_.open(log_file_path, std::ios::out | std::ios::app);
    }
}

// 过程型函数：主要通过副作用完成状态更新
void Logger::setLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
}

const char* Logger::levelName(LogLevel lvl) {
    switch (lvl) {
        case LogLevel::Debug: return "DEBUG";
        case LogLevel::Info: return "INFO";
        case LogLevel::Warning: return "WARN";
        case LogLevel::Error: return "ERROR";
    }
    // 返回当前阶段的处理结果或默认兜底值
    return "?";
}

// 布尔返回值通常用于表示是否执行成功
bool Logger::shouldLog(LogLevel lvl) const {
    // 返回当前阶段的处理结果或默认兜底值
    return static_cast<int>(lvl) >= static_cast<int>(level_);
}

// 过程型函数：主要通过副作用完成状态更新
void Logger::write(LogLevel lvl, const std::string& msg) {
    // 条件判断：根据运行时状态决定后续流程
    if (!shouldLog(lvl)) {
        return;
    }
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_buf{};
#if defined(_WIN32)
    localtime_s(&tm_buf, &t);
#else
    localtime_r(&t, &tm_buf);
#endif
    std::ostringstream line;
    line << std::put_time(&tm_buf, "%Y-%m-%d %H:%M:%S")
         << " [" << levelName(lvl) << "] " << msg << '\n';
    const std::string out = line.str();
    std::cerr << out;
    // 条件判断：根据运行时状态决定后续流程
    if (file_.is_open()) {
        file_ << out;
        file_.flush();
    }
}

// 过程型函数：主要通过副作用完成状态更新
void Logger::debug(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Debug, msg);
}

// 过程型函数：主要通过副作用完成状态更新
void Logger::info(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Info, msg);
}

// 过程型函数：主要通过副作用完成状态更新
void Logger::warning(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Warning, msg);
}

// 过程型函数：主要通过副作用完成状态更新
void Logger::error(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Error, msg);
}

}  // namespace oj
