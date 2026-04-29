#pragma once
#include <entt/entt.hpp>
#include <vector>
#include "../Events.hpp"
#include "../Map.hpp"
#include "../Definitions.hpp"

class SpellSystem
{
public:
    SpellSystem(entt::registry &reg, entt::dispatcher &disp, Map &mapRef, std::vector<entt::entity> &grid, int w, int h, const std::vector<SpellDef> &spells);
    void onSpellCast(const SpellCastEvent &event);

private:
    entt::registry &registry;
    entt::dispatcher &dispatcher;
    Map &gameMap;
    std::vector<entt::entity> &spatialGrid;
    int mapWidth;
    int mapHeight;
    std::vector<SpellDef> loadedSpells;

    entt::entity getBlockingEntityAt(int x, int y);
};