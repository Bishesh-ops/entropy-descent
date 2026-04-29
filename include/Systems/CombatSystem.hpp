#pragma once
#include <entt/entt.hpp>
#include <vector>
#include "../Events.hpp"
#include "../Game.hpp"

class CombatSystem
{
public:
    CombatSystem(Game &gameRef, entt::registry &reg, entt::dispatcher &disp, std::vector<entt::entity> &grid, int w, int h);

    // Handles deferred destruction
    void update();

    void onMeleeAttack(const MeleeAttackEvent &event);
    void onEntityDeath(const EntityDeathEvent &event);
    void onDamage(const DamageEvent &event);
    void onHeal(const HealEvent &event);

private:
    Game &game;
    entt::registry &registry;
    entt::dispatcher &dispatcher;
    std::vector<entt::entity> &spatialGrid;
    int mapWidth;
    int mapHeight;

    std::vector<entt::entity> pendingDestroy;
};