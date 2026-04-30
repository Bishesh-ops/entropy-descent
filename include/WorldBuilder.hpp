#pragma once
#include <entt/entt.hpp>
#include <vector>
#include "Map.hpp"
#include "DataLoader.hpp"

class WorldBuilder
{
public:
    static entt::entity generateFloor(entt::registry &registry, Map &gameMap, std::vector<entt::entity> &spatialGrid, int mapWidth, int mapHeight, const std::vector<EnemyDef> &enemies, const std::vector<ItemDef> &items);

    static void repopulateFloor(entt::registry &registry, Map &gameMap, std::vector<entt::entity> &spatialGrid, int mapWidth, int mapHeight, const std::vector<EnemyDef> &enemies, const std::vector<ItemDef> &items, entt::entity existingPlayer, int floorDepth);
};