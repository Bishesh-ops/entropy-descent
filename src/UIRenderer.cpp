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
                entt::entity itemEntity = inv.items[i];
                if (registry.all_of<RenderColor>(itemEntity))
                {
                    const auto &color = registry.get<RenderColor>(itemEntity);
                    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
                }
                else
                {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // Fallback
                }
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
    // --- 3. Entropy Bar ---
    if (registry.all_of<EntropyStats>(playerEntity))
    {
        const auto &es = registry.get<EntropyStats>(playerEntity);
        float entropyY = barY + barHeight + 10.0f + 15.0f + 10.0f;
        float entropyPercent = std::clamp(es.entropy / 100.0f, 0.0f, 1.0f);

        // Background
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_FRect eBg = {barX, entropyY, barWidth, 10.0f};
        SDL_RenderFillRect(renderer, &eBg);

        // Fill — green to red based on entropy
        if (entropyPercent > 0.0f)
        {
            uint8_t r = static_cast<uint8_t>(255 * entropyPercent);
            uint8_t g = static_cast<uint8_t>(255 * (1.0f - entropyPercent));
            SDL_SetRenderDrawColor(renderer, r, g, 0, 255);
            SDL_FRect eFill = {barX, entropyY, barWidth * entropyPercent, 10.0f};
            SDL_RenderFillRect(renderer, &eFill);
        }

        // Border
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderRect(renderer, &eBg);
    }
}