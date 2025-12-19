cmake_minimum_required(VERSION 3.20)
project(tactics_nav LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tactics_nav
    src/GridMap.cpp
)

target_include_directories(tactics_nav PUBLIC include)

# ---- Demo ----
add_executable(tactics_demo demo/main.cpp)
target_link_libraries(tactics_demo PRIVATE tactics_nav)

# ---- Tests ----
add_executable(tactics_tests tests/test_path.cpp)
target_link_libraries(tactics_tests PRIVATE tactics_nav)
