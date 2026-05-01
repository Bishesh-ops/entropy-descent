#include "../../include/Systems/MovementSystem.hpp"
#include "../../include/Components.hpp"

MovementSystem::MovementSystem(entt::registry &reg, Map &mapRef, std::vector<entt::entity> &grid, int w, int h)
    : registry(reg), gameMap(mapRef), spatialGrid(grid), mapWidth(w), mapHeight(h) {}

entt::entity MovementSystem::getBlockingEntityAt(int x, int y) { return entt::null; /* Deprecated for physics, kept for legacy signature */ }

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
    auto view = registry.view<Transform, Velocity, Hitbox, Position>();

    auto hasOverlap = [&](float testX, float testY, float w, float h, entt::entity self) -> bool
    {
        int minGridX = static_cast<int>(testX / 20.0f);
        int maxGridX = static_cast<int>((testX + w - 0.01f) / 20.0f);
        int minGridY = static_cast<int>(testY / 20.0f);
        int maxGridY = static_cast<int>((testY + h - 0.01f) / 20.0f);

        for (int y = minGridY; y <= maxGridY; ++y)
        {
            for (int x = minGridX; x <= maxGridX; ++x)
            {
                if (!gameMap.isFloor(x, y))
                    return true;
            }
        }
        if (registry.all_of<Collider>(self))
        {
            auto colliders = registry.view<Transform, Hitbox, Collider>();
            for (auto other : colliders)
            {
                if (other == self)
                    continue;

                auto &oTransform = colliders.get<Transform>(other);
                auto &oHitbox = colliders.get<Hitbox>(other);

                float oX = oTransform.x + oHitbox.offsetX;
                float oY = oTransform.y + oHitbox.offsetY;

                if (testX < oX + oHitbox.width &&
                    testX + w > oX &&
                    testY < oY + oHitbox.height &&
                    testY + h > oY)
                {
                    return true;
                }
            }
        }
        return false;
    };

    for (auto entity : view)
    {
        auto &transform = view.get<Transform>(entity);
        auto &vel = view.get<Velocity>(entity);
        auto &hitbox = view.get<Hitbox>(entity);
        auto &pos = view.get<Position>(entity);

        if (vel.dx == 0.0f && vel.dy == 0.0f)
            continue;

        float nextX = transform.x + (vel.dx * vel.speed * dt);
        if (!hasOverlap(nextX + hitbox.offsetX, transform.y + hitbox.offsetY, hitbox.width, hitbox.height, entity))
        {
            transform.x = nextX;
        }

        float nextY = transform.y + (vel.dy * vel.speed * dt);
        if (!hasOverlap(transform.x + hitbox.offsetX, nextY + hitbox.offsetY, hitbox.width, hitbox.height, entity))
        {
            transform.y = nextY;
        }
        int newGridX = static_cast<int>((transform.x + 10.0f) / 20.0f);
        int newGridY = static_cast<int>((transform.y + 10.0f) / 20.0f);

        if (newGridX != pos.x || newGridY != pos.y)
        {
            updateSpatialGrid(entity, pos.x, pos.y, newGridX, newGridY);
            pos.x = newGridX;
            pos.y = newGridY;
        }
    }
}