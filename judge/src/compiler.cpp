#include "compiler.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <sstream>
#include <filesystem>
#include <cctype>
#include <vector>
#ifndef _WIN32
#include <unistd.h>
#endif

namespace fs = std::filesystem;

namespace {
std::string guessJavaPublicClassName(const std::string& source_code) {
    const std::string key = "public class";
    const auto pos = source_code.find(key);
    if (pos == std::string::npos) return "Main";

    size_t i = pos + key.size();
    while (i < source_code.size() && std::isspace(static_cast<unsigned char>(source_code[i]))) ++i;

    std::string name;
    while (i < source_code.size()) {
        const unsigned char c = static_cast<unsigned char>(source_code[i]);
        if (std::isalnum(c) || c == '_' || c == '$') {
            name.push_back(static_cast<char>(c));
            ++i;
            continue;
        }
        break;
    }

    return name.empty() ? "Main" : name;
}
}

// 通过 shell 执行编译命令，捕获 stdout/stderr 与退出码
CompileResult executeCommand(const std::string& command) {
    CompileResult result;
    result.success = false;
    result.exit_code = -1;
    
    // 合并 stderr 到 stdout，便于统一展示编译错误
    std::string cmd = command + " 2>&1";
#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) {
        result.error = "Failed to open pipe";
        result.output = "Command: " + cmd;
        return result;
    }
    
    // 逐块读取编译器输出直至管道关闭
    char buffer[1024];
    std::string output;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) output += buffer;
    
#ifdef _WIN32
    result.exit_code = _pclose(pipe);
#else
    result.exit_code = pclose(pipe);
#endif
    // 退出码 0 视为编译成功
    result.success = (result.exit_code == 0);
    result.output = "Command: " + cmd + "\nOutput: " + output;
    if (!result.success) result.error = result.output;
    
    return result;
}

// 将源码写入临时文件，供 gcc/g++/javac 等工具读取
std::string saveToTempFile(const std::string& code, const std::string& extension) {
    std::string temp_file;

#ifdef _WIN32
    // Windows：使用 tmpnam_s 或递增计数器生成唯一文件名
    char temp_name[L_tmpnam_s];
    errno_t err = tmpnam_s(temp_name, L_tmpnam_s);
    if (err != 0) {
        static int counter = 0;
        temp_file = "temp_" + std::to_string(counter++) + extension;
    } else {
        temp_file = std::string(temp_name) + extension;
    }
#else
    // Linux：mkstemp 创建安全临时路径
    std::string tmpl = "/tmp/oj_XXXXXX";
    std::vector<char> buf(tmpl.begin(), tmpl.end());
    buf.push_back('\0');
    int fd = mkstemp(buf.data());
    if (fd >= 0) {
        ::close(fd);
        temp_file = std::string(buf.data()) + extension;
        ::unlink(buf.data());
    } else {
        static int counter = 0;
        temp_file = "/tmp/oj_temp_" + std::to_string(counter++) + extension;
    }
#endif

    // 写入源码内容到临时文件
    std::ofstream file(temp_file);
    if (file.is_open()) {
        file << code;
        file.close();
    }
    return temp_file;
}

// C 语言：gcc 编译为可执行文件
CompileResult CCompiler::compile(const std::string& source_code, const std::string& output_path) {
    std::string temp_file = saveToTempFile(source_code, ".c");
    std::string command = "gcc -std=c11 -O2 -o " + output_path + " " + temp_file;
    
    CompileResult result = executeCommand(command);
    
    // 编译后立即删除临时源文件
    remove(temp_file.c_str());
    
    return result;
}

// C++：g++ 编译，标准 C++17
CompileResult CppCompiler::compile(const std::string& source_code, const std::string& output_path) {
    std::string temp_file = saveToTempFile(source_code, ".cpp");
    std::string command = "g++ -std=c++17 -O2 -o " + output_path + " " + temp_file;
    
    CompileResult result = executeCommand(command);
    
    remove(temp_file.c_str());
    
    return result;
}

// Java：写入 .java 源文件后调用 javac 输出到目录
CompileResult JavaCompiler::compile(const std::string& source_code, const std::string& output_path) {
    const std::string class_name = guessJavaPublicClassName(source_code);
    const fs::path out_dir = fs::path(output_path);

    std::error_code ec;
    fs::create_directories(out_dir, ec);

    const fs::path java_file = out_dir / (class_name + ".java");

    std::ofstream file(java_file, std::ios::binary);
    if (!file.is_open()) {
        CompileResult r;
        r.success = false;
        r.exit_code = -1;
        r.error = "Failed to write java source file: " + java_file.string();
        return r;
    }
    file << source_code;
    file.close();

    std::string command = "javac -encoding UTF-8 -d \"" + out_dir.string() + "\" \"" + java_file.string() + "\"";
    return executeCommand(command);
}

// Python 无需编译，直接将源码保存为 .py 脚本
// Python 无需编译，直接将源码保存为 .py 脚本
CompileResult PythonCompiler::compile(const std::string& source_code, const std::string& output_path) {
    CompileResult result;
    result.success = true;
    result.exit_code = 0;
    result.output = "Python code does not require compilation";
    
    std::ofstream file(output_path);
    if (file.is_open()) {
        file << source_code;
        file.close();
    }
    
    return result;
}
