#pragma once
#include <SDL3/SDL.h>
#include <entt/entt.hpp>

class UIRenderer
{
public:
    void render(SDL_Renderer *renderer, const entt::registry &registry, entt::entity playerEntity);
};