if(NOT DEFINED OJ_SUBPROJECT_BUILD)
    set(OJ_SUBPROJECT_BUILD OFF)
endif()

option(OJ_RANK_ENABLE_MYSQL "Enable MySQL persistence via libmariadbclient C API" OFF)

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
    src/storage/mysql_c_wrapper.cpp
    src/storage/mysql_repository.cpp
    src/storage/repository_factory.cpp
)

target_include_directories(leaderboard_core PUBLIC include)
target_link_libraries(leaderboard_core PUBLIC oj_common Threads::Threads)

if(OJ_RANK_ENABLE_MYSQL)
    set(MYSQL_ROOT_DIR "C:/msys64/ucrt64" CACHE PATH "MariaDB/MySQL client install root")
    find_path(MYSQL_CLIENT_INCLUDE_DIR
        NAMES mysql/mysql.h
        PATHS ${MYSQL_ROOT_DIR}/include
        PATH_SUFFIXES mysql
        NO_DEFAULT_PATH
    )
    if(NOT MYSQL_CLIENT_INCLUDE_DIR)
        find_path(MYSQL_CLIENT_INCLUDE_DIR
            NAMES mysql/mysql.h
            PATHS ${MYSQL_ROOT_DIR}/include
        )
    endif()
    find_library(MYSQL_CLIENT_LIB
        NAMES libmysqlclient mysqlclient
        PATHS ${MYSQL_ROOT_DIR}/lib
    )
    if(MYSQL_CLIENT_INCLUDE_DIR AND MYSQL_CLIENT_LIB)
        target_compile_definitions(leaderboard_core PUBLIC OJ_WITH_MYSQL)
        target_include_directories(leaderboard_core PUBLIC ${MYSQL_ROOT_DIR}/include)
        target_link_libraries(leaderboard_core PRIVATE ${MYSQL_CLIENT_LIB})
        message(STATUS "rank MySQL: include=${MYSQL_ROOT_DIR}/include  lib=${MYSQL_CLIENT_LIB}")
    else()
        message(WARNING "rank MySQL: libmysqlclient not found. Install mingw-w64-ucrt-x86_64-libmariadbclient")
        message(WARNING "  Expected: ${MYSQL_ROOT_DIR}/include/mysql/mysql.h")
        message(WARNING "  Expected: ${MYSQL_ROOT_DIR}/lib/libmysqlclient.a")
    endif()
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
