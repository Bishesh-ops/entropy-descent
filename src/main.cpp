#include <SDL3/SDL.h>
#include <iostream>
#include "Map.hpp"

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    SDL_Window *window = SDL_CreateWindow("Narrative Roguelike - Map Gen", 800, 600, 0);
    if (!window)
        return 1;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer)
        return 1;

    Map gameMap(80, 60, 10);

    gameMap.generateCaves(45, 5);

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                running = false;
            }

            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_SPACE)
            {
                gameMap.generateCaves(45, 5);
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        gameMap.render(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}