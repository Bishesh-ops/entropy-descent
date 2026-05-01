#include "../include/WorldBuilder.hpp"
#include "../include/Components.hpp"
#include <iostream>
#include <random>
#include <algorithm>

entt::entity WorldBuilder::generateFloor(entt::registry &registry, Map &gameMap, std::vector<entt::entity> &spatialGrid,
                                         int mapWidth, int mapHeight,
                                         const std::vector<EnemyDef> &enemies, const std::vector<ItemDef> &items)
{
    std::fill(spatialGrid.begin(), spatialGrid.end(), static_cast<entt::entity>(entt::null));

    gameMap.generateCaves(45, 5);
    gameMap.processMap();

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(2, mapWidth - 3);
    std::uniform_int_distribution<int> distY(2, mapHeight - 3);
    std::uniform_int_distribution<int> chance(0, 99);

    for (int i = 0; i < 40; ++i)
    {
        int cx = distX(rng), cy = distY(rng);
        while (!gameMap.isFloor(cx, cy))
        {
            cx = distX(rng);
            cy = distY(rng);
        }
        for (int dy = -2; dy <= 2; ++dy)
        {
            for (int dx = -2; dx <= 2; ++dx)
            {
                if (gameMap.isFloor(cx + dx, cy + dy) && chance(rng) < (100 - ((std::abs(dx) + std::abs(dy)) * 25)))
                    gameMap.setTileState(cx + dx, cy + dy, TileState::WATER);
            }
        }
    }
    for (int i = 0; i < 15; ++i)
    {
        int cx = distX(rng), cy = distY(rng);
        while (!gameMap.isFloor(cx, cy))
        {
            cx = distX(rng);
            cy = distY(rng);
        }
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (gameMap.isFloor(cx + dx, cy + dy) && chance(rng) < 60)
                    gameMap.setTileState(cx + dx, cy + dy, TileState::FIRE);
            }
        }
    }

    auto playerEntity = registry.create();
    registry.emplace<Player>(playerEntity);
    registry.emplace<Collider>(playerEntity);
    registry.emplace<Health>(playerEntity, 100, 100);
    registry.emplace<CombatStats>(playerEntity, 10, 5);
    registry.emplace<Inventory>(playerEntity);
    registry.emplace<EntropyStats>(playerEntity);

    int startX = 100, startY = 75;
    while (!gameMap.isFloor(startX, startY))
    {
        startX++;
        if (startX >= mapWidth)
        {
            startX = 0;
            startY++;
        }
    }
    registry.emplace<Position>(playerEntity, startX, startY);
    registry.emplace<Transform>(playerEntity, static_cast<float>(startX * 20), static_cast<float>(startY * 20));
    registry.emplace<Velocity>(playerEntity, 0.0f, 0.0f, 150.0f);
    spatialGrid[startY * mapWidth + startX] = playerEntity;

    if (!enemies.empty())
    {
        std::uniform_int_distribution<int> distArch(0, enemies.size() - 1);
        for (int i = 0; i < 60; ++i)
        {
            int ex = distX(rng), ey = distY(rng);
            while (!gameMap.isFloor(ex, ey) || (std::abs(ex - startX) < 10 && std::abs(ey - startY) < 10) || spatialGrid[ey * mapWidth + ex] != entt::null)
            {
                ex = distX(rng);
                ey = distY(rng);
            }
            const EnemyDef &def = enemies[distArch(rng)];
            auto enemy = registry.create();
            registry.emplace<Position>(enemy, ex, ey);
            registry.emplace<Transform>(enemy, static_cast<float>(ex * 20), static_cast<float>(ey * 20));
            registry.emplace<Velocity>(enemy, 0.0f, 0.0f, 75.0f);

            registry.emplace<Enemy>(enemy);
            registry.emplace<Collider>(enemy);
            registry.emplace<Health>(enemy, def.hp, def.hp);
            registry.emplace<CombatStats>(enemy, def.attack, def.defense);
            registry.emplace<RenderColor>(enemy, def.r, def.g, def.b, def.a);
            spatialGrid[ey * mapWidth + ex] = enemy;
        }
    }

    if (!items.empty())
    {
        std::uniform_int_distribution<int> distItem(0, items.size() - 1);
        for (int i = 0; i < 10; ++i)
        {
            int ix = distX(rng), iy = distY(rng);
            while (!gameMap.isFloor(ix, iy))
            {
                ix = distX(rng);
                iy = distY(rng);
            }
            const ItemDef &def = items[distItem(rng)];
            auto item = registry.create();
            registry.emplace<Position>(item, ix, iy);
            registry.emplace<Item>(item);
            registry.emplace<ItemEffect>(item, def.effectType, def.magnitude);
            registry.emplace<RenderColor>(item, def.r, def.g, def.b, def.a);
        }
    }

    int playerX = registry.get<Position>(playerEntity).x;
    int playerY = registry.get<Position>(playerEntity).y;
    while (true)
    {
        int sx = rand() % mapWidth;
        int sy = rand() % mapHeight;
        if (gameMap.isFloor(sx, sy) && spatialGrid[sy * mapWidth + sx] == entt::null)
        {
            int dist = std::abs(sx - playerX) + std::abs(sy - playerY);
            if (dist >= 30)
            {
                auto stairs = registry.create();
                registry.emplace<Position>(stairs, sx, sy);
                registry.emplace<Stairs>(stairs);
                registry.emplace<RenderColor>(stairs, static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(0), static_cast<uint8_t>(255));
                break;
            }
        }
    }

    return playerEntity;
}

