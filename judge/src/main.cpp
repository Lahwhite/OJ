#include "judge_cli_config.h"
#include "judge_engine.h"
#include "web_server.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {

// 从命令行解析 --key=value 形式的参数
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

// 将内部判定枚举转为对外展示的缩写（AC/WA/TLE 等）
std::string judgeStatusToString(JudgeStatus s) {
    switch (s) {
        case JudgeStatus::PENDING:
            return "PENDING";
        case JudgeStatus::JUDGING:
            return "JUDGING";
        case JudgeStatus::ACCEPTED:
            return "AC";
        case JudgeStatus::WRONG_ANSWER:
            return "WA";
        case JudgeStatus::TIME_LIMIT_EXCEEDED:
            return "TLE";
        case JudgeStatus::MEMORY_LIMIT_EXCEEDED:
            return "MLE";
        case JudgeStatus::RUNTIME_ERROR:
            return "RE";
        case JudgeStatus::COMPILE_ERROR:
            return "CE";
        default:
            return "UNKNOWN";
    }
}

// 以二进制方式读取源码或配置文件全文
std::string readTextFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// 获取可执行文件所在目录，用于定位 config 与结果输出路径
std::filesystem::path getExecutableDir(const char* argv0) {
    std::error_code ec;
    auto abs = std::filesystem::absolute(argv0, ec);
    if (!ec) {
        auto parent = abs.parent_path();
        if (!parent.empty()) {
            return parent;
        }
    }
    return std::filesystem::current_path();
}

std::filesystem::path getExecutablePath(const char* argv0) {
    std::error_code ec;
    return std::filesystem::absolute(argv0, ec);
}

// 将评测结果序列化为 JSON 并写入磁盘
void writeJsonFile(const std::filesystem::path& path, const json& payload) {
    std::error_code ec;
    // 确保父目录存在
    std::filesystem::create_directories(path.parent_path(), ec);

    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to write result file: " + path.string());
    }
    out << payload.dump(2, ' ', true, json::error_handler_t::replace);
}

struct SavedResultPaths {
    std::filesystem::path timestamped;
    long long timestamp_ms = 0;
};

// 按时间戳命名保存结果文件，供 Web 页面拉取
SavedResultPaths writeJsonResult(const char* argv0, const json& payload, const std::string& username) {
    const auto exe_dir = getExecutableDir(argv0);
    // 毫秒时间戳保证文件名唯一
    const auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();

    SavedResultPaths saved;
    saved.timestamp_ms = ts;
    saved.timestamped = exe_dir / ("judge_result_" + username + "_" + std::to_string(ts) + ".json");
    writeJsonFile(saved.timestamped, payload);
    return saved;
}

// 解析 Web 端口：auto 时在配置范围内扫描可用端口
int resolveWebPort(const JudgeCliConfig& cfg) {
    if (cfg.web_port == "auto" || cfg.web_port == "0") {
        return findAvailablePort(cfg.web_host, cfg.web_port_start, cfg.web_port_end);
    }
    try {
        return std::stoi(cfg.web_port);
    } catch (...) {
        return -1;
    }
}

// 根据 web_mode 决定：仅写文件 / 内嵌服务 / 跳转外部页面
void presentResultInWeb(const char* argv0,
                        int argc,
                        char* argv[],
                        const JudgeCliConfig& cfg,
                        const std::filesystem::path& result_json_path,
                        long long timestamp_ms) {
    if (cfg.web_mode == WebMode::FileOnly || !cfg.open_web) {
        return;
    }

    // external 模式：直接打开用户配置的静态站点 URL
    if (cfg.web_mode == WebMode::External) {
        if (!cfg.web_url.empty()) {
            const std::string url = buildExternalResultUrl(cfg.web_url, timestamp_ms);
            openBrowserUrl(url);
            std::cout << "\nOpened external result page: " << url;
            std::cout << "\nOJ_WEB_URL=" << url;
        } else {
            std::cout << "\nweb_mode=external but web_url is empty; skip opening browser.";
        }
        return;
    }

    // embedded 模式：启动内嵌 HTTP 服务并打开浏览器
    const int port = resolveWebPort(cfg);
    if (port < 0) {
        std::cout << "\nNo available web port in configured range "
                  << cfg.web_port_start << '-' << cfg.web_port_end;
        return;
    }

    const std::string exe_path = getExecutablePath(argv0).string();
    if (!spawnDetachedWebServer(
            exe_path,
            cfg.web_host,
            port,
            cfg.web_keep_alive_sec,
            result_json_path.string())) {
        std::cout << "\nFailed to start embedded web server on port " << port;
        return;
    }
    // 等待子进程完成端口绑定后再打开浏览器
    std::this_thread::sleep_for(std::chrono::milliseconds(600));

    const std::string url = buildResultPageUrl(cfg.web_host, port, timestamp_ms);
    openBrowserUrl(url);
    std::cout << "\nOpened result page: " << url;
    // 输出机器可读变量，供外部脚本解析
    std::cout << "\nOJ_WEB_URL=" << url;
    std::cout << "\nOJ_WEB_PORT=" << port;
}

