#ifndef DEFAULT_LANGUAGE_CONFIG_H
#define DEFAULT_LANGUAGE_CONFIG_H

// 内置默认语言配置。用于找不到外部 config/languages.json 时的回退加载。
inline constexpr const char* kDefaultLanguageConfigJson = R"({
  "languages": {
    "c": {
      "id": "c",
      "name": "C",
      "version": "C11",
      "compiler": "gcc",
      "compile_command": "gcc -std=c11 -O2 -o {output} {source}",
      "source_extension": ".c",
      "output_extension": ".out",
      "execute_command": "./{output}",
      "time_limit_multiplier": 1.0,
      "memory_limit_multiplier": 1.0
    },
    "cpp": {
      "id": "cpp",
      "name": "C++",
      "version": "C++17",
      "compiler": "g++",
      "compile_command": "g++ -std=c++17 -O2 -o {output} {source}",
      "source_extension": ".cpp",
      "output_extension": ".out",
      "execute_command": "./{output}",
      "time_limit_multiplier": 1.0,
      "memory_limit_multiplier": 1.0
    },
    "java": {
      "id": "java",
      "name": "Java",
      "version": "Java 17",
      "compiler": "javac",
      "compile_command": "javac {source}",
      "source_extension": ".java",
      "output_extension": ".class",
      "execute_command": "java {filename}",
      "time_limit_multiplier": 1.5,
      "memory_limit_multiplier": 1.5
    },
    "python": {
      "id": "python",
      "name": "Python",
      "version": "Python 3.10",
      "compiler": "python3",
      "compile_command": "",
      "source_extension": ".py",
      "output_extension": "",
      "execute_command": "python3 {source}",
      "time_limit_multiplier": 2.0,
      "memory_limit_multiplier": 1.5
    }
  }
})";

#endif // DEFAULT_LANGUAGE_CONFIG_H
