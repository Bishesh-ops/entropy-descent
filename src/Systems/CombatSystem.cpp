#include "../../include/Systems/CombatSystem.hpp"
#include "../../include/Components.hpp"
#include <iostream>

CombatSystem::CombatSystem(Game &gameRef, entt::registry &reg, entt::dispatcher &disp, sol::state &luaState, std::vector<entt::entity> &grid, int w, int h)
    : game(gameRef), registry(reg), dispatcher(disp), lua(luaState), spatialGrid(grid), mapWidth(w), mapHeight(h)
{
    dispatcher.sink<MeleeAttackEvent>().connect<&CombatSystem::onMeleeAttack>(this);
    dispatcher.sink<EntityDeathEvent>().connect<&CombatSystem::onEntityDeath>(this);
    dispatcher.sink<DamageEvent>().connect<&CombatSystem::onDamage>(this);
    dispatcher.sink<HealEvent>().connect<&CombatSystem::onHeal>(this);
}

void CombatSystem::update(float dt)
{
    auto view = registry.view<PendingDestroy>();
    std::vector<entt::entity> toDestroy(view.begin(), view.end());

    for (auto deadEntity : toDestroy)
    {
        if (registry.all_of<Position>(deadEntity))
        {
            auto &deadPos = registry.get<Position>(deadEntity);
            int index = deadPos.y * mapWidth + deadPos.x;

            if (index >= 0 && index < (mapWidth * mapHeight) && spatialGrid[index] == deadEntity)
            {
                spatialGrid[index] = entt::null;
            }
        }
        registry.destroy(deadEntity);
    }
    auto pView = registry.view<Transform, Particle>();
    std::vector<entt::entity> deadParticles;
    for (auto e : pView)
    {
        auto &t = pView.get<Transform>(e);
        auto &p = pView.get<Particle>(e);
        t.x += p.vx * dt;
        t.y += p.vy * dt;
        p.lifeTime -= dt;
        if (p.lifeTime <= 0.0f)
            deadParticles.push_back(e);
    }
    for (auto e : deadParticles)
        registry.destroy(e);

    auto projView = registry.view<Transform, Velocity, Hitbox, Projectile>();
    auto enemyView = registry.view<Transform, Hitbox, Enemy>();
    std::vector<entt::entity> deadProjectiles;

    for (auto pEntity : projView)
    {
        auto &pT = projView.get<Transform>(pEntity);
        auto &pV = projView.get<Velocity>(pEntity);
        auto &pH = projView.get<Hitbox>(pEntity);
        auto &proj = projView.get<Projectile>(pEntity);

        proj.lifeTime -= dt;
        if (proj.lifeTime <= 0.0f)
        {
            deadProjectiles.push_back(pEntity);
            continue;
        }

        pT.x += pV.dx * pV.speed * dt;
        pT.y += pV.dy * pV.speed * dt;

        float pLeft = pT.x + pH.offsetX;
        float pRight = pLeft + pH.width;
        float pTop = pT.y + pH.offsetY;
        float pBot = pTop + pH.height;

        bool hit = false;
        for (auto eEntity : enemyView)
        {
            auto &eT = enemyView.get<Transform>(eEntity);
            auto &eH = enemyView.get<Hitbox>(eEntity);

            if (pLeft < eT.x + eH.offsetX + eH.width && pRight > eT.x + eH.offsetX &&
                pTop < eT.y + eH.offsetY + eH.height && pBot > eT.y + eH.offsetY)
            {

                dispatcher.trigger(DamageEvent{eEntity, proj.damage});

                lua["SpawnParticles"](eT.x, eT.y, 10, 200, 20, 20);

                hit = true;
                break;
            }
        }
        if (hit)
            deadProjectiles.push_back(pEntity);
    }
    for (auto e : deadProjectiles)
        registry.destroy(e);
}

void CombatSystem::onMeleeAttack(const MeleeAttackEvent &event)
{
    if (!registry.valid(event.attacker) || !registry.valid(event.target) || registry.all_of<PendingDestroy>(event.target))
        return;

    auto &aStats = registry.get<CombatStats>(event.attacker);
    auto &tHealth = registry.get<Health>(event.target);
    auto &tStats = registry.get<CombatStats>(event.target);

    int damage = lua["CalculateMeleeDamage"](aStats.attack, tStats.defense);

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

        auto view = registry.view<Player, EntropyStats>();
        for (auto p : view)
        {
            auto &eStats = view.get<EntropyStats>(p);
            if (eStats.healthLocked)
            {
                eStats.entropy = std::max(0, eStats.entropy - 5);
                std::cout << "Martyr's Vow siphons essence. Entropy reduced by 5! (Current: " << eStats.entropy << ")\n";
            }
        }

        if (!registry.all_of<PendingDestroy>(event.deadEntity))
        {
            registry.emplace<PendingDestroy>(event.deadEntity);
        }
    }
}

void CombatSystem::onDamage(const DamageEvent &event)
{
    if (!registry.valid(event.target) || !registry.all_of<Health>(event.target) || registry.all_of<PendingDestroy>(event.target))
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
    if (registry.all_of<EntropyStats>(event.target) && registry.get<EntropyStats>(event.target).healthLocked)
    {
        std::cout << "Your vitality is bound to the abyss. Healing failed!\n";
        return;
    }
    auto &hp = registry.get<Health>(event.target);
    hp.current = std::min(hp.max, hp.current + event.amount);
    std::cout << "Entity " << static_cast<uint32_t>(event.target) << " healed for " << event.amount << " HP! (HP: " << hp.current << ")\n";
}