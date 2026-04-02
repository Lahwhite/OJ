#include "compiler.h"
#include <iostream>

int main() {
    std::cout << "Testing C compiler..." << std::endl;
    CCompiler c_compiler;
    std::string c_code = "#include <stdio.h>\nint main() { printf(\"Hello\"); return 0; }";
    CompileResult c_result = c_compiler.compile(c_code, "test_c.exe");
    std::cout << "C compile success: " << (c_result.success ? "Yes" : "No") << std::endl;
    if (!c_result.success) {
        std::cout << "Error: " << c_result.output << std::endl;
    }
    
    std::cout << "Testing C++ compiler..." << std::endl;
    CppCompiler cpp_compiler;
    std::string cpp_code = "#include <iostream>\nint main() { std::cout << \"Hello\" << std::endl; return 0; }";
    CompileResult cpp_result = cpp_compiler.compile(cpp_code, "test_cpp.exe");
    std::cout << "C++ compile success: " << (cpp_result.success ? "Yes" : "No") << std::endl;
    if (!cpp_result.success) {
        std::cout << "Error: " << cpp_result.output << std::endl;
    }
    
    std::cout << "Testing Java compiler..." << std::endl;
    JavaCompiler java_compiler;
    std::string java_code = "public class Test { public static void main(String[] args) { System.out.println(\"Hello\"); } }";
    CompileResult java_result = java_compiler.compile(java_code, "Test.java");
    std::cout << "Java compile success: " << (java_result.success ? "Yes" : "No") << std::endl;
    if (!java_result.success) {
        std::cout << "Error: " << java_result.output << std::endl;
    }
    
    std::cout << "Testing Python compiler..." << std::endl;
    PythonCompiler python_compiler;
    std::string python_code = "print('Hello')";
    CompileResult python_result = python_compiler.compile(python_code, "test.py");
    std::cout << "Python compile success: " << (python_result.success ? "Yes" : "No") << std::endl;
    if (!python_result.success) {
        std::cout << "Error: " << python_result.output << std::endl;
    }
    
    return 0;
}