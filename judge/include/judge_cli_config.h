#ifndef JUDGE_CLI_CONFIG_H
#define JUDGE_CLI_CONFIG_H

#include <filesystem>
#include <string>

// Web 结果展示模式
enum class WebMode {
    Embedded,
    FileOnly,
    External
};

// judge_engine CLI 的 Web 相关配置项
struct JudgeCliConfig {
    WebMode web_mode = WebMode::Embedded;
    bool open_web = true;
    std::string web_host = "127.0.0.1";
    std::string web_port = "auto";
    int web_port_start = 8080;
    int web_port_end = 8180;
    int web_keep_alive_sec = 300;
    std::string web_url;
};

// 合并配置文件、环境变量与命令行参数，返回最终 CLI 配置
JudgeCliConfig loadJudgeCliConfig(int argc, char* argv[], const std::filesystem::path& exe_dir);

WebMode parseWebMode(const std::string& value);

#endif
