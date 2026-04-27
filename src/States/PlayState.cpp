#include "../../include/States/PlayState.hpp"
#include "../../include/Components.hpp"
#include <iostream>

PlayState::PlayState(Game &gameRef)
    : game(gameRef), gameMap(200, 150, 20), cameraX(0), cameraY(0)
{

    std::cout << "Generating massive world..." << std::endl;
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

void PlayState::processInput()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            game.quit();
        }

        if (event.type == SDL_EVENT_KEY_DOWN)
        {

            if (event.key.key == SDLK_ESCAPE)
            {
                game.getStateMachine().popState();
                return;
            }

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

void PlayState::update()
{
    auto &pos = registry.get<Position>(playerEntity);
    gameMap.calculateFOV(pos.x, pos.y, 15);
    int playerPixelX = pos.x * 20;
    int playerPixelY = pos.y * 20;

    cameraX = playerPixelX - (game.getWindowWidth() / 2);
    cameraY = playerPixelY - (game.getWindowHeight() / 2);

    if (cameraX < 0)
        cameraX = 0;
    if (cameraY < 0)
        cameraY = 0;
    if (cameraX > (200 * 20) - game.getWindowWidth())
        cameraX = (200 * 20) - game.getWindowWidth();
    if (cameraY > (150 * 20) - game.getWindowHeight())
        cameraY = (150 * 20) - game.getWindowHeight();
}

void PlayState::render()
{
    SDL_Renderer *renderer = game.getRenderer();

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