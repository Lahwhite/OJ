#include "judge_cli_config.h"

#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>

namespace {

// 解析 --key=value 命令行参数
bool parseArg(int argc, char* argv[], const std::string& key, std::string& out) {
    const std::string prefix = "--" + key + "=";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a.rfind(prefix, 0) == 0) {
            out = a.substr(prefix.size());
            return true;
        }
    }
    return false;
}

// 解析布尔配置：支持 1/0、true/false、yes/no 等写法
bool parseBoolString(const std::string& value, bool default_value) {
    if (value.empty()) {
        return default_value;
    }
    if (value == "1" || value == "true" || value == "yes" || value == "on") {
        return true;
    }
    if (value == "0" || value == "false" || value == "no" || value == "off") {
        return false;
    }
    return default_value;
}

// 读取环境变量，未设置时返回空串
std::string envOrEmpty(const char* name) {
    const char* value = std::getenv(name);
    return value ? std::string(value) : std::string();
}

// 解析端口范围，支持 JSON 数组 [start,end] 或字符串 "start-end"
void applyPortRange(const nlohmann::json& node, JudgeCliConfig& cfg) {
    if (node.is_array() && node.size() >= 2) {
        cfg.web_port_start = node.at(0).get<int>();
        cfg.web_port_end = node.at(1).get<int>();
        return;
    }
    if (node.is_string()) {
        const auto text = node.get<std::string>();
        const auto dash = text.find('-');
        if (dash != std::string::npos) {
            cfg.web_port_start = std::stoi(text.substr(0, dash));
            cfg.web_port_end = std::stoi(text.substr(dash + 1));
        }
    }
}

// 将 judge_cli.json 中的字段映射到配置结构体
void applyJsonConfig(const nlohmann::json& root, JudgeCliConfig& cfg) {
    if (root.contains("open_web")) {
        cfg.open_web = root.at("open_web").get<bool>();
    }
    if (root.contains("web_mode")) {
        cfg.web_mode = parseWebMode(root.at("web_mode").get<std::string>());
    }
    if (root.contains("web_host")) {
        cfg.web_host = root.at("web_host").get<std::string>();
    }
    if (root.contains("web_port")) {
        cfg.web_port = root.at("web_port").get<std::string>();
    }
    if (root.contains("web_port_range")) {
        applyPortRange(root.at("web_port_range"), cfg);
    }
    if (root.contains("web_keep_alive_sec")) {
        cfg.web_keep_alive_sec = root.at("web_keep_alive_sec").get<int>();
    }
    if (root.contains("web_url")) {
        cfg.web_url = root.at("web_url").get<std::string>();
    }
    if (root.contains("username")) {
        cfg.username = root.at("username").get<std::string>();
    }
}

bool loadConfigFile(const std::filesystem::path& path, JudgeCliConfig& cfg) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        return false;
    }
    try {
        nlohmann::json root = nlohmann::json::parse(in);
        applyJsonConfig(root, cfg);
        return true;
    } catch (...) {
        return false;
    }
}
} // namespace

// 将字符串 web_mode 转为枚举：embedded / file_only / external
WebMode parseWebMode(const std::string& value) {
    if (value == "embedded") {
        return WebMode::Embedded;
    }
    if (value == "file_only") {
        return WebMode::FileOnly;
    }
    if (value == "external") {
        return WebMode::External;
    }
    return WebMode::Embedded;
}

// 加载 CLI 配置，优先级：默认值 < 配置文件 < 环境变量 < 命令行参数
JudgeCliConfig loadJudgeCliConfig(int argc, char* argv[], const std::filesystem::path& exe_dir) {
    JudgeCliConfig cfg;

    // 查找配置文件：--config 参数 > exe_dir/config/ > exe_dir 根目录
    std::filesystem::path config_path;
    std::string config_arg;
    if (parseArg(argc, argv, "config", config_arg)) {
        config_path = config_arg;
    } else if (std::filesystem::exists(exe_dir / "config" / "judge_cli.json")) {
        config_path = exe_dir / "config" / "judge_cli.json";
    } else if (std::filesystem::exists(exe_dir / "judge_cli.json")) {
        config_path = exe_dir / "judge_cli.json";
    }

    if (!config_path.empty()) {
        loadConfigFile(config_path, cfg);
    }

    // 环境变量覆盖配置文件中的 Web 相关选项
    const std::string env_mode = envOrEmpty("OJ_WEB_MODE");
    if (!env_mode.empty()) {
        cfg.web_mode = parseWebMode(env_mode);
    }
    const std::string env_open = envOrEmpty("OJ_OPEN_WEB");
    if (!env_open.empty()) {
        cfg.open_web = parseBoolString(env_open, cfg.open_web);
    }
    const std::string env_host = envOrEmpty("OJ_WEB_HOST");
    if (!env_host.empty()) {
        cfg.web_host = env_host;
    }
    const std::string env_port = envOrEmpty("OJ_WEB_PORT");
    if (!env_port.empty()) {
        cfg.web_port = env_port;
    }
    const std::string env_range = envOrEmpty("OJ_WEB_PORT_RANGE");
    if (!env_range.empty()) {
        applyPortRange(nlohmann::json(env_range), cfg);
    }
    const std::string env_url = envOrEmpty("OJ_WEB_URL");
    if (!env_url.empty()) {
        cfg.web_url = env_url;
    }
    const std::string env_keep = envOrEmpty("OJ_WEB_KEEP_ALIVE_SEC");
    if (!env_keep.empty()) {
        cfg.web_keep_alive_sec = std::stoi(env_keep);
    }
    const std::string env_username = envOrEmpty("OJ_USERNAME");
    if (!env_username.empty()) {
        cfg.username = env_username;
    }

    // 命令行参数拥有最高优先级
    std::string arg_mode;
    if (parseArg(argc, argv, "web_mode", arg_mode)) {
        cfg.web_mode = parseWebMode(arg_mode);
    }
    std::string arg_open;
    if (parseArg(argc, argv, "open_web", arg_open)) {
        cfg.open_web = parseBoolString(arg_open, cfg.open_web);
    }
    std::string arg_host;
    if (parseArg(argc, argv, "web_host", arg_host)) {
        cfg.web_host = arg_host;
    }
    std::string arg_port;
    if (parseArg(argc, argv, "web_port", arg_port)) {
        cfg.web_port = arg_port;
    }
    std::string arg_range;
    if (parseArg(argc, argv, "web_port_range", arg_range)) {
        applyPortRange(nlohmann::json(arg_range), cfg);
    }
    std::string arg_keep;
    if (parseArg(argc, argv, "web_keep_alive_sec", arg_keep)) {
        cfg.web_keep_alive_sec = std::stoi(arg_keep);
    }
    std::string arg_url;
    if (parseArg(argc, argv, "web_url", arg_url)) {
        cfg.web_url = arg_url;
    }
    std::string arg_username;
    if (parseArg(argc, argv, "username", arg_username)) {
        cfg.username = arg_username;
    }

    // 规范化：端口范围升序、最小存活时间 30 秒
    if (cfg.web_port_end < cfg.web_port_start) {
        std::swap(cfg.web_port_start, cfg.web_port_end);
    }
    if (cfg.web_keep_alive_sec < 30) {
        cfg.web_keep_alive_sec = 30;
    }

    return cfg;
}
