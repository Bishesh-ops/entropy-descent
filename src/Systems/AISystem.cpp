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

void AISystem::update(entt::entity playerEntity, int floorDepth, float dt)
{
    if (!registry.valid(playerEntity) || !registry.all_of<Position>(playerEntity))
        return;
    auto &playerTransform = registry.get<Transform>(playerEntity);
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

    auto view = registry.view<Enemy, Transform, Velocity, Position>();
    for (auto entity : view)
    {
        if (registry.all_of<Phantom>(entity))
            continue;

        auto &enemy = view.get<Enemy>(entity);
        auto &enemyTransform = view.get<Transform>(entity);
        auto &enemyVel = view.get<Velocity>(entity);
        auto &enemyPos = view.get<Position>(entity);

        // Tick down the attack cooldown
        if (enemy.attackCooldown > 0.0f)
        {
            enemy.attackCooldown -= dt;
        }

        // Calculate real pixel distance to player
        float dx = playerTransform.x - enemyTransform.x;
        float dy = playerTransform.y - enemyTransform.y;
        float pixelDistToPlayer = std::sqrt(dx * dx + dy * dy);

        if (pixelDistToPlayer <= 25.0f)
        {
            enemyVel.dx = 0.0f; // Stop moving to attack
            enemyVel.dy = 0.0f;

            if (enemy.attackCooldown <= 0.0f)
            {
                dispatcher.trigger(MeleeAttackEvent{entity, playerEntity});
                enemy.attackCooldown = 1.0f; // Attack once per second
            }
        }
        else if (pixelDistToPlayer <= (aggroRadius * 20.0f))
        {
            auto path = gameMap.findPath(enemyPos.x, enemyPos.y, playerPos.x, playerPos.y);

            if (!path.empty())
            {
                int nextIndex = path[0];
                float targetX = static_cast<float>((nextIndex % mapWidth) * 20);
                float targetY = static_cast<float>((nextIndex / mapWidth) * 20);

                float dirX = targetX - enemyTransform.x;
                float dirY = targetY - enemyTransform.y;
                float distToNode = std::sqrt(dirX * dirX + dirY * dirY);

                if (distToNode > 2.0f)
                {
                    enemyVel.dx = dirX / distToNode;
                    enemyVel.dy = dirY / distToNode;
                }
            }
            else
            {
                enemyVel.dx = 0.0f;
                enemyVel.dy = 0.0f;
            }
        }
        else
        {
            enemyVel.dx = 0.0f;
            enemyVel.dy = 0.0f;
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