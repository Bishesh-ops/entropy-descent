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
    if (!registry.valid(playerEntity) || !registry.all_of<Transform>(playerEntity) || !registry.all_of<Hitbox>(playerEntity))
        return;

    auto &playerTransform = registry.get<Transform>(playerEntity);
    auto &playerPos = registry.get<Position>(playerEntity);
    auto &pHitbox = registry.get<Hitbox>(playerEntity);

    float pX = playerTransform.x + pHitbox.offsetX;
    float pY = playerTransform.y + pHitbox.offsetY;

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

    auto view = registry.view<Enemy, Transform, Velocity, Position, Hitbox>();

    for (auto entity : view)
    {
        if (registry.all_of<Phantom>(entity))
            continue;

        auto &enemy = view.get<Enemy>(entity);
        auto &enemyTransform = view.get<Transform>(entity);
        auto &enemyVel = view.get<Velocity>(entity);
        auto &enemyPos = view.get<Position>(entity);
        auto &enemyHitbox = view.get<Hitbox>(entity);

        if (enemy.attackCooldown > 0.0f)
        {
            enemy.attackCooldown -= dt;
        }

        float eX = enemyTransform.x + enemyHitbox.offsetX;
        float eY = enemyTransform.y + enemyHitbox.offsetY;

        float reach = 2.0f;
        bool inCombatRange = (eX - reach < pX + pHitbox.width &&
                              eX + enemyHitbox.width + reach > pX &&
                              eY - reach < pY + pHitbox.height &&
                              eY + enemyHitbox.height + reach > pY);

        float dx = (playerTransform.x + 10.0f) - (enemyTransform.x + 10.0f);
        float dy = (playerTransform.y + 10.0f) - (enemyTransform.y + 10.0f);
        float pixelDistToPlayer = std::sqrt(dx * dx + dy * dy);

        if (inCombatRange)
        {
            enemyVel.dx = 0.0f;
            enemyVel.dy = 0.0f;

            if (enemy.attackCooldown <= 0.0f)
            {
                dispatcher.trigger(MeleeAttackEvent{entity, playerEntity});
                enemy.attackCooldown = 1.0f;
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
                if (enemy.auraTickTimer > 0.0f)
                {
                    enemy.auraTickTimer -= dt;
                }
                else
                {
                    dispatcher.trigger(DamageEvent{entity, 5});
                    enemy.auraTickTimer = 1.0f;

                    sol::function spawnFunc = lua["SpawnParticles"];
                    if (spawnFunc.valid())
                    {
                        spawnFunc(enemyTransform.x, enemyTransform.y, 3, 150, 0, 255);
                    }
                }
            }
        }
    }
}