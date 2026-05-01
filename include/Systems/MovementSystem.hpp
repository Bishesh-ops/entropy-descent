#pragma once
#include <entt/entt.hpp>
#include <vector>
#include "../Map.hpp"

class MovementSystem
{
public:
    MovementSystem(entt::registry &reg, Map &mapRef, std::vector<entt::entity> &grid, int w, int h);
    void update(float dt);

private:
    entt::registry &registry;
    Map &gameMap;
    std::vector<entt::entity> &spatialGrid;
    int mapWidth;
    int mapHeight;

    entt::entity getBlockingEntityAt(int x, int y);
    void updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY);
};