void WorldBuilder::repopulateFloor(entt::registry &registry, Map &gameMap, std::vector<entt::entity> &spatialGrid,
                                   int mapWidth, int mapHeight, const std::vector<EnemyDef> &enemies,
                                   const std::vector<ItemDef> &items, entt::entity existingPlayer, int floorDepth)
{
    std::fill(spatialGrid.begin(), spatialGrid.end(), static_cast<entt::entity>(entt::null));

    gameMap.generateCaves(45, 5);
    gameMap.processMap();

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(2, mapWidth - 3);
    std::uniform_int_distribution<int> distY(2, mapHeight - 3);
    std::uniform_int_distribution<int> chance(0, 99);

    for (int i = 0; i < 40; ++i)
    {
        int cx = distX(rng), cy = distY(rng);
        while (!gameMap.isFloor(cx, cy))
        {
            cx = distX(rng);
            cy = distY(rng);
        }
        for (int dy = -2; dy <= 2; ++dy)
        {
            for (int dx = -2; dx <= 2; ++dx)
            {
                if (gameMap.isFloor(cx + dx, cy + dy) && chance(rng) < (100 - ((std::abs(dx) + std::abs(dy)) * 25)))
                    gameMap.setTileState(cx + dx, cy + dy, TileState::WATER);
            }
        }
    }
    for (int i = 0; i < 15; ++i)
    {
        int cx = distX(rng), cy = distY(rng);
        while (!gameMap.isFloor(cx, cy))
        {
            cx = distX(rng);
            cy = distY(rng);
        }
        for (int dy = -1; dy <= 1; ++dy)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (gameMap.isFloor(cx + dx, cy + dy) && chance(rng) < 60)
                    gameMap.setTileState(cx + dx, cy + dy, TileState::FIRE);
            }
        }
    }

    int startX, startY;
    while (true)
    {
        startX = rand() % mapWidth;
        startY = rand() % mapHeight;
        if (gameMap.isFloor(startX, startY))
        {
            registry.replace<Transform>(existingPlayer, static_cast<float>(startX * 20), static_cast<float>(startY * 20));
            registry.replace<Velocity>(existingPlayer, 0.0f, 0.0f, 150.0f);
            spatialGrid[startY * mapWidth + startX] = existingPlayer;
            break;
        }
    }

    if (!enemies.empty())
    {
        float scaleMultiplier = 1.0f + ((floorDepth - 1) * 0.15f);
        for (int i = 0; i < 60; ++i)
        {
            while (true)
            {
                int ex = rand() % mapWidth;
                int ey = rand() % mapHeight;
                if (gameMap.isFloor(ex, ey) && spatialGrid[ey * mapWidth + ex] == entt::null)
                {
                    const auto &def = enemies[rand() % enemies.size()];
                    auto enemy = registry.create();

                    registry.emplace<Position>(enemy, ex, ey);
                    registry.emplace<Transform>(enemy, static_cast<float>(ex * 20), static_cast<float>(ey * 20));
                    registry.emplace<Velocity>(enemy, 0.0f, 0.0f, 75.0f);
                    registry.emplace<RenderColor>(enemy, def.r, def.g, def.b, static_cast<uint8_t>(255));
                    registry.emplace<Enemy>(enemy);
                    registry.emplace<Collider>(enemy);

                    registry.emplace<Health>(enemy, std::max(1, static_cast<int>(def.hp * scaleMultiplier)));
                    registry.emplace<CombatStats>(enemy,
                                                  std::max(1, static_cast<int>(def.attack * scaleMultiplier)),
                                                  std::max(1, static_cast<int>(def.defense * scaleMultiplier)));

                    spatialGrid[ey * mapWidth + ex] = enemy;
                    break;
                }
            }
        }
    }

    if (!items.empty())
    {
        for (int i = 0; i < 20; ++i)
        {
            while (true)
            {
                int ix = rand() % mapWidth;
                int iy = rand() % mapHeight;
                if (gameMap.isFloor(ix, iy) && spatialGrid[iy * mapWidth + ix] == entt::null)
                {
                    const auto &def = items[rand() % items.size()];
                    auto item = registry.create();
                    registry.emplace<Position>(item, ix, iy);
                    registry.emplace<RenderColor>(item, def.r, def.g, def.b, static_cast<uint8_t>(255));
                    registry.emplace<Item>(item);
                    registry.emplace<ItemEffect>(item, def.effectType, def.magnitude);
                    break;
                }
            }
        }
    }

    while (true)
    {
        int sx = rand() % mapWidth;
        int sy = rand() % mapHeight;
        if (gameMap.isFloor(sx, sy) && spatialGrid[sy * mapWidth + sx] == entt::null)
        {
            int dist = std::abs(sx - startX) + std::abs(sy - startY);
            if (dist >= 30)
            {
                auto stairs = registry.create();
                registry.emplace<Position>(stairs, sx, sy);
                registry.emplace<Stairs>(stairs);
                registry.emplace<RenderColor>(stairs, static_cast<uint8_t>(255), static_cast<uint8_t>(255), static_cast<uint8_t>(0), static_cast<uint8_t>(255));
                break;
            }
        }
    }
}