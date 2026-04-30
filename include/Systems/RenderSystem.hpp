#pragma once
#include <SDL3/SDL.h>
#include <entt/entt.hpp>
#include "../Map.hpp"
#include "../UIRenderer.hpp"

class RenderSystem
{
public:
    void update(SDL_Renderer *renderer, const entt::registry &registry, const Map &gameMap,
                int cameraX, int cameraY, int windowWidth, int windowHeight,
                UIRenderer &uiRenderer, entt::entity playerEntity, int floorDepth);
};