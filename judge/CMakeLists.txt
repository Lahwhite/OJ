cmake_minimum_required(VERSION 3.14)
project(OJ_Judge LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# 直接包含common模块的源文件
add_library(judge_core
    src/judge_engine.cpp
    src/compiler.cpp
    src/sandbox.cpp
    src/result_judger.cpp
    src/submission_handler.cpp
    src/language_config.cpp
    ../common/src/log.cpp
    ../common/src/config.cpp
    ../common/src/mysql_pool.cpp
    ../common/src/redis_cache.cpp
    ../common/src/json_error.cpp
    ../common/src/bootstrap.cpp
)

target_include_directories(judge_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/asio
    ${CMAKE_CURRENT_LIST_DIR}/../common/include
)

target_link_libraries(judge_core PUBLIC Threads::Threads)

add_executable(judge_engine src/main.cpp)
target_link_libraries(judge_engine PRIVATE judge_core)

enable_testing()

add_executable(test_compiler tests/test_compiler.cpp)
target_link_libraries(test_compiler PRIVATE judge_core)

add_executable(test_sandbox tests/test_sandbox.cpp)
target_link_libraries(test_sandbox PRIVATE judge_core)

add_test(NAME CompilerTest COMMAND test_compiler)
add_test(NAME SandboxTest COMMAND test_sandbox)
