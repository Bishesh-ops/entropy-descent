#include <SDL3/SDL.h>
#include <iostream>
#include <entt/entt.hpp>
#include "Map.hpp"
#include "Components.hpp"

int main(int argc, char *argv[])
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        return 1;

    SDL_Window *window = SDL_CreateWindow("Narrative Roguelike - ECS Spawn", 800, 600, 0);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    Map gameMap(80, 60, 10);
    gameMap.generateCaves(45, 5);

    entt::registry registry;

    // Create the Player Entity
    auto playerEntity = registry.create();
    registry.emplace<Player>(playerEntity); // Tag it as the player

    // Find an empty floor tile to spawn the player on
    int startX = 40, startY = 30;
    while (!gameMap.isFloor(startX, startY))
    {
        startX++; // Bruteforce find an empty spot for now
    }
    registry.emplace<Position>(playerEntity, startX, startY);

    bool running = true;
    SDL_Event event;

    while (running)
    {
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            if (event.type == SDL_EVENT_KEY_DOWN)
            {

                auto &pos = registry.get<Position>(playerEntity);
                int nextX = pos.x;
                int nextY = pos.y;

                if (event.key.key == SDLK_SPACE)
                    gameMap.generateCaves(45, 5);

                if (event.key.key == SDLK_W || event.key.key == SDLK_UP)
                    nextY--;
                if (event.key.key == SDLK_S || event.key.key == SDLK_DOWN)
                    nextY++;
                if (event.key.key == SDLK_A || event.key.key == SDLK_LEFT)
                    nextX--;
                if (event.key.key == SDLK_D || event.key.key == SDLK_RIGHT)
                    nextX++;
                if (gameMap.isFloor(nextX, nextY))
                {
                    pos.x = nextX;
                    pos.y = nextY;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderClear(renderer);

        gameMap.render(renderer);
        auto view = registry.view<Position>();
        for (auto entity : view)
        {
            auto &pos = view.get<Position>(entity);

            // If the entity also has a Player tag, draw them as a cyan square
            if (registry.all_of<Player>(entity))
            {
                SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red for enemies later
            }

            SDL_FRect rect = {
                static_cast<float>(pos.x * 10),
                static_cast<float>(pos.y * 10),
                10.0f, 10.0f};
            SDL_RenderFillRect(renderer, &rect);
        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}