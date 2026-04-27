#include "Game.hpp"
#include "Components.hpp"
#include <iostream>

Game::Game() : isRunning(true), gameMap(200, 150, 20), windowWidth(800), windowHeight(600), cameraX(0), cameraY(0)
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Narrative RogueLike - Massive World Update", windowWidth, windowHeight, 0);
    renderer = SDL_CreateRenderer(window, NULL);

    gameMap.generateCaves(45, 5);
    gameMap.processMap();

    playerEntity = registry.create();
    registry.emplace<Player>(playerEntity);

    int startX = 100, startY = 75;
    while (!gameMap.isFloor(startX, startY))
    {
        startX++;
        if (startX >= 200)
        {
            startX = 0;
            startY++;
            if (startY >= 150)
                startY = 0;
        }
    }
    registry.emplace<Position>(playerEntity, startX, startY);
}

Game::~Game()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Game::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
            isRunning = false;

        if (event.type == SDL_EVENT_KEY_DOWN)
        {
            auto &pos = registry.get<Position>(playerEntity);
            int nextX = pos.x;
            int nextY = pos.y;

            if (event.key.key == SDLK_SPACE)
            {
                gameMap.generateCaves(45, 5);
                gameMap.processMap();
                while (!gameMap.isFloor(pos.x, pos.y))
                {
                    pos.x++;
                    if (pos.x >= 200)
                    {
                        pos.x = 0;
                        pos.y++;
                    }
                    if (pos.y >= 150)
                    {
                        pos.y = 0;
                    }
                }
            }

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
}

void Game::update()
{
    auto &pos = registry.get<Position>(playerEntity);
    int playerPixelX = pos.x * 20;
    int playerPixelY = pos.y * 20;

    cameraX = playerPixelX - (windowWidth / 2);
    cameraY = playerPixelY - (windowHeight / 2);

    if (cameraX < 0)
        cameraX = 0;
    if (cameraY < 0)
        cameraY = 0;
    if (cameraX > (200 * 20) - windowWidth)
        cameraX = (200 * 20) - windowWidth;
    if (cameraY > (150 * 20) - windowHeight)
        cameraY = (150 * 20) - windowHeight;
}

void Game::render()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    gameMap.render(renderer, cameraX, cameraY);

    auto view = registry.view<Position>();
    for (auto entity : view)
    {
        auto &pos = view.get<Position>(entity);

        if (registry.all_of<Player>(entity))
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        }

        SDL_FRect rect = {
            static_cast<float>((pos.x * 20) - cameraX),
            static_cast<float>((pos.y * 20) - cameraY),
            20.0f, 20.0f};
        SDL_RenderFillRect(renderer, &rect);
    }

    SDL_RenderPresent(renderer);
}

void Game::run()
{
    while (isRunning)
    {
        processInput();
        update();
        render();
    }
}