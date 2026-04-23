cmake_minimum_required(VERSION 3.16)
project(oj_leaderboard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_MYSQL "Enable MySQL adapter with connector headers" OFF)
option(ENABLE_REDIS "Enable Redis adapter with redis-plus-plus headers" OFF)

add_library(leaderboard_core
    src/leaderboard/leaderboard_service.cpp
    src/api/leaderboard_api.cpp
    src/storage/in_memory_repository.cpp
    src/storage/in_memory_cache.cpp
)

target_include_directories(leaderboard_core PUBLIC include)

if(ENABLE_MYSQL)
    target_compile_definitions(leaderboard_core PUBLIC ENABLE_MYSQL=1)
    target_sources(leaderboard_core PRIVATE src/storage/mysql_repository.cpp)
endif()

if(ENABLE_REDIS)
    target_compile_definitions(leaderboard_core PUBLIC ENABLE_REDIS=1)
    target_sources(leaderboard_core PRIVATE src/storage/redis_cache.cpp)
endif()

add_executable(leaderboard_demo src/main.cpp)
target_link_libraries(leaderboard_demo PRIVATE leaderboard_core)

add_executable(leaderboard_tests tests/leaderboard_service_test.cpp)
target_link_libraries(leaderboard_tests PRIVATE leaderboard_core)

enable_testing()
add_test(NAME leaderboard_tests COMMAND leaderboard_tests)
cmake_minimum_required(VERSION 3.16)
project(oj_leaderboard LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(ENABLE_MYSQL "Enable MySQL adapter with connector headers" OFF)
option(ENABLE_REDIS "Enable Redis adapter with redis-plus-plus headers" OFF)

add_library(leaderboard_core
    src/leaderboard/leaderboard_service.cpp
    src/api/leaderboard_api.cpp
    src/storage/in_memory_repository.cpp
    src/storage/in_memory_cache.cpp
)

target_include_directories(leaderboard_core PUBLIC include)

if(ENABLE_MYSQL)
    target_compile_definitions(leaderboard_core PUBLIC ENABLE_MYSQL=1)
    target_sources(leaderboard_core PRIVATE src/storage/mysql_repository.cpp)
endif()

if(ENABLE_REDIS)
    target_compile_definitions(leaderboard_core PUBLIC ENABLE_REDIS=1)
    target_sources(leaderboard_core PRIVATE src/storage/redis_cache.cpp)
endif()

add_executable(leaderboard_demo src/main.cpp)
target_link_libraries(leaderboard_demo PRIVATE leaderboard_core)

add_executable(leaderboard_tests tests/leaderboard_service_test.cpp)
target_link_libraries(leaderboard_tests PRIVATE leaderboard_core)

enable_testing()
add_test(NAME leaderboard_tests COMMAND leaderboard_tests)
