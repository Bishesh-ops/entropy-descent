#include "../../include/Systems/AISystem.hpp"
#include "../../include/Components.hpp"
#include <cmath>
#include <iostream>

AISystem::AISystem(entt::registry &reg, entt::dispatcher &disp, sol::state &luaState, Map &mapRef, std::vector<entt::entity> &grid, int w, int h)
    : registry(reg), dispatcher(disp), lua(luaState), gameMap(mapRef), spatialGrid(grid), mapWidth(w), mapHeight(h)
{
}

entt::entity AISystem::getBlockingEntityAt(int x, int y)
{
    if (x < 0 || y < 0 || x >= mapWidth || y >= mapHeight)
        return entt::null;
    return spatialGrid[y * mapWidth + x];
}

void AISystem::updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY)
{
    if (oldX >= 0 && oldY >= 0 && oldX < mapWidth && oldY < mapHeight)
    {
        int oldIndex = oldY * mapWidth + oldX;
        if (spatialGrid[oldIndex] == entity)
        {
            spatialGrid[oldIndex] = entt::null;
        }
    }

    if (newX >= 0 && newY >= 0 && newX < mapWidth && newY < mapHeight)
    {
        spatialGrid[newY * mapWidth + newX] = entity;
    }
}

void AISystem::update(entt::entity playerEntity, int floorDepth)
{
    if (!registry.valid(playerEntity) || !registry.all_of<Position>(playerEntity))
        return;
    auto &playerPos = registry.get<Position>(playerEntity);

    int currentEntropy = 0;
    if (registry.all_of<EntropyStats>(playerEntity))
    {
        currentEntropy = registry.get<EntropyStats>(playerEntity).entropy;
    }

    int aggroRadius = 15;
    try
    {
        aggroRadius = lua["GetAggroRadius"](currentEntropy, floorDepth);
    }
    catch (const sol::error &e)
    {
        std::cerr << "Lua AI Error: " << e.what() << "\n";
    }

    auto view = registry.view<Position, Enemy>();
    for (auto entity : view)
    {
        if (registry.all_of<Phantom>(entity))
            continue;

        auto &enemyPos = view.get<Position>(entity);
        int distToPlayer = std::abs(enemyPos.x - playerPos.x) + std::abs(enemyPos.y - playerPos.y);

        if (distToPlayer == 1)
        {
            dispatcher.trigger(MeleeAttackEvent{entity, playerEntity});
        }
        else if (distToPlayer <= aggroRadius)
        {
            auto path = gameMap.findPath(enemyPos.x, enemyPos.y, playerPos.x, playerPos.y);
            if (!path.empty())
            {
                int nextIndex = path[0];
                int nextX = nextIndex % mapWidth;
                int nextY = nextIndex / mapWidth;

                if (getBlockingEntityAt(nextX, nextY) == entt::null && !(nextX == playerPos.x && nextY == playerPos.y))
                {
                    updateSpatialGrid(entity, enemyPos.x, enemyPos.y, nextX, nextY);
                    enemyPos.x = nextX;
                    enemyPos.y = nextY;
                }
            }
        }

        if (registry.all_of<EntropyStats>(playerEntity))
        {
            auto &pStats = registry.get<EntropyStats>(playerEntity);
            if (pStats.hasPassiveAura && gameMap.isVisible(enemyPos.x, enemyPos.y))
            {
                dispatcher.trigger(DamageEvent{entity, 5});
                std::cout << "Enemy scorched by your Entropic Aura for 5 DMG!\n";
            }
        }
    }
}