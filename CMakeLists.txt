cmake_minimum_required(VERSION 3.31)
project(NarrativeRoguelike VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# --- Dependencies ---
find_package(SDL3 REQUIRED)

include(FetchContent)
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG        v3.13.2 # Latest stable branch
)
FetchContent_MakeAvailable(entt)

add_executable(Roguelike src/main.cpp src/Game.cpp src/States/MenuState.cpp src/States/PlayState.cpp)
target_include_directories(Roguelike PRIVATE include)

target_link_libraries(Roguelike PRIVATE SDL3::SDL3 EnTT::EnTT)