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

# ── Locate Python interpreter (python3 preferred, fall back to python) ────────
find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
    set(OJ_PYTHON_EXECUTABLE "${Python3_EXECUTABLE}")
else()
    # Fallback: look for python3 then python on PATH
    find_program(OJ_PYTHON3_EXE NAMES python3 python)
    if(OJ_PYTHON3_EXE)
        set(OJ_PYTHON_EXECUTABLE "${OJ_PYTHON3_EXE}")
    else()
        message(FATAL_ERROR
            "Python 3 interpreter not found. "
            "Install Python 3 and ensure 'python3' or 'python' is on PATH.")
    endif()
endif()
message(STATUS "OJ: Using Python interpreter: ${OJ_PYTHON_EXECUTABLE}")

# ── Embed web/ static files into a C++ translation unit ──────────────────────
set(OJ_WEB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../web")
set(OJ_WEB_ASSETS_CPP "${CMAKE_CURRENT_SOURCE_DIR}/src/web_assets.cpp")

add_custom_command(
    OUTPUT "${OJ_WEB_ASSETS_CPP}"
    COMMAND ${CMAKE_COMMAND} -E env PYTHONIOENCODING=utf-8
            "${OJ_PYTHON_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_web_assets.py"
            "${OJ_WEB_DIR}"
            "${OJ_WEB_ASSETS_CPP}"
    DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/tools/embed_web_assets.py"
        "${OJ_WEB_DIR}/index.html"
        "${OJ_WEB_DIR}/css/style.css"
        "${OJ_WEB_DIR}/js/app.js"
    COMMENT "Embedding web assets into judge_engine"
    VERBATIM
)

# ── Core library (evaluator logic, no web/CLI code) ──────────────────────────
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

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(judge_engine
    src/main.cpp
    src/judge_cli_config.cpp
    src/web_server.cpp
    "${OJ_WEB_ASSETS_CPP}"
)

target_include_directories(judge_engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(judge_engine PRIVATE judge_core)
if(WIN32)
    target_link_libraries(judge_engine PRIVATE ws2_32 shell32)
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()

add_executable(test_compiler tests/part/test_compiler.cpp)
target_link_libraries(test_compiler PRIVATE judge_core)

add_executable(test_sandbox tests/part/test_sandbox.cpp)
target_link_libraries(test_sandbox PRIVATE judge_core)

add_test(NAME CompilerTest COMMAND test_compiler)
add_test(NAME SandboxTest COMMAND test_sandbox)
