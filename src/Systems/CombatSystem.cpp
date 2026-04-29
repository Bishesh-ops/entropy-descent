#include "../../include/Systems/CombatSystem.hpp"
#include "../../include/Components.hpp"
#include <iostream>

CombatSystem::CombatSystem(Game &gameRef, entt::registry &reg, entt::dispatcher &disp, std::vector<entt::entity> &grid, int w, int h)
    : game(gameRef), registry(reg), dispatcher(disp), spatialGrid(grid), mapWidth(w), mapHeight(h)
{
    // Subscribe to events
    dispatcher.sink<MeleeAttackEvent>().connect<&CombatSystem::onMeleeAttack>(this);
    dispatcher.sink<EntityDeathEvent>().connect<&CombatSystem::onEntityDeath>(this);
    dispatcher.sink<DamageEvent>().connect<&CombatSystem::onDamage>(this);
    dispatcher.sink<HealEvent>().connect<&CombatSystem::onHeal>(this);
}

void CombatSystem::update()
{
    // Deferred Destruction Cleanup
    for (auto deadEntity : pendingDestroy)
    {
        if (registry.all_of<Position>(deadEntity))
        {
            auto &deadPos = registry.get<Position>(deadEntity);
            int index = deadPos.y * mapWidth + deadPos.x;

            if (index >= 0 && index < (mapWidth * mapHeight) && spatialGrid[index] == deadEntity)
            {
                spatialGrid[index] = entt::null; // Clear collision
            }
        }
        registry.destroy(deadEntity);
    }
    pendingDestroy.clear();
}

void CombatSystem::onMeleeAttack(const MeleeAttackEvent &event)
{
    if (!registry.valid(event.attacker) || !registry.valid(event.target))
        return;

    auto &aStats = registry.get<CombatStats>(event.attacker);
    auto &tHealth = registry.get<Health>(event.target);
    auto &tStats = registry.get<CombatStats>(event.target);

    int damage = std::max(1, aStats.attack - tStats.defense);
    tHealth.current -= damage;

    std::cout << "Entity " << static_cast<uint32_t>(event.attacker)
              << " hit Entity " << static_cast<uint32_t>(event.target)
              << " for " << damage << " dmg! (HP: " << tHealth.current << ")" << std::endl;

    if (tHealth.current <= 0)
    {
        dispatcher.trigger(EntityDeathEvent{event.target});
    }
}

void CombatSystem::onEntityDeath(const EntityDeathEvent &event)
{
    if (registry.all_of<Player>(event.deadEntity))
    {
        std::cout << "YOU HAVE DESCENDED. GAME OVER." << std::endl;
        game.quit();
    }
    else
    {
        std::cout << "Enemy shattered into entropy!" << std::endl;
        pendingDestroy.push_back(event.deadEntity);
    }
}

void CombatSystem::onDamage(const DamageEvent &event)
{
    if (!registry.valid(event.target) || !registry.all_of<Health>(event.target))
        return;
    auto &hp = registry.get<Health>(event.target);
    hp.current -= event.damage;
    std::cout << "Entity " << static_cast<uint32_t>(event.target) << " took " << event.damage << " flat DMG! (HP: " << hp.current << ")\n";
    if (hp.current <= 0)
        dispatcher.trigger(EntityDeathEvent{event.target});
}

void CombatSystem::onHeal(const HealEvent &event)
{
    if (!registry.valid(event.target) || !registry.all_of<Health>(event.target))
        return;
    auto &hp = registry.get<Health>(event.target);
    hp.current = std::min(hp.max, hp.current + event.amount);
    std::cout << "Entity " << static_cast<uint32_t>(event.target) << " healed for " << event.amount << " HP! (HP: " << hp.current << ")\n";
}