#include "../include/UIRenderer.hpp"
#include "../include/Components.hpp"
#include <algorithm>

void UIRenderer::render(SDL_Renderer *renderer, const entt::registry &registry, entt::entity playerEntity)
{
    if (!registry.valid(playerEntity) || !registry.all_of<Health>(playerEntity))
        return;

    const auto &hp = registry.get<Health>(playerEntity);

    float barX = 20.0f;
    float barY = 20.0f;
    float barWidth = 200.0f;
    float barHeight = 20.0f;

    // --- 1. Health Bar ---
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_FRect bgRect = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    float hpPercent = std::clamp(static_cast<float>(hp.current) / static_cast<float>(hp.max), 0.0f, 1.0f);
    if (hpPercent > 0.0f)
    {
        SDL_SetRenderDrawColor(renderer, 220, 30, 30, 255);
        SDL_FRect fillRect = {barX, barY, barWidth * hpPercent, barHeight};
        SDL_RenderFillRect(renderer, &fillRect);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &bgRect);

    // --- 2. Inventory Slots ---
    if (registry.all_of<Inventory>(playerEntity))
    {
        const auto &inv = registry.get<Inventory>(playerEntity);
        float slotY = barY + barHeight + 10.0f;
        float slotSize = 15.0f;
        float padding = 5.0f;

        for (int i = 0; i < inv.maxCapacity; ++i)
        {
            SDL_FRect slotRect = {barX + i * (slotSize + padding), slotY, slotSize, slotSize};

            if (i < inv.items.size())
            {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &slotRect);
            }
            else
            {
                SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
                SDL_RenderFillRect(renderer, &slotRect);
                SDL_SetRenderDrawColor(renderer, 120, 120, 120, 255);
                SDL_RenderRect(renderer, &slotRect);
            }
        }
    }
}