#include "../include/WorldBuilder.hpp"
#include "../include/Components.hpp"
#include <iostream>
#include <random>

entt::entity WorldBuilder::generateFloor(entt::registry &registry, Map &gameMap, std::vector<entt::entity> &spatialGrid,
                                         int mapWidth, int mapHeight,
                                         const std::vector<EnemyDef> &enemies, const std::vector<ItemDef> &items)
{
    std::cout << "Generating massive world...\n";
    gameMap.generateCaves(45, 5);
    gameMap.processMap();

    std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> distX(2, mapWidth - 3);
    std::uniform_int_distribution<int> distY(2, mapHeight - 3);
    std::uniform_int_distribution<int> chance(0, 99);

    // Entropic Pools (Water & Fire)
    for (int i = 0; i < 40; ++i)
    { // 40 Water
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
    { // 15 Fire
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

    // Player Init
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
    spatialGrid[startY * mapWidth + startX] = playerEntity;

    // Enemy Init
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
            registry.emplace<Enemy>(enemy);
            registry.emplace<Collider>(enemy);
            registry.emplace<Health>(enemy, def.hp, def.hp);
            registry.emplace<CombatStats>(enemy, def.attack, def.defense);
            registry.emplace<RenderColor>(enemy, def.r, def.g, def.b, def.a);
            spatialGrid[ey * mapWidth + ex] = enemy;
        }
    }

    // Item Init
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

    return playerEntity;
}