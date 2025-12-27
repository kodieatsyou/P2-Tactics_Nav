include(FetchContent)

cmake_minimum_required(VERSION 3.20)
project(tactics_nav LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(tactics_nav
    src/GridMap.cpp
    src/DynamicOccupancy.cpp
    src/Pathfinding.cpp
    src/Reachability.cpp
    src/LOS.cpp
    src/InfluenceField.cpp
)

target_include_directories(tactics_nav PUBLIC include)

# ---- Demo ----
add_executable(tactics_demo 
    demo/main.cpp
)
target_link_libraries(tactics_demo PRIVATE tactics_nav)

find_package(SDL2 REQUIRED)
target_include_directories(tactics_demo PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(tactics_demo PRIVATE ${SDL2_LIBRARIES})

include(FetchContent)
FetchContent_Declare(
  imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui.git
  GIT_TAG        v1.91.2
)
FetchContent_MakeAvailable(imgui)

target_sources(tactics_demo PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

target_include_directories(tactics_demo PRIVATE
  ${imgui_SOURCE_DIR}
  ${imgui_SOURCE_DIR}/backends
)

# SDL2 (system)
find_package(SDL2 REQUIRED)
target_include_directories(tactics_demo PRIVATE ${SDL2_INCLUDE_DIRS})
target_link_libraries(tactics_demo PRIVATE ${SDL2_LIBRARIES})

# ---- Tests ----
add_executable(tactics_tests tests/test_path.cpp)
target_link_libraries(tactics_tests PRIVATE tactics_nav)
