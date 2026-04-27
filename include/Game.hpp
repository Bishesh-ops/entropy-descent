#pragma once
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "Map.hpp"

class Game
{
public:
    Game();
    ~Game();
    void run();

private:
    void processInput();
    void update();
    void render();

    SDL_Window *window;
    SDL_Renderer *renderer;
    bool isRunning;
    entt::registry registry;
    entt::entity playerEntity;
    Map gameMap;

    int cameraX;
    int cameraY;
    int windowWidth;
    int windowHeight;
};