// 统一出口：写 JSON → 打印路径 → 按需展示 Web 结果页
void finalizeResultOutput(const char* argv0,
                           int argc,
                           char* argv[],
                           const json& payload) {
    const auto cfg = loadJudgeCliConfig(argc, argv, getExecutableDir(argv0));
    const auto saved = writeJsonResult(argv0, payload, cfg.username);
    std::cout << "Result json saved to: " << saved.timestamped.string();
    std::cout << "\nOJ_RESULT_JSON=" << saved.timestamped.string();
    presentResultInWeb(argv0, argc, argv, cfg, saved.timestamped, saved.timestamp_ms);
}

// CLI 评测主流程：读入源码与用例 → 调用引擎 → 组装 JSON 响应
json runJudgeCli(int argc, char* argv[]) {
    std::string program_language;
    std::string src_file;
    std::string expect_result;
    if (!parseArg(argc, argv, "program_language", program_language) ||
        !parseArg(argc, argv, "src_file", src_file) ||
        !parseArg(argc, argv, "expect_result", expect_result)) {
        throw std::runtime_error(
            "Missing required args. Usage: judge_engine.exe "
            "--program_language=... --src_file=... --expect_result=... "
            "[--web_mode=embedded|file_only|external] [--web_port=auto|PORT] "
            "[--web_host=127.0.0.1] [--open_web=1] [--web_url=...]");
    }

    const std::string code = readTextFile(src_file);

    // expect_result 为 JSON 文件，包含时限、内存限制与测试用例
    std::ifstream f(expect_result, std::ios::binary);
    if (!f.is_open()) {
        throw std::runtime_error("Failed to open expect_result file: " + expect_result);
    }
    json in = json::parse(f);

    const int time_limit_ms = in.value("time_limit_ms", 1000);
    const int memory_limit_mb = in.value("memory_limit_mb", 128);

    std::vector<TestCase> test_cases;
    // 兼容 test_cases / testcases 两种字段名
    const char* key1 = "test_cases";
    const char* key2 = "testcases";
    if (in.contains(key1)) {
        const auto& arr = in.at(key1);
        int idx = 0;
        for (const auto& tc : arr) {
            TestCase t;
            t.id = tc.value("id", idx + 1);
            t.input = tc.value("input", "");
            // 兼容 expected_output 与 output 两种期望输出字段名
            if (tc.contains("expected_output")) {
                t.expected_output = tc.value("expected_output", "");
            } else {
                t.expected_output = tc.value("output", "");
            }
            t.score = tc.value("score", 0);
            test_cases.push_back(t);
            ++idx;
        }
    } else if (in.contains(key2)) {
        const auto& arr = in.at(key2);
        int idx = 0;
        for (const auto& tc : arr) {
            TestCase t;
            t.id = tc.value("id", idx + 1);
            t.input = tc.value("input", "");
            if (tc.contains("expected_output")) {
                t.expected_output = tc.value("expected_output", "");
            } else {
                t.expected_output = tc.value("output", "");
            }
            t.score = tc.value("score", 0);
            test_cases.push_back(t);
            ++idx;
        }
    } else {
        throw std::runtime_error("expect_result json must contain test_cases");
    }

    if (test_cases.empty()) {
        throw std::runtime_error("expect_result json test_cases is empty");
    }

    JudgeEngine engine;
    // 执行编译、沙箱运行与逐用例比对
    JudgeResult result =
        engine.judge(code, program_language, test_cases, time_limit_ms, memory_limit_mb);

    // 将引擎结果映射为对外 JSON 结构
    json out;
    out["program_language"] = program_language;
    out["overall_status"] = static_cast<int>(result.overall_status);
    // verdict 为字符串缩写，便于前端直接展示
    out["verdict"] = judgeStatusToString(result.overall_status);
    out["runtime"] = result.runtime_ms;
    out["memory"] = result.memory_kb;
    out["error_message"] = result.error_message;
    out["total_score"] = result.total_score;
    out["max_score"] = result.max_score;

    // 逐用例写入判定、用时、内存与得分
    json tc_results = json::array();
    for (const auto& tc : result.test_case_results) {
        json one;
        one["case_id"] = tc.case_id;
        one["status"] = static_cast<int>(tc.status);
        one["verdict"] = judgeStatusToString(tc.status);
        one["passed"] = tc.passed;
        one["runtime"] = tc.runtime_ms;
        one["memory"] = tc.memory_kb;
        one["score"] = tc.score_awarded;
        tc_results.push_back(one);
    }
    out["test_cases"] = tc_results;
    return out;
}
} // namespace

int main(int argc, char* argv[]) {
    // --serve_web 进入内嵌 HTTP 服务模式（由父进程拉起）
    std::string serve_web;
    if (parseArg(argc, argv, "serve_web", serve_web)) {
        return runWebServeMode(argc, argv);
    }

    try {
        const json out = runJudgeCli(argc, argv);
        finalizeResultOutput(argv[0], argc, argv, out);
        return 0;
    } catch (const std::exception& e) {
        // 异常也写入 JSON，便于 Web 页面展示错误信息
        json err;
        err["error"] = "cli_error";
        err["message"] = e.what();
        try {
            finalizeResultOutput(argv[0], argc, argv, err);
        } catch (...) {
            std::cout << err.dump(2);
        }
        return 1;
    }
}
