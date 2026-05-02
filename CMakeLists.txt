cmake_minimum_required(VERSION 3.31)
project(NarrativeRoguelike VERSION 1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

# --- Define ImGui Directory ---
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/third_party/imgui)

# --- Dependencies ---
find_package(SDL3 REQUIRED)
find_package(Lua 5.4 EXACT REQUIRED)

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
set(JSON_BuildTests OFF CACHE BOOL "" FORCE) 
FetchContent_MakeAvailable(json)

# Fetch sol2
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG        v3.3.1 
)
FetchContent_MakeAvailable(sol2)

# --- Gather ImGui Source Files ---
file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/*.cpp"
)

# --- Build Executable ---
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
    src/Systems/MovementSystem.cpp

    # ImGui
    ${IMGUI_SOURCES}
)

target_include_directories(Roguelike PRIVATE include ${LUA_INCLUDE_DIR} ${IMGUI_DIR})

target_link_libraries(Roguelike PRIVATE SDL3::SDL3 EnTT::EnTT nlohmann_json::nlohmann_json sol2::sol2 ${LUA_LIBRARIES})