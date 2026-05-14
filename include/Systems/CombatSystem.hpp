#pragma once
#include <entt/entt.hpp>
#include <vector>
#include <sol/sol.hpp>
#include <SDL3_mixer/SDL_mixer.h>
#include "../Events.hpp"
#include "../Game.hpp"

class CombatSystem
{
public:
    CombatSystem(Game &gameRef, entt::registry &reg, entt::dispatcher &disp, sol::state &luaState, std::vector<entt::entity> &grid, int w, int h);
    ~CombatSystem();

    void update(float dt);

    void onMeleeAttack(const MeleeAttackEvent &event);
    void onEntityDeath(const EntityDeathEvent &event);
    void onDamage(const DamageEvent &event);
    void onHeal(const HealEvent &event);

private:
    Game &game;
    entt::registry &registry;
    entt::dispatcher &dispatcher;
    std::vector<entt::entity> &spatialGrid;
    sol::state &lua;
    int mapWidth;
    int mapHeight;

    // Modern uncompressed/compressed cached RAM audio assets
    MIX_Audio *hitAudio = nullptr;
    MIX_Audio *deathAudio = nullptr;
};
