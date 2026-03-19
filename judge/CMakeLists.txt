cmake_minimum_required(VERSION 3.10)
project(OJ_Judge)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

find_package(Threads REQUIRED)

add_executable(judge_engine
    src/judge_engine.cpp
    src/compiler.cpp
    src/sandbox.cpp
    src/result_judger.cpp
    src/submission_handler.cpp
)

target_link_libraries(judge_engine
    Threads::Threads
)

enable_testing()

add_executable(test_compiler tests/test_compiler.cpp)
target_link_libraries(test_compiler judge_engine)

add_executable(test_sandbox tests/test_sandbox.cpp)
target_link_libraries(test_sandbox judge_engine)

add_test(NAME CompilerTest COMMAND test_compiler)
add_test(NAME SandboxTest COMMAND test_sandbox)