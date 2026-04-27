cmake_minimum_required(VERSION 3.31)
project(NarrativeRoguelike VERSION 1.0)

# Pushing the standard to the latest stable edge
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find the SDL3 library
find_package(SDL3 REQUIRED)

# 1. Point to the new location of main.cpp
add_executable(Roguelike src/main.cpp)

# 2. Tell the compiler where the header files live
target_include_directories(Roguelike PRIVATE include)

# Link SDL3
target_link_libraries(Roguelike PRIVATE SDL3::SDL3)