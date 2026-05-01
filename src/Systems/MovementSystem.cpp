#include "../../include/Systems/MovementSystem.hpp"
#include "../../include/Components.hpp"

MovementSystem::MovementSystem(entt::registry &reg, Map &mapRef, std::vector<entt::entity> &grid, int w, int h)
    : registry(reg), gameMap(mapRef), spatialGrid(grid), mapWidth(w), mapHeight(h) {}

entt::entity MovementSystem::getBlockingEntityAt(int x, int y)
{
    if (x < 0 || y < 0 || x >= mapWidth || y >= mapHeight)
        return entt::null;
    return spatialGrid[y * mapWidth + x];
}

void MovementSystem::updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY)
{
    if (oldX >= 0 && oldY >= 0 && oldX < mapWidth && oldY < mapHeight)
    {
        int oldIndex = oldY * mapWidth + oldX;
        if (spatialGrid[oldIndex] == entity)
            spatialGrid[oldIndex] = entt::null;
    }
    if (newX >= 0 && newY >= 0 && newX < mapWidth && newY < mapHeight)
    {
        spatialGrid[newY * mapWidth + newX] = entity;
    }
}

void MovementSystem::update(float dt)
{
    auto view = registry.view<Transform, Velocity, Position>();

    for (auto entity : view)
    {
        auto &transform = view.get<Transform>(entity);
        auto &vel = view.get<Velocity>(entity);
        auto &pos = view.get<Position>(entity);

        if (vel.dx == 0.0f && vel.dy == 0.0f)
            continue;

        float nextX = transform.x + (vel.dx * vel.speed * dt);
        float nextY = transform.y + (vel.dy * vel.speed * dt);

        int currentGridX = static_cast<int>((transform.x + 10.0f) / 20.0f);
        int currentGridY = static_cast<int>((transform.y + 10.0f) / 20.0f);

        int nextGridX = static_cast<int>((nextX + 10.0f) / 20.0f);
        int nextGridY = static_cast<int>((nextY + 10.0f) / 20.0f);

        bool hasCollider = registry.all_of<Collider>(entity);

        bool canMoveX = gameMap.isFloor(nextGridX, currentGridY);
        if (canMoveX && hasCollider)
        {
            entt::entity blocker = getBlockingEntityAt(nextGridX, currentGridY);
            if (blocker != entt::null && blocker != entity)
                canMoveX = false;
        }
        if (canMoveX)
            transform.x = nextX;

        int updatedGridX = static_cast<int>((transform.x + 10.0f) / 20.0f);
        bool canMoveY = gameMap.isFloor(updatedGridX, nextGridY);
        if (canMoveY && hasCollider)
        {
            entt::entity blocker = getBlockingEntityAt(updatedGridX, nextGridY);
            if (blocker != entt::null && blocker != entity)
                canMoveY = false;
        }
        if (canMoveY)
            transform.y = nextY;

        int finalGridX = static_cast<int>((transform.x + 10.0f) / 20.0f);
        int finalGridY = static_cast<int>((transform.y + 10.0f) / 20.0f);

        if (finalGridX != pos.x || finalGridY != pos.y)
        {
            if (hasCollider)
            {
                updateSpatialGrid(entity, pos.x, pos.y, finalGridX, finalGridY);
            }
            pos.x = finalGridX;
            pos.y = finalGridY;
        }
    }
}