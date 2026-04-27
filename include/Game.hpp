#pragma once
#include <SDL3/SDL.h>
#include "StateMachine.hpp"

class Game
{
public:
    Game();
    ~Game();
    void run();
    void quit() { isRunning = false; }

    SDL_Renderer *getRenderer() const { return renderer; }
    StateMachine &getStateMachine() { return stateMachine; }
    int getWindowWidth() const { return windowWidth; }
    int getWindowHeight() const { return windowHeight; }

private:
    SDL_Window *window = nullptr;
    SDL_Renderer *renderer = nullptr;
    bool isRunning;

    StateMachine stateMachine;

    int windowWidth;
    int windowHeight;
};