#pragma once
#include "State.hpp"
#include "../Game.hpp"
#include "../Map.hpp"
#include "../Events.hpp"
#include "../UIRenderer.hpp"
#include <sol/sol.hpp>
#include <entt/entt.hpp>
#include <vector>
#include "../Systems/RenderSystem.hpp"
#include "../Systems/CombatSystem.hpp"
#include "../Systems/ItemSystem.hpp"
#include "../Systems/SpellSystem.hpp"
#include "../Systems/AISystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include <memory>

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
    void update(float dt) override;
    void render() override;

private:
    Game &game;

    int floorDepth = 1;
    std::vector<EnemyDef> loadedEnemies;
    std::vector<ItemDef> loadedItems;
    std::unique_ptr<MovementSystem> movementSystem;

    Map gameMap;
    bool lastSpellSucceeded = false;
    entt::registry registry;
    entt::dispatcher dispatcher;
    sol::state lua;

    UIRenderer uiRenderer;
    RenderSystem renderSystem;

    entt::entity playerEntity;
    int cameraX;
    int cameraY;
    TurnState currentTurnState;
    bool needsFOVUpdate;

    std::vector<entt::entity> spatialGrid;
    std::unique_ptr<CombatSystem> combatSystem;
    std::unique_ptr<ItemSystem> itemSystem;
    std::unique_ptr<SpellSystem> spellSystem;
    std::unique_ptr<AISystem> aiSystem;

    entt::entity getBlockingEntityAt(int x, int y);
    void updateSpatialGrid(entt::entity entity, int oldX, int oldY, int newX, int newY);
    void onDescend(const DescendEvent &event);
};