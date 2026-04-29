#include "../include/UIRenderer.hpp"
#include "../include/Components.hpp"
#include <algorithm>

void UIRenderer::render(SDL_Renderer *renderer, const entt::registry &registry, entt::entity playerEntity)
{
    if (!registry.valid(playerEntity) || !registry.all_of<Health>(playerEntity))
    {
        return;
    }
    const auto &hp = registry.get<Health>(playerEntity);

    float barX = 20.0f;
    float barY = 20.0f;
    float barWidth = 200.0f;
    float barHeight = 20.0f;

    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
    SDL_FRect bgRect = {barX, barY, barWidth, barHeight};
    SDL_RenderFillRect(renderer, &bgRect);

    float hpPercent = static_cast<float>(hp.current) / static_cast<float>(hp.max);
    hpPercent = std::clamp(hpPercent, 0.0f, 1.0f); // Prevent drawing negatively or out of bounds

    if (hpPercent > 0.0f)
    {
        SDL_SetRenderDrawColor(renderer, 220, 30, 30, 255);
        SDL_FRect fillRect = {barX, barY, barWidth * hpPercent, barHeight};
        SDL_RenderFillRect(renderer, &fillRect);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderRect(renderer, &bgRect);
}