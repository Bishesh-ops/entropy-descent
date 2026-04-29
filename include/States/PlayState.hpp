#pragma once
#include "State.hpp"
#include "../Game.hpp"
#include "../Map.hpp"
#include "../Events.hpp"
#include "../UIRenderer.hpp"
#include <entt/entt.hpp>
#include <vector>

enum class TurnState
{
    WAITING_FOR_PLAYER,
    ENEMY_TURN
};

class PlayState : public State
{
public:
    PlayState(Game &gameRef);
    void processInput() override;
    void update() override;
    void render() override;

private:
    Game &game;

    Map gameMap;
    entt::registry registry;
    entt::dispatcher dispatcher;

    UIRenderer uiRenderer;

    entt::entity playerEntity;
    int cameraX;
    int cameraY;
    TurnState currentTurnState;
    bool needsFOVUpdate;

    std::vector<entt::entity> spatialGrid;

    entt::entity getBlockingEntityAt(int x, int y);
    void updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY);

    void onMeleeAttack(const MeleeAttackEvent &event);
    void onEntityDeath(const EntityDeathEvent &event);
};