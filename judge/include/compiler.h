/**
 * @file compiler.h
 * @brief 编译器接口定义
 * @author OJ Team
 * @date 2024-01-01
 */
#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include <vector>

/**
 * @struct CompileResult
 * @brief 编译结果结构体
 */
struct CompileResult {
    bool success;        ///< 编译是否成功
    std::string output;  ///< 编译输出
    std::string error;   ///< 编译错误信息
    int exit_code;       ///< 编译退出码
};

/**
 * @class Compiler
 * @brief 编译器基类
 */
class Compiler {
public:
    virtual ~Compiler() = default;
    
    /**
     * @brief 编译代码
     * @param source_code 源代码
     * @param output_path 输出文件路径
     * @return 编译结果
     */
    virtual CompileResult compile(const std::string& source_code, 
                            const std::string& output_path) = 0;
};

/**
 * @class CCompiler
 * @brief C语言编译器
 */
class CCompiler : public Compiler {
public:
    /**
     * @brief 编译C代码
     * @param source_code C源代码
     * @param output_path 输出文件路径
     * @return 编译结果
     */
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

/**
 * @class CppCompiler
 * @brief C++语言编译器
 */
class CppCompiler : public Compiler {
public:
    /**
     * @brief 编译C++代码
     * @param source_code C++源代码
     * @param output_path 输出文件路径
     * @return 编译结果
     */
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

/**
 * @class JavaCompiler
 * @brief Java语言编译器
 */
class JavaCompiler : public Compiler {
public:
    /**
     * @brief 编译Java代码
     * @param source_code Java源代码
     * @param output_path 输出文件路径
     * @return 编译结果
     */
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

/**
 * @class PythonCompiler
 * @brief Python语言编译器
 */
class PythonCompiler : public Compiler {
public:
    /**
     * @brief 编译Python代码
     * @param source_code Python源代码
     * @param output_path 输出文件路径
     * @return 编译结果
     */
    CompileResult compile(const std::string& source_code, 
                     const std::string& output_path) override;
};

#endif // COMPILER_H