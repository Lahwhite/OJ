if(NOT DEFINED OJ_SUBPROJECT_BUILD)
    # 允许 discussion 既作为总项目子模块编译，也可以单独打开目录编译。
    set(OJ_SUBPROJECT_BUILD OFF)
endif()

if(NOT OJ_SUBPROJECT_BUILD)
    # 单独编译时需要自己声明项目和 C++ 标准，总项目编译时由根 CMake 负责。
    cmake_minimum_required(VERSION 3.14)
    project(OJ_Discussion LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    find_package(Threads REQUIRED)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common ${CMAKE_BINARY_DIR}/oj_common_standalone)
endif()

find_package(Threads REQUIRED)

# discussion_core 包含业务逻辑、HTTP 路由和 Gemini 客户端，测试和服务端可执行文件共用它。
add_library(discussion_core
    src/discussion_service.cpp
    src/discussion_handler.cpp
    src/gemini_client.cpp
)

# include 中放本模块头文件，judge/include 提供 Crow/asio 等已有依赖。
target_include_directories(discussion_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../judge/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../judge/include/asio
)

# 编译期写入源码 web 目录，源码运行时无需复制静态资源。
target_compile_definitions(discussion_core PUBLIC
    DISCUSSION_WEB_DIR="${CMAKE_CURRENT_SOURCE_DIR}/web"
)

# 公共模块提供配置、日志、MySQL 连接池和错误 JSON。
target_link_libraries(discussion_core PUBLIC oj_common Threads::Threads)

if(WIN32)
    # Windows 下 Crow 需要 Winsock，Gemini 摘要请求使用 WinINet。
    target_link_libraries(discussion_core PUBLIC ws2_32 mswsock wininet)
endif()

# 讨论区 HTTP 服务入口，运行包中的 discussion_server.exe 由它生成。
add_executable(discussion_server src/main.cpp)
target_link_libraries(discussion_server PRIVATE discussion_core)

enable_testing()

# 测试既覆盖无数据库时的兜底行为，也可通过环境变量开启数据库集成测试。
add_executable(test_discussion_service tests/test_discussion_service.cpp)
target_link_libraries(test_discussion_service PRIVATE discussion_core)

# CTest 名称保持独立，方便总项目里只跑讨论区测试。
add_test(NAME DiscussionServiceTest COMMAND test_discussion_service)
