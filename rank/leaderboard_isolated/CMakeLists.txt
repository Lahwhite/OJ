cmake_minimum_required(VERSION 3.16)
project(leaderboard_isolated LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(leaderboard_isolated_core
    src/leaderboard.cpp
)
target_include_directories(leaderboard_isolated_core PUBLIC include)

add_executable(leaderboard_isolated_demo src/demo.cpp)
target_link_libraries(leaderboard_isolated_demo PRIVATE leaderboard_isolated_core)

add_executable(leaderboard_isolated_tests tests/leaderboard_test.cpp)
target_link_libraries(leaderboard_isolated_tests PRIVATE leaderboard_isolated_core)

enable_testing()
add_test(NAME leaderboard_isolated_tests COMMAND leaderboard_isolated_tests)
