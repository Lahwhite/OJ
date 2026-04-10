#include "oj/log.h"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>

namespace oj {

Logger& Logger::instance() {
    static Logger inst;
    return inst;
}

void Logger::init(LogLevel level, const std::string& log_file_path) {
    std::lock_guard<std::mutex> lock(mutex_);
    level_ = level;
    file_.close();
    if (!log_file_path.empty()) {
        file_.open(log_file_path, std::ios::out | std::ios::app);
    }
}

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
    return "?";
}

bool Logger::shouldLog(LogLevel lvl) const {
    return static_cast<int>(lvl) >= static_cast<int>(level_);
}

void Logger::write(LogLevel lvl, const std::string& msg) {
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
    if (file_.is_open()) {
        file_ << out;
        file_.flush();
    }
}

void Logger::debug(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Debug, msg);
}

void Logger::info(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Info, msg);
}

void Logger::warning(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Warning, msg);
}

void Logger::error(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mutex_);
    write(LogLevel::Error, msg);
}

}  // namespace oj
