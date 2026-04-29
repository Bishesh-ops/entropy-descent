cmake_minimum_required(VERSION 3.31)
project(NarrativeRoguelike VERSION 1.0)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# --- Dependencies ---
find_package(SDL3 REQUIRED)
find_package(Lua REQUIRED)

include(FetchContent)

# Fetch EnTT
FetchContent_Declare(
    entt
    GIT_REPOSITORY https://github.com/skypjack/entt
    GIT_TAG        v3.13.2 
)
set(ENTT_BUILD_TESTING OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(entt)

# Fetch nlohmann/json
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3 
)
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.3.1 
)


set(JSON_BuildTests OFF CACHE BOOL "" FORCE) 
FetchContent_MakeAvailable(json)
FetchContent_MakeAvailable(sol2)

add_executable(Roguelike 
    src/main.cpp 
    src/Game.cpp 
    src/DataLoader.cpp 
    src/UIRenderer.cpp
    src/WorldBuilder.cpp
    
    # States
    src/States/MenuState.cpp 
    src/States/PlayState.cpp 
    src/States/VowState.cpp
    
    # Systems
    src/Systems/RenderSystem.cpp
    src/Systems/CombatSystem.cpp
    src/Systems/SpellSystem.cpp
    src/Systems/ItemSystem.cpp
    src/Systems/AISystem.cpp
)
target_include_directories(Roguelike PRIVATE include ${LUA_INCLUDE_DIR})

# Link SDL3, EnTT, and JSON
target_link_libraries(Roguelike PRIVATE SDL3::SDL3 EnTT::EnTT nlohmann_json::nlohmann_json sol2::sol2 ${LUA_LIBRARIES})