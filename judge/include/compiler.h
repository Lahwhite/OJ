#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>

// 编译阶段的结果：成功与否、输出日志与退出码
struct CompileResult {
    bool success;
    std::string output;
    std::string error;
    int exit_code;
};

// 各语言编译器的抽象基类
class Compiler {
public:
    virtual ~Compiler() = default;
    
    virtual CompileResult compile(const std::string& source_code, 
                            const std::string& output_path) = 0;
};

// C 语言编译器（gcc）
class CCompiler : public Compiler {
public:
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

// C++ 编译器（g++）
class CppCompiler : public Compiler {
public:
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

// Java 编译器（javac）
class JavaCompiler : public Compiler {
public:
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

// Python 解释型语言，compile 仅保存源码文件
class PythonCompiler : public Compiler {
public:
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

#endif // COMPILER_H
