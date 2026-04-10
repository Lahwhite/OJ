#pragma once

#include <chrono>
#include <fstream>
#include <mutex>
#include <string>

namespace oj {

enum class LogLevel { Debug, Info, Warning, Error };

class Logger {
public:
    static Logger& instance();

    void init(LogLevel level, const std::string& log_file_path = "");
    void setLevel(LogLevel level);

    void debug(const std::string& msg);
    void info(const std::string& msg);
    void warning(const std::string& msg);
    void error(const std::string& msg);

private:
    Logger() = default;
    void write(LogLevel lvl, const std::string& msg);
    static const char* levelName(LogLevel lvl);
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
