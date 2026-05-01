#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <sol/sol.hpp>
#include "../Map.hpp"
#include "../Events.hpp"

class AISystem
{
public:
    AISystem(entt::registry &reg, entt::dispatcher &disp, sol::state &luaState, Map &mapRef, std::vector<entt::entity> &grid, int w, int h);

    void update(entt::entity playerEntity, int floorDepth, float dt);

private:
    entt::registry &registry;
    entt::dispatcher &dispatcher;
    sol::state &lua;
    Map &gameMap;
    std::vector<entt::entity> &spatialGrid;
    int mapWidth;
    int mapHeight;

    entt::entity getBlockingEntityAt(int x, int y);
    void updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY);
};