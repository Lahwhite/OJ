#include "judge_engine.h"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <iostream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace {
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

std::string readTextFile(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

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

std::filesystem::path writeJsonResultToExeDir(const char* argv0, const json& payload) {
    const auto exe_dir = getExecutableDir(argv0);
    const auto ts = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();
    const auto out_path = exe_dir / ("judge_result_" + std::to_string(ts) + ".json");

    std::ofstream out(out_path, std::ios::binary);
    if (!out.is_open()) {
        throw std::runtime_error("Failed to write result file: " + out_path.string());
    }
    out << payload.dump(2);
    out.close();
    return out_path;
}
} // namespace

int main(int argc, char* argv[]) {
    // CLI 模式：a.exe --program_language=... --src_file=... --expect_result=...
    std::string program_language;
    std::string src_file;
    std::string expect_result;
    if (parseArg(argc, argv, "program_language", program_language) &&
        parseArg(argc, argv, "src_file", src_file) && parseArg(argc, argv, "expect_result", expect_result)) {
        try {
            std::string code = readTextFile(src_file);

            std::ifstream f(expect_result, std::ios::binary);
            if (!f.is_open()) {
                throw std::runtime_error("Failed to open expect_result file: " + expect_result);
            }
            json in = json::parse(f);

            int time_limit_ms = in.value("time_limit_ms", 1000);
            int memory_limit_mb = in.value("memory_limit_mb", 128);

            std::vector<TestCase> test_cases;
            const char* key1 = "test_cases";
            const char* key2 = "testcases";
            if (in.contains(key1)) {
                const auto& arr = in.at(key1);
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
            JudgeResult result = engine.judge(code, program_language, test_cases, time_limit_ms, memory_limit_mb);

            json out;
            out["program_language"] = program_language;
            out["overall_status"] = static_cast<int>(result.overall_status);
            out["verdict"] = judgeStatusToString(result.overall_status);
            out["runtime"] = result.runtime_ms;
            out["memory"] = result.memory_kb;
            out["error_message"] = result.error_message;
            out["total_score"] = result.total_score;
            out["max_score"] = result.max_score;

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
                // 如果你希望输出/预期也返回，可按需打开：
                // one["output"] = tc.output;
                // one["expected_output"] = tc.expected_output;
                tc_results.push_back(one);
            }
            out["test_cases"] = tc_results;

            const auto saved = writeJsonResultToExeDir(argv[0], out);
            std::cout << "Result json saved to: " << saved.string();
            return 0;
        } catch (const std::exception& e) {
            json err;
            err["error"] = "cli_error";
            err["message"] = e.what();
            try {
                const auto saved = writeJsonResultToExeDir(argv[0], err);
                std::cout << "Error json saved to: " << saved.string();
            } catch (...) {
                std::cout << err.dump(2);
            }
            return 1;
        }
    }

    json err;
    err["error"] = "cli_error";
    err["message"] =
        "Missing required args. Usage: a.exe --program_language=... --src_file=... --expect_result=...";
    try {
        const auto saved = writeJsonResultToExeDir(argv[0], err);
        std::cout << "Error json saved to: " << saved.string();
    } catch (...) {
        std::cout << err.dump(2);
    }
    return 1;
}
