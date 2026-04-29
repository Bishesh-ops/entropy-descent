#include "../../include/Systems/SpellSystem.hpp"
#include "../../include/Components.hpp"
#include <iostream>
#include <cmath>

SpellSystem::SpellSystem(entt::registry &reg, entt::dispatcher &disp, Map &mapRef, std::vector<entt::entity> &grid, int w, int h, const std::vector<SpellDef> &spells)
    : registry(reg), dispatcher(disp), gameMap(mapRef), spatialGrid(grid), mapWidth(w), mapHeight(h), loadedSpells(spells)
{
    dispatcher.sink<SpellCastEvent>().connect<&SpellSystem::onSpellCast>(this);
}

entt::entity SpellSystem::getBlockingEntityAt(int x, int y)
{
    if (x < 0 || y < 0 || x >= mapWidth || y >= mapHeight)
        return entt::null;
    return spatialGrid[y * mapWidth + x];
}

void SpellSystem::onSpellCast(const SpellCastEvent &event)
{
    if (!registry.valid(event.caster) || !registry.all_of<Position>(event.caster))
        return;
    auto &pos = registry.get<Position>(event.caster);

    // --- Pull data from JSON defs instead of magic numbers ---
    const SpellDef *def = nullptr;
    std::string searchName = (event.type == SpellCastEvent::SpellType::CRYO) ? "Cryo" : "Fire";
    for (const auto &s : loadedSpells)
    {
        if (s.name == searchName)
        {
            def = &s;
            break;
        }
    }

    int spellCost = def ? def->baseCost : 20;
    int spellDmg = def ? def->damage : 0;

    bool validTargetFound = false;
    std::vector<std::pair<int, int>> targetTiles;

    for (int dy = -4; dy <= 4; ++dy)
    {
        for (int dx = -4; dx <= 4; ++dx)
        {
            if (std::abs(dx) + std::abs(dy) <= 4)
            {
                int tx = pos.x + dx;
                int ty = pos.y + dy;
                if (tx >= 0 && tx < mapWidth && ty >= 0 && ty < mapHeight)
                {
                    TileState currentState = gameMap.getTileState(tx, ty);
                    if (event.type == SpellCastEvent::SpellType::CRYO && currentState == TileState::WATER)
                    {
                        targetTiles.push_back({tx, ty});
                        validTargetFound = true;
                    }
                    else if (event.type == SpellCastEvent::SpellType::FIRE && currentState == TileState::FIRE)
                    {
                        targetTiles.push_back({tx, ty});
                        validTargetFound = true;
                    }
                }
            }
        }
    }

    if (validTargetFound)
    {
        std::vector<entt::entity> hitEnemies;
        for (const auto &t : targetTiles)
        {
            gameMap.setTileState(t.first, t.second, (event.type == SpellCastEvent::SpellType::CRYO) ? TileState::FROZEN : TileState::SCORCHED);

            for (int bY = -1; bY <= 1; ++bY)
            {
                for (int bX = -1; bX <= 1; ++bX)
                {
                    entt::entity aoeTarget = getBlockingEntityAt(t.first + bX, t.second + bY);
                    if (aoeTarget != entt::null && registry.all_of<Enemy>(aoeTarget))
                    {
                        if (std::find(hitEnemies.begin(), hitEnemies.end(), aoeTarget) == hitEnemies.end())
                        {
                            hitEnemies.push_back(aoeTarget);
                        }
                    }
                }
            }
        }

        for (auto enemy : hitEnemies)
        {
            if (event.type == SpellCastEvent::SpellType::CRYO)
            {
                dispatcher.trigger(MeleeAttackEvent{event.caster, enemy});
            }
            else if (event.type == SpellCastEvent::SpellType::FIRE)
            {
                dispatcher.trigger(DamageEvent{enemy, spellDmg}); // HANDOFF TO COMBAT SYSTEM
            }
        }

        if (registry.all_of<EntropyStats>(event.caster))
        {
            auto &stats = registry.get<EntropyStats>(event.caster);
            stats.entropy += spellCost;
            std::cout << "Exchange successful! Entropy level: " << stats.entropy << std::endl;
        }
    }
    else
    {
        std::cout << "No entropic source nearby.\n";
    }

    if (event.successFlag)
        *(event.successFlag) = validTargetFound;
}