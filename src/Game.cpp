#include "../include/Game.hpp"
#include "../include/States/MenuState.hpp"
#include <iostream>

Game::Game() : isRunning(true), windowWidth(800), windowHeight(600)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Narrative Roguelike", windowWidth, windowHeight, 0);
    renderer = SDL_CreateRenderer(window, NULL);

    // Boot up the game by throwing the Menu on the stack
    stateMachine.pushState(std::make_unique<MenuState>(*this));
}

Game::~Game()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::run()
{
    Uint64 lastTime = SDL_GetTicks();

    while (isRunning)
    {
        Uint64 currentTime = SDL_GetTicks();
        float dt = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        if (dt > 0.05f)
            dt = 0.05f;
        stateMachine.processStateChanges();

        if (!stateMachine.isEmpty())
        {
            stateMachine.getActiveState().processInput();
            stateMachine.getActiveState().update(dt);
            stateMachine.getActiveState().render();
        }
        else
        {
            isRunning = false;
        }
    }
}