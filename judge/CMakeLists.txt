if(NOT DEFINED OJ_SUBPROJECT_BUILD)
    set(OJ_SUBPROJECT_BUILD OFF)
endif()

if(NOT OJ_SUBPROJECT_BUILD)
    cmake_minimum_required(VERSION 3.14)
    project(OJ_Judge LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    find_package(Threads REQUIRED)
endif()

find_package(Threads REQUIRED)

add_library(judge_core
    src/judge_engine.cpp
    src/compiler.cpp
    src/sandbox.cpp
    src/result_judger.cpp
    src/language_config.cpp
)

target_include_directories(judge_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include/asio
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
