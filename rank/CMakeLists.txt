if(NOT DEFINED OJ_SUBPROJECT_BUILD)
    set(OJ_SUBPROJECT_BUILD OFF)
endif()

option(OJ_ENABLE_MYSQL "Enable MySQL persistence via common mysql pool" OFF)

if(NOT OJ_SUBPROJECT_BUILD)
    cmake_minimum_required(VERSION 3.16)
    project(oj_leaderboard LANGUAGES CXX)
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)
    find_package(Threads REQUIRED)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../common ${CMAKE_BINARY_DIR}/oj_common_standalone)
endif()

find_package(Threads REQUIRED)

option(ENABLE_REDIS "Enable Redis adapter with redis-plus-plus headers" OFF)

add_library(leaderboard_core
    src/leaderboard/leaderboard_service.cpp
    src/leaderboard/stats_analyzer.cpp
    src/api/leaderboard_api.cpp
    src/storage/in_memory_repository.cpp
    src/storage/in_memory_cache.cpp
    src/storage/mysql_repository.cpp
    src/storage/repository_factory.cpp
)

target_include_directories(leaderboard_core PUBLIC include)
target_link_libraries(leaderboard_core PUBLIC oj_common Threads::Threads)

if(OJ_ENABLE_MYSQL AND DEFINED MYSQL_CONCPP_LIB)
    target_link_libraries(leaderboard_core PRIVATE ${MYSQL_CONCPP_LIB})
endif()

if(ENABLE_REDIS)
    target_compile_definitions(leaderboard_core PUBLIC ENABLE_REDIS=1)
    target_sources(leaderboard_core PRIVATE src/storage/redis_cache.cpp)
endif()

add_library(leaderboard_http
    src/leaderboard_handler.cpp
)

target_include_directories(leaderboard_http PUBLIC include)

target_compile_definitions(leaderboard_http PUBLIC
    RANK_WEB_DIR="${CMAKE_CURRENT_SOURCE_DIR}/web"
)

target_link_libraries(leaderboard_http PUBLIC leaderboard_core Threads::Threads)

if(WIN32)
    target_link_libraries(leaderboard_http PUBLIC ws2_32)
endif()

add_executable(leaderboard_demo src/main.cpp)
target_link_libraries(leaderboard_demo PRIVATE leaderboard_core)

add_executable(leaderboard_server src/server_main.cpp)
target_link_libraries(leaderboard_server PRIVATE leaderboard_http)

add_executable(leaderboard_tests tests/leaderboard_service_test.cpp)
target_link_libraries(leaderboard_tests PRIVATE leaderboard_core)

add_executable(leaderboard_extended_tests tests/leaderboard_extended_test.cpp)
target_link_libraries(leaderboard_extended_tests PRIVATE leaderboard_core)

add_executable(leaderboard_mysql_tests tests/leaderboard_mysql_repository_test.cpp)
target_link_libraries(leaderboard_mysql_tests PRIVATE leaderboard_core)

enable_testing()
add_test(NAME leaderboard_tests COMMAND leaderboard_tests)
add_test(NAME leaderboard_extended_tests COMMAND leaderboard_extended_tests)
add_test(NAME leaderboard_mysql_tests COMMAND leaderboard_mysql_tests)
