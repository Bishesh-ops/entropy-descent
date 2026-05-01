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

        float renderX = static_cast<float>(pos.x * 20);
        float renderY = static_cast<float>(pos.y * 20);

        if (registry.all_of<Transform>(entity))
        {
            auto &t = registry.get<Transform>(entity);
            renderX = t.x;
            renderY = t.y;
        }

        SDL_FRect rect = {
            renderX - static_cast<float>(cameraX),
            renderY - static_cast<float>(cameraY),
            20.0f, 20.0f};
        SDL_RenderFillRect(renderer, &rect);
    }
    auto projView = registry.view<Transform, Projectile, RenderColor>();
    for (auto e : projView)
    {
        auto &t = projView.get<Transform>(e);
        auto &color = projView.get<RenderColor>(e);

        SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
        SDL_FRect rect = {t.x - cameraX, t.y - cameraY, 10.0f, 10.0f};
        SDL_RenderFillRect(renderer, &rect);
    }

    auto particleView = registry.view<Transform, Particle>();
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    for (auto e : particleView)
    {
        auto &t = particleView.get<Transform>(e);
        auto &p = particleView.get<Particle>(e);

        float alphaPct = p.lifeTime / p.maxLife;
        uint8_t a = static_cast<uint8_t>(255.0f * alphaPct);

        SDL_SetRenderDrawColor(renderer, p.r, p.g, p.b, a);
        SDL_FRect rect = {t.x - cameraX, t.y - cameraY, 4.0f, 4.0f};
        SDL_RenderFillRect(renderer, &rect);
    }
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    uiRenderer.render(renderer, registry, playerEntity, windowWidth, windowHeight, floorDepth);
    SDL_RenderPresent(renderer);
}