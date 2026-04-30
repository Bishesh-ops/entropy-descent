#include "../../include/Systems/RenderSystem.hpp"
#include "../../include/Components.hpp"

void RenderSystem::update(SDL_Renderer *renderer, const entt::registry &registry, const Map &gameMap,
                          int cameraX, int cameraY, int windowWidth, int windowHeight,
                          UIRenderer &uiRenderer, entt::entity playerEntity, int floorDepth)
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    gameMap.render(renderer, cameraX, cameraY, windowWidth, windowHeight);

    auto view = registry.view<Position>();
    for (auto entity : view)
    {
        auto &pos = view.get<Position>(entity);

        if (registry.all_of<Enemy>(entity) && !gameMap.isVisible(pos.x, pos.y))
            continue;

        if (registry.all_of<Player>(entity))
        {
            SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
        }
        else if (registry.all_of<Enemy>(entity))
        {
            if (registry.all_of<RenderColor>(entity))
            {
                auto &color = registry.get<RenderColor>(entity);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            }
        }
        else if (registry.all_of<Item>(entity) || registry.all_of<Stairs>(entity))
        {
            if (registry.all_of<RenderColor>(entity))
            {
                auto &color = registry.get<RenderColor>(entity);
                SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            }
        }
        else
        {
            SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);
        }

        SDL_FRect rect = {
            static_cast<float>((pos.x * 20) - cameraX),
            static_cast<float>((pos.y * 20) - cameraY),
            20.0f, 20.0f};
        SDL_RenderFillRect(renderer, &rect);
    }

    uiRenderer.render(renderer, registry, playerEntity, windowWidth, windowHeight, floorDepth);
    SDL_RenderPresent(renderer);
}