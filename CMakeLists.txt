cmake_minimum_required(VERSION 3.31)
project(NarrativeRoguelike VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(SDL3 REQUIRED)

add_executable(Roguelike main.cpp)

target_link_libraries(Roguelike PRIVATE SDL3::SDL